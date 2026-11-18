/* HHKB Classic: two independently validated EEPROM snapshots of VIA keymap, combos, and macros.
 * 0x08080600..0x08080d9f were zero in the supplied stock EEPROM backup.
 * Never overwrite a slot containing data that is not ours. */
#include "vial.h"
#include "stm32l0xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef SLOT_ADDRESS
#define SLOT_ADDRESS 0x08080600UL
#endif
#ifdef HHKB_JIS
#define SLOT_MAGIC_V1 0x314a4d44UL
#define SLOT_MAGIC_V2 0x324a4d44UL
#else
#define SLOT_MAGIC_V1 0x314b4d44UL // 'DMK1' (Keymap only)
#define SLOT_MAGIC_V2 0x324b4d44UL // 'DMK2' (Keymap + Combos + Macros)
#endif
#define SLOT_MAGIC    SLOT_MAGIC_V2
#define SLOT_COMMIT   0x54494d43UL // 'CMIT'

#define V1_KEYMAP_WORDS 240U
#define V1_SLOT_WORDS   (4U + V1_KEYMAP_WORDS)

#define KEYMAP_WORDS  (DYNAMIC_KEYMAP_MAX_LAYERS * NUM_KEYS)
#define COMBO_WORDS   ((VIAL_COMBO_ENTRIES * sizeof(vial_combo_entry_t) + 3U) / 4U)
#define MACRO_WORDS   ((DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE + 3U) / 4U)
#define PAYLOAD_WORDS (KEYMAP_WORDS + COMBO_WORDS + MACRO_WORDS)
#define SLOT_WORDS    (4U + PAYLOAD_WORDS)

#define FLASH_ERRORS (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_NOTZEROERR | FLASH_SR_FWWERR)
_Static_assert(DYNAMIC_KEYMAP_MAX_LAYERS == 4 && (NUM_KEYS == 60 || NUM_KEYS == 69), "HHKB EEPROM layout requires four layers and a supported matrix");
_Static_assert(2 * SLOT_WORDS * 4 <= 0xa00, "keymap slots overlap factory sensor data");

static uint32_t snapshot[PAYLOAD_WORDS];
static uint32_t generation, checksum;
static int active_slot = -1, target_slot = -1;
static unsigned stage;
static bool pending;

static volatile const uint32_t *slot(unsigned n) {
    return (volatile const uint32_t *)(SLOT_ADDRESS + n * SLOT_WORDS * 4U);
}
static volatile const uint32_t *slot_v1(unsigned n) {
    return (volatile const uint32_t *)(SLOT_ADDRESS + n * V1_SLOT_WORDS * 4U);
}
static uint32_t hash_word(uint32_t hash, uint32_t word) {
    for (unsigned n = 0; n < 4; n++) { hash ^= word & 0xffU; hash *= 16777619U; word >>= 8; }
    return hash;
}
static uint32_t slot_hash(volatile const uint32_t *p) {
    uint32_t h = hash_word(2166136261U, p[1]);
    for (unsigned i = 0; i < PAYLOAD_WORDS; i++) h = hash_word(h, p[4 + i]);
    return h;
}
static uint32_t slot_v1_hash(volatile const uint32_t *p) {
    uint32_t h = hash_word(2166136261U, p[1]);
    for (unsigned i = 0; i < V1_KEYMAP_WORDS; i++) h = hash_word(h, p[4 + i]);
    return h;
}
static bool valid(unsigned n) {
    volatile const uint32_t *p = slot(n);
    return p[0] == SLOT_MAGIC && p[3] == SLOT_COMMIT && p[2] == slot_hash(p);
}
static bool valid_v1(unsigned n) {
    volatile const uint32_t *p = slot_v1(n);
    return p[0] == SLOT_MAGIC_V1 && p[3] == SLOT_COMMIT && p[2] == slot_v1_hash(p);
}
static bool has_valid_v1(void) {
    return valid_v1(0) || valid_v1(1);
}
static bool writable(unsigned n) {
    volatile const uint32_t *p = slot(n);
    if (p[0] == SLOT_MAGIC) return true;
    if (n == 0 && p[0] == SLOT_MAGIC_V1) return true;
    if (valid(1 - n)) return true;
    if (has_valid_v1()) return true;
    for (unsigned i = 0; i < SLOT_WORDS; i++) if (p[i] != 0) return false;
    return true;
}
void vial_keyboard_keymap_load(void) {
    bool a = valid(0), b = valid(1);
    if (a && b) active_slot = (int32_t)(slot(1)[1] - slot(0)[1]) > 0 ? 1 : 0;
    else if (a) active_slot = 0;
    else if (b) active_slot = 1;
    else {
        bool v1_a = valid_v1(0), v1_b = valid_v1(1);
        if (!v1_a && !v1_b) return;
        int v1_slot = (v1_a && v1_b) ? ((int32_t)(slot_v1(1)[1] - slot_v1(0)[1]) > 0 ? 1 : 0) : (v1_a ? 0 : 1);
        volatile const uint32_t *p = slot_v1((unsigned)v1_slot);
        generation = p[1];
        uint32_t *dst = &dynamic_keymap[0][0];
        for (unsigned i = 0; i < KEYMAP_WORDS; i++) dst[i] = p[4 + i];
        memset(&vial_storage, 0, sizeof(vial_storage));
        pending = true;
        return;
    }
    volatile const uint32_t *p = slot((unsigned)active_slot);
    generation = p[1];
    uint32_t *dst = &dynamic_keymap[0][0];
    for (unsigned i = 0; i < KEYMAP_WORDS; i++) dst[i] = p[4 + i];

    uint8_t *combo_dst = (uint8_t *)vial_storage.data.combos;
    const volatile uint8_t *combo_src = (const volatile uint8_t *)&p[4 + KEYMAP_WORDS];
    for (size_t i = 0; i < sizeof(vial_storage.data.combos); i++) combo_dst[i] = combo_src[i];

    uint8_t *macro_dst = vial_storage.data.macro_buffer;
    const volatile uint8_t *macro_src = (const volatile uint8_t *)&p[4 + KEYMAP_WORDS + COMBO_WORDS];
    for (size_t i = 0; i < sizeof(vial_storage.data.macro_buffer); i++) macro_dst[i] = macro_src[i];
}
void vial_keyboard_keymap_save(void) { pending = true; }

static bool write_word(volatile uint32_t *dst, uint32_t value) {
    if (*dst == value) return true;
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEFU;
        FLASH->PEKEYR = 0x02030405U;
    }
    if (FLASH->PECR & FLASH_PECR_PELOCK) return false;
    while (FLASH->SR & FLASH_SR_BSY) {}
    FLASH->SR = FLASH_ERRORS | FLASH_SR_EOP;
    *dst = value;
    while (FLASH->SR & FLASH_SR_BSY) {}
    bool ok = !(FLASH->SR & FLASH_ERRORS) && *dst == value;
    FLASH->PECR |= FLASH_PECR_PELOCK;
    return ok;
}

bool hhkb_eeprom_clear_words(volatile uint32_t *dst, size_t words) {
    // Leave the ownership marker in place until the rest is cleared, so a retry is safe.
    while (words != 0)
        if (!write_word(&dst[--words], 0)) return false;
    return true;
}

bool vial_keyboard_keymap_reset(void) {
    pending = false;
    target_slot = -1;
    active_slot = -1;
    bool first_v2 = slot(0)[0] == SLOT_MAGIC_V2;
    bool second_v2 = slot(1)[0] == SLOT_MAGIC_V2;
    // The second legacy header lies inside the first current slot.
    bool first_v1 = slot_v1(0)[0] == SLOT_MAGIC_V1;
    bool second_v1 = !first_v2 && slot_v1(1)[0] == SLOT_MAGIC_V1;
    if (second_v2 && !hhkb_eeprom_clear_words((volatile uint32_t *)slot(1), SLOT_WORDS)) return false;
    if (second_v1 && !hhkb_eeprom_clear_words((volatile uint32_t *)slot_v1(1), V1_SLOT_WORDS)) return false;
    if (first_v2 && !hhkb_eeprom_clear_words((volatile uint32_t *)slot(0), SLOT_WORDS)) return false;
    if (first_v1 && !hhkb_eeprom_clear_words((volatile uint32_t *)slot_v1(0), V1_SLOT_WORDS)) return false;
    return true;
}
/* One EEPROM word per poll keeps the USB task responsive during a full save. */
void vial_keyboard_keymap_poll(void) {
    if (target_slot < 0) {
        if (!pending) return;
        pending = false;
        int candidate = active_slot < 0 ? 0 : 1 - active_slot;
        if (!writable((unsigned)candidate)) {
            if (active_slot >= 0 || !writable(1)) return;
            candidate = 1;
        }
        memset(snapshot, 0, sizeof(snapshot));
        memcpy(&snapshot[0], dynamic_keymap, sizeof(dynamic_keymap));
        memcpy(&snapshot[KEYMAP_WORDS], vial_storage.data.combos, sizeof(vial_storage.data.combos));
        memcpy(&snapshot[KEYMAP_WORDS + COMBO_WORDS], vial_storage.data.macro_buffer, sizeof(vial_storage.data.macro_buffer));
        generation++;
        checksum = hash_word(2166136261U, generation);
        for (unsigned i = 0; i < PAYLOAD_WORDS; i++) checksum = hash_word(checksum, snapshot[i]);
        target_slot = candidate;
        stage = 0;
    }
    volatile uint32_t *dst = (volatile uint32_t *)(SLOT_ADDRESS + (unsigned)target_slot * SLOT_WORDS * 4U);
    unsigned offset; uint32_t value;
    if (stage == 0) { offset = 3; value = 0; }
    else if (stage == 1) { offset = 0; value = SLOT_MAGIC; }
    else if (stage == 2) { offset = 1; value = generation; }
    else if (stage == 3) { offset = 2; value = checksum; }
    else if (stage < 4 + PAYLOAD_WORDS) { offset = stage; value = snapshot[stage - 4]; }
    else { offset = 3; value = SLOT_COMMIT; }
    if (!write_word(&dst[offset], value)) { target_slot = -1; return; }
    if (++stage == 5 + PAYLOAD_WORDS) {
        active_slot = target_slot;
        target_slot = -1;
    }
}

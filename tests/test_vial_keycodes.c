#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "keys.h"
#include "vial.h"

TickType_t g_mock_ticks = 0;
const uint32_t keymap[1][60] = {{0}};
const size_t keymap_layers = 1;

void bootloader_jump(void) {}
static unsigned settings_erases, keymap_erases, resets;
static bool reset_allowed = true;
bool vial_keyboard_reset_allowed(void) { return reset_allowed; }
bool vial_keyboard_settings_reset(void) { ++settings_erases; return true; }
bool vial_keyboard_keymap_reset(void) { ++keymap_erases; return true; }
void platform_system_reset(void) { ++resets; }
void hhkb_enter_bank1(void) { assert(0 && "settings reset must stay in DMK"); }
int16_t keyboard_get_flat_key_index(uint8_t row, uint8_t col) { (void)row; (void)col; return 0; }
void keyboard_send_key(uint16_t key, bool pressed) { (void)key; (void)pressed; }
static bool s_mock_matrix[16][32] = {{0}};
bool matrix_is_pressed(uint8_t row, uint8_t col) {
    if (row < 16 && col < 32) return s_mock_matrix[row][col];
    return false;
}

// Forward declarations from vial.c
uint16_t to_via_keycode(uint32_t dmk_key);
uint32_t from_via_keycode(uint16_t via_key);
void vial_process_packet(uint8_t const *request, uint8_t *response);

int main(void) {
    assert(to_via_keycode(MK(MOD_LCTRL|MOD_RSHIFT,K_A))==0);
    assert(to_via_keycode(HT(MK(MOD_LCTRL|MOD_RSHIFT,K_NULL),K_A))==0);
    assert(to_via_keycode(OS_MOD(MOD_LCTRL|MOD_RSHIFT))==0);

#ifdef HHKB_GRAVE_ESCAPE
    assert(from_via_keycode(0x7c16) == 0x7c16);
    assert(to_via_keycode(0x7c16) == 0x7c16);
#endif
    for (unsigned mods = 1; mods < 32; ++mods) {
        if (!(mods & 15)) continue;
        uint32_t key = from_via_keycode(0x52a0 | mods);
        assert((key & 0xff000000) == DMK_OS && (key & 0x10000));
        assert(to_via_keycode(key) == (0x52a0 | mods));
    }

    // VIA bit 4 selects one side, never both. Test all modifier masks.
    for (unsigned mods = 0; mods < 32; ++mods) {
        uint8_t expected = (mods & 0x10) ? ((mods & 0x0F) << 4) : (mods & 0x0F);
        if (mods != 0) {
            uint16_t encoded = (uint16_t)((mods << 8) | 0x33);
            assert(from_via_keycode(encoded) == (uint32_t)MK(expected, K_SCLN));
            if (mods & 0x0F) assert(to_via_keycode(from_via_keycode(encoded)) == encoded);
        }
        uint16_t mt = (uint16_t)(0x2000 | (mods << 8) | 0x33);
        assert(from_via_keycode(mt) == (uint32_t)HT(MK(expected,K_NULL), K_SCLN));
        if (mods & 0x0F) assert(to_via_keycode(from_via_keycode(mt)) == mt);
    }
    assert(to_via_keycode((DMK_HT_LEGACY | (MOD_RCTRL << 8) | K_APP)) == 0x3165); // Existing EEPROM encoding.
    assert(to_via_keycode(0x40001065) == 0x3165); // Prior dev format.
    assert(to_via_keycode(HT(K_RCTL, K_APP)) == 0x3165); // Legacy static keymap.
    for (unsigned layer = 0; layer < 16; ++layer) {
        uint16_t lt = 0x4000 | (layer << 8) | 0x33;
        assert(from_via_keycode(lt) == (uint32_t)HT(MO(layer), K_SCLN));
        assert(to_via_keycode(from_via_keycode(lt)) == lt);
    }
    // Layer keycodes (MO, TG, TO, DF, TT, OSL) roundtrip via the VIA ranges.
    for (unsigned layer = 0; layer < 16; ++layer) {
        assert(from_via_keycode(0x5220 | layer) == (uint32_t)MO(layer));
        assert(to_via_keycode(MO(layer)) == (0x5220 | layer));
        assert(from_via_keycode(0x5260 | layer) == (uint32_t)TG(layer));
        assert(to_via_keycode(TG(layer)) == (0x5260 | layer));
        assert(from_via_keycode(0x5200 | layer) == (uint32_t)TO(layer));
        assert(to_via_keycode(TO(layer)) == (0x5200 | layer));
        assert(from_via_keycode(0x5240 | layer) == (uint32_t)DF(layer));
        assert(to_via_keycode(DF(layer)) == (0x5240 | layer));
        assert(from_via_keycode(0x52C0 | layer) == (uint32_t)TT(layer));
        assert(to_via_keycode(TT(layer)) == (0x52C0 | layer));
        assert(from_via_keycode(0x5280 | layer) == (uint32_t)OS_LAYER(layer));
        assert(to_via_keycode(OS_LAYER(layer)) == (0x5280 | layer));
    }
    // Issue #6: LCTL(KC_SCLN) must not read back as RCTL(KC_SCLN).
    assert(from_via_keycode(0x0133) == MK(MOD_LCTRL, K_SCLN));
    assert(to_via_keycode(from_via_keycode(0x0133)) == 0x0133);
    assert(from_via_keycode(0x1133) == MK(MOD_RCTRL, K_SCLN));
    assert(to_via_keycode(from_via_keycode(0x1133)) == 0x1133);
    assert(from_via_keycode(0x0433) == MK(MOD_LALT, K_SCLN));
    assert(from_via_keycode(0x1433) == MK(MOD_RALT, K_SCLN));

    // Exercise the actual VIA write/read commands, not only conversion helpers.
    const uint16_t modified[] = {0x0133, 0x0233, 0x0433, 0x0833,
                                 0x32E7, 0x3165, 0x2104, 0x2204, 0x2404, 0x2804, 0x3E04, 0x3F04,
                                 0x1133, 0x1233, 0x1433, 0x1833, 0x0F33, 0x1E33, 0x1F33};
    for (unsigned i = 0; i < sizeof(modified) / sizeof(modified[0]); ++i) {
        uint8_t set[32] = {0x05, 0, 0, 0, modified[i] >> 8, modified[i] & 0xFF};
        uint8_t get[32] = {0x04, 0, 0, 0};
        uint8_t response[32] = {0};
        vial_process_packet(set, response);
        assert(dynamic_keymap[0][0] == from_via_keycode(modified[i]));
        vial_process_packet(get, response);
        assert(response[4] == (modified[i] >> 8) && response[5] == (modified[i] & 0xFF));
    }

    // 1. Media keys roundtrip
    assert(from_via_keycode(0x00A8) == K_MUTE);
    assert(to_via_keycode(K_MUTE) == 0x00A8);

    assert(from_via_keycode(0x00A9) == K_VOLU);
    assert(to_via_keycode(K_VOLU) == 0x00A9);

    assert(from_via_keycode(0x00AA) == K_VOLD);
    assert(to_via_keycode(K_VOLD) == 0x00AA);

    assert(from_via_keycode(0x00AB) == K_MNXT);
    assert(to_via_keycode(K_MNXT) == 0x00AB);

    assert(from_via_keycode(0x00AC) == K_MPRV);
    assert(to_via_keycode(K_MPRV) == 0x00AC);

    assert(from_via_keycode(0x00AD) == K_MSTP);
    assert(to_via_keycode(K_MSTP) == 0x00AD);

    assert(from_via_keycode(0x00AE) == K_MPLY);
    assert(to_via_keycode(K_MPLY) == 0x00AE);

    assert(from_via_keycode(0x00B2) == K_CALC);
    assert(to_via_keycode(K_CALC) == 0x00B2);

    assert(from_via_keycode(0x00BD) == K_BRIU);
    assert(to_via_keycode(K_BRIU) == 0x00BD);

    assert(from_via_keycode(0x00BE) == K_BRID);
    assert(to_via_keycode(K_BRID) == 0x00BE);

    // 2. System keys roundtrip
    assert(from_via_keycode(0x00A5) == K_PWR);
    assert(to_via_keycode(K_PWR) == 0x00A5);

    assert(from_via_keycode(0x00A6) == K_SLEP);
    assert(to_via_keycode(K_SLEP) == 0x00A6);

    assert(from_via_keycode(0x00A7) == K_WAKE);
    assert(to_via_keycode(K_WAKE) == 0x00A7);

    // 3. RGB keys must NEVER collide with media keys
    assert(from_via_keycode(0x7820) == K_RGB_TOGG);
    assert(to_via_keycode(K_RGB_TOGG) == 0x7820);

    assert(from_via_keycode(0x7821) == K_RGB_NEXT);
    assert(to_via_keycode(K_RGB_NEXT) == 0x7821);

    assert(from_via_keycode(0x7822) == K_RGB_PREV);
    assert(to_via_keycode(K_RGB_PREV) == 0x7822);

    assert(from_via_keycode(0x7823) == K_RGB_HUI);
    assert(to_via_keycode(K_RGB_HUI) == 0x7823);

    // Ensure K_RGB_* is definitely not in the media range 0x00A5..0x00BE
    assert(to_via_keycode(K_RGB_NEXT) != 0x00A8);
    assert(to_via_keycode(K_RGB_HUI) != 0x00AA);

    // 4. Bootloader
    assert(from_via_keycode(0x7C00) == K_BOOTLOADER);
    assert(from_via_keycode(0x5C00) == K_BOOTLOADER);
    assert(to_via_keycode(K_BOOTLOADER) == 0x7C00);

    // 5. Test Matrix State packing (VIA_SUB_SWITCH_MATRIX_STATE)
    // Press key at Row 1, Col 3 (Key '1' on HHKB: col in 0..7 range)
    s_mock_matrix[1][3] = true;
    uint8_t req[32] = {0x02, 0x03, 0x00}; // GET_KEYBOARD_VALUE, id_switch_matrix_state, row_offset=0
    uint8_t resp[32] = {0};
    vial_process_packet(req, resp);
    assert(resp[0] == 0x02);
    assert(resp[1] == 0x03);
    assert(resp[2] == 0x00); // Echoed offset
    // Row 0 has 2 bytes: resp[3], resp[4] (no keys pressed)
    assert(resp[3] == 0x00);
    assert(resp[4] == 0x00);
    // Row 1 has 2 bytes: resp[5] (high byte: cols 8..15), resp[6] (low byte: cols 0..7)
    // Col 3 is bit 3 of the low byte (1 << 3 = 0x08)
    assert(resp[5] == 0x00);
    assert(resp[6] == 0x08);

    // Also press key at Row 1, Col 11 (Key '8' on HHKB: col in 8..15 range -> bit 3 of high byte)
    s_mock_matrix[1][11] = true;
    memset(resp, 0, sizeof(resp));
    vial_process_packet(req, resp);
    assert(resp[5] == 0x08); // High byte has bit (11 - 8) = bit 3 set
    assert(resp[6] == 0x08); // Low byte still has bit 3 set

    memset(req, 0, sizeof(req));
    req[0] = 0x07;
    req[2] = 38; // Reset button
    req[3] = 1;
    vial_process_packet(req, resp);
    assert(resp[0] == 0xFF);
    req[2] = 37; // Arm reset
    vial_process_packet(req, resp);
    assert(resp[0] == 0x07);
    req[2] = 38;
    reset_allowed = false;
    vial_process_packet(req, resp);
    assert(resp[0] == 0xFF);
    vial_eeprom_task();
    assert(settings_erases == 0 && keymap_erases == 0 && resets == 0);
    reset_allowed = true;
    req[2] = 37;
    vial_process_packet(req, resp);
    req[2] = 38;
    vial_process_packet(req, resp);
    assert(resp[0] == 0x07);
    vial_eeprom_task();
    assert(settings_erases == 0 && keymap_erases == 0 && resets == 0);
    g_mock_ticks += 150;
    vial_eeprom_task();
    assert(settings_erases == 1 && keymap_erases == 1 && resets == 1);

    puts("vial keycode translation tests passed");
    return 0;
}

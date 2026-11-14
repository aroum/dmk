#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "vial.h"
#include "stm32l0xx.h"

flash_t flash;
flash_t *FLASH = &flash;
uint32_t dynamic_keymap[4][60];
vial_storage_t vial_storage;
static uint32_t mock_eeprom[768];
#define SLOT_ADDRESS ((uintptr_t)mock_eeprom)
#include "../../keyboards/hhkb_classic/modules/via_settings/hhkb_keymap_storage.c"

static void restart(void) {
    active_slot = -1;
    target_slot = -1;
    generation = 0;
    stage = 0;
    pending = false;
    memset(dynamic_keymap, 0, sizeof dynamic_keymap);
    memset(&vial_storage, 0, sizeof vial_storage);
    vial_keyboard_keymap_load();
}
static void poll_all(void) {
    for (int i = 0; i < 400; i++) vial_keyboard_keymap_poll();
}
int main(void) {
    dynamic_keymap[2][23] = 0x12345678;
    vial_keyboard_keymap_save();
    poll_all();
    restart();
    assert(dynamic_keymap[2][23] == 0x12345678);

    dynamic_keymap[2][23] = 77;
    vial_keyboard_keymap_save();
    for (int i = 0; i < 20; i++) vial_keyboard_keymap_poll();
    restart();
    assert(dynamic_keymap[2][23] == 0x12345678);

    dynamic_keymap[2][23] = 99;
    vial_keyboard_keymap_save();
    poll_all();
    restart();
    assert(dynamic_keymap[2][23] == 99);

    dynamic_keymap[0][0] = 123;
    vial_keyboard_keymap_save();
    for (int i = 0; i < 30; i++) vial_keyboard_keymap_poll();
    dynamic_keymap[0][0] = 456;
    vial_keyboard_keymap_save();
    poll_all();
    poll_all();
    restart();
    assert(dynamic_keymap[0][0] == 456);

    // Verify combos and macros persistence
    vial_storage.data.combos[0].input[0] = 0x0004; // Key 'A'
    vial_storage.data.combos[0].input[1] = 0x0005; // Key 'B'
    vial_storage.data.combos[0].output = 0x0028;   // Enter
    vial_storage.data.macro_buffer[0] = 0x01;      // Down action
    vial_storage.data.macro_buffer[1] = 0x04;      // Key 'A'
    vial_storage.data.macro_buffer[2] = 0x02;      // Up action
    vial_storage.data.macro_buffer[3] = 0x04;      // Key 'A'
    vial_storage.data.macro_buffer[4] = 0x00;      // End of macro
    vial_keyboard_keymap_save();
    poll_all();
    restart();
    assert(vial_storage.data.combos[0].input[0] == 0x0004);
    assert(vial_storage.data.combos[0].input[1] == 0x0005);
    assert(vial_storage.data.combos[0].output == 0x0028);
    assert(vial_storage.data.macro_buffer[0] == 0x01);
    assert(vial_storage.data.macro_buffer[1] == 0x04);
    assert(vial_storage.data.macro_buffer[2] == 0x02);
    assert(vial_storage.data.macro_buffer[3] == 0x04);
    assert(vial_storage.data.macro_buffer[4] == 0x00);

    memset(mock_eeprom, 0, sizeof mock_eeprom);
    mock_eeprom[12] = 0x1234;
    mock_eeprom[SLOT_WORDS + 12] = 0x5678;
    restart();
    dynamic_keymap[0][0] = 1;
    vial_keyboard_keymap_save();
    poll_all();
    assert(mock_eeprom[12] == 0x1234 && mock_eeprom[SLOT_WORDS + 12] == 0x5678);
    assert(active_slot == -1);
    return 0;
}

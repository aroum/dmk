#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "keys.h"

TickType_t g_mock_ticks = 0;
const uint32_t keymap[1][60] = {{0}};
const size_t keymap_layers = 1;

void bootloader_jump(void) {}
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

    puts("vial keycode translation tests passed");
    return 0;
}

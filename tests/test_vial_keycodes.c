#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "keys.h"
#include "vial.h"
#include "app_usb_hid.h"
#include "usb.h"

TickType_t g_mock_ticks = 0;
const uint32_t keymap[1][NUM_KEYS] = {{0}};
const size_t keymap_layers = 1;

void bootloader_jump(void) {}
static unsigned settings_erases, keymap_erases, resets;
static bool reset_allowed = true;
bool vial_keyboard_reset_allowed(void) { return reset_allowed; }
bool vial_keyboard_settings_reset(void) { ++settings_erases; return true; }
bool vial_keyboard_keymap_reset(void) { ++keymap_erases; return true; }
void platform_system_reset(void) { ++resets; }
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

static USB_HID_KeyboardReport_TypeDef last_report;
static unsigned report_count;
bool USB_HID_Init(void) { return true; }
bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    last_report = *report;
    ++report_count;
    return true;
}
bool USB_HID_SendConsumerReport(uint16_t usage) { (void)usage; return true; }

static void test_modified_keycodes(void) {
    // VIA uses four modifier bits plus a side flag, not an eight-bit HID mask.
    // Exercise all 15 combinations on each side, including LGUI(Right), LALT
    // and high right-side masks that overlap HID modifier keycode numbers.
    for (unsigned right = 0; right < 2; ++right) {
        for (unsigned bits = 1; bits < 16; ++bits) {
            uint8_t mods = (uint8_t)(bits << (right ? 4 : 0));
            uint16_t via = (uint16_t)(((bits | (right ? 0x10 : 0)) << 8) | K_RGHT);
            uint32_t dmk = MK(mods, K_RGHT);
            assert(from_via_keycode(via) == dmk);
            assert(to_via_keycode(dmk) == via);

            // Both VIA keymap write paths must store the exact HID mask.
            uint8_t req[32] = {0x05, 0, 0, 0, via >> 8, via & 0xFF};
            uint8_t resp[32] = {0};
            vial_process_packet(req, resp);
            assert(dynamic_keymap[0][0] == dmk);
            req[0] = 0x04;
            vial_process_packet(req, resp);
            assert(resp[4] == (via >> 8) && resp[5] == (via & 0xFF));

            req[0] = 0x13; req[3] = 2; // SET_BUFFER, offset 0, size 2
            dynamic_keymap[0][0] = K_NULL;
            vial_process_packet(req, resp);
            assert(dynamic_keymap[0][0] == dmk);
            req[0] = 0x12; // GET_BUFFER
            vial_process_packet(req, resp);
            assert(resp[4] == (via >> 8) && resp[5] == (via & 0xFF));

            // Exercise the production USB report path used by DMK_MK dispatch.
            usb_init();
            report_count = 0;
            uint32_t stored = dynamic_keymap[0][0];
            usb_process_key_with_modifiers((stored >> 8) & 0xFF, stored & 0xFF, true);
            assert(report_count == 1);
            assert(last_report.Modifier == mods);
            assert(last_report.Keycodes[0] == K_RGHT);
            usb_process_key_with_modifiers((stored >> 8) & 0xFF, stored & 0xFF, false);
            assert(report_count == 2 && last_report.Modifier == 0);
            for (unsigned i = 0; i < 6; ++i) assert(last_report.Keycodes[i] == 0);
        }
    }
    // A mask-free MK must not silently gain Ctrl on readback.
    assert(to_via_keycode(MK(0, K_RGHT)) == K_RGHT);
    assert(from_via_keycode(K_LGUI) == K_LGUI);
    assert(from_via_keycode(K_RGUI) == K_RGUI);
    assert(from_via_keycode(K_LALT) == K_LALT);
    assert(from_via_keycode(K_RALT) == K_RALT);
}

int main(void) {
    test_modified_keycodes();
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

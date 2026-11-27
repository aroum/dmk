// Exercise VIA translation, the real hold-tap state machine and USB reports.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "keys.h"
#include "vial.h"
#include "hold_tap.h"
#include "usb.h"
#include "app_usb_hid.h"

TickType_t g_mock_ticks;
const uint32_t keymap[1][NUM_KEYS] = {{0}};
const size_t keymap_layers = 1;
bool vial_keyboard_settings_reset(void) { assert(false); return false; }
bool vial_keyboard_keymap_reset(void) { assert(false); return false; }
void bootloader_jump(void) { assert(false); }
void platform_system_reset(void) { assert(false); }
int16_t keyboard_get_flat_key_index(uint8_t r, uint8_t c) { (void)r; (void)c; return 0; }
bool matrix_is_pressed(uint8_t r, uint8_t c) { (void)r; (void)c; return false; }
static unsigned layer_state, lazy_mod_calls, tap_calls;
void layers_on(uint8_t layer) { layer_state |= 1U << layer; }
void layers_off(uint8_t layer) { layer_state &= ~(1U << layer); }
void oneshot_send_lazy_mods(void) { ++lazy_mod_calls; }
void oneshot_on_tap_key(void) { ++tap_calls; }
void keyboard_send_key(uint16_t key, bool down) { usb_process_key(key, down); }
static uint16_t settings[4];
uint16_t hold_tap_config_value(uint8_t id) { return settings[id]; }
static USB_HID_KeyboardReport_TypeDef reports[16], last_report;
static unsigned report_count;
bool USB_HID_Init(void) { return true; }
bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    assert(report_count < 16);
    reports[report_count++] = last_report = *report;
    return true;
}
bool USB_HID_SendConsumerReport(uint16_t usage) { (void)usage; assert(false); return false; }
uint16_t to_via_keycode(uint32_t key);
uint32_t from_via_keycode(uint16_t key);
void vial_process_packet(const uint8_t *request, uint8_t *response);

static void reset(void) {
    usb_init();
    hold_tap_init();
    memset(reports, 0, sizeof(reports));
    memset(&last_report, 0, sizeof(last_report));
    report_count = layer_state = lazy_mod_calls = tap_calls = 0;
    settings[0] = 220; settings[1] = 0; settings[2] = 0; settings[3] = 150;
    g_mock_ticks = 1000;
}

static void expect_report(uint8_t mods, uint8_t key) {
    assert(last_report.Modifier == mods);
    assert(last_report.Keycodes[0] == key);
    for (unsigned i = 1; i < 6; ++i) assert(last_report.Keycodes[i] == 0);
}

static void event(unsigned col, uint32_t key, bool down) {
    assert(hold_tap_process_event(0, col, key, down));
}

static uint32_t store_via(uint16_t via) {
    uint8_t req[32] = {0x05, 0, 0, 0, via >> 8, via & 0xFF}, resp[32] = {0};
    vial_process_packet(req, resp);
    uint32_t key = dynamic_keymap[0][0];
    assert(to_via_keycode(key) == via);
    req[0] = 0x04;
    vial_process_packet(req, resp);
    assert(resp[4] == (via >> 8) && resp[5] == (via & 0xFF));
    req[0] = 0x13; req[3] = 2;
    dynamic_keymap[0][0] = K_NULL;
    vial_process_packet(req, resp);
    assert(dynamic_keymap[0][0] == key);
    req[0] = 0x12;
    vial_process_packet(req, resp);
    assert(resp[4] == (via >> 8) && resp[5] == (via & 0xFF));
    return key;
}

static void test_via_modifiers(void) {
    const uint8_t taps[] = {K_LEFT, K_Z, K_LANG2};
    for (unsigned side = 0; side < 2; ++side) {
        for (unsigned bits = 1; bits < 16; ++bits) {
            uint8_t mods = bits << (side ? 4 : 0);
            for (unsigned t = 0; t < sizeof(taps); ++t) {
                reset();
                uint16_t via = 0x2000 | ((bits | (side ? 0x10 : 0)) << 8) | taps[t];
                uint32_t key = store_via(via);
                assert(key == MT(mods, taps[t]));
                event(0, key, true);
                assert(report_count == 0 && hold_tap_has_active());
                g_mock_ticks += 219;
                hold_tap_check_timeouts(g_mock_ticks);
                event(0, key, false);
                assert(report_count == 2 && reports[0].Modifier == 0);
                assert(reports[0].Keycodes[0] == taps[t]);
                expect_report(0, 0);
                assert(lazy_mod_calls == 1 && tap_calls == 1 && layer_state == 0);

                reset();
                event(0, key, true);
                g_mock_ticks += 220;
                hold_tap_check_timeouts(g_mock_ticks);
                assert(report_count == 1 && !hold_tap_has_active());
                expect_report(mods, 0);
                event(0, key, false);
                assert(report_count == 2 && tap_calls == 0 && layer_state == 0);
                expect_report(0, 0);
            }
        }
    }
}

static void test_explicit_masks_and_legacy(void) {
    // Raw masks must never become layer numbers or HID usages (notably 0xE0).
    for (unsigned mods = 0; mods <= 255; ++mods) {
        reset();
        uint32_t key = MT(mods, K_LEFT);
        event(0, key, true);
        g_mock_ticks += 220;
        hold_tap_check_timeouts(g_mock_ticks);
        expect_report(mods, 0);
        assert(layer_state == 0);
        event(0, key, false);
        expect_report(0, 0);
    }
    for (unsigned m = 0; m < 8; ++m) {
        reset();
        uint32_t key = HT(K_LCTL + m, K_Z);
        assert(to_via_keycode(key) == (0x2000 | ((m < 4 ? 1U << m : 0x10 | (1U << (m - 4))) << 8) | K_Z));
        event(0, key, true);
        g_mock_ticks += 220;
        hold_tap_check_timeouts(g_mock_ticks);
        expect_report(1U << m, 0);
        event(0, key, false);
        expect_report(0, 0);
    }
    assert(to_via_keycode(HT(MOD_RCTRL, K_Z)) == 0x311D);
    for (unsigned layer = 0; layer < 16; ++layer) {
        reset();
        uint16_t via = 0x4000 | (layer << 8) | K_Z;
        uint32_t key = store_via(via);
        event(0, key, true);
        g_mock_ticks += 220;
        hold_tap_check_timeouts(g_mock_ticks);
        assert(layer_state == (1U << layer) && report_count == 0);
        event(0, key, false);
        assert(layer_state == 0 && report_count == 0);
    }
}

static void test_timing(void) {
    uint32_t key = from_via_keycode(0x2450); // MT(MOD_LALT, KC_LEFT)
    reset(); settings[1] = 1;
    event(0, key, true);
    hold_tap_permissive_resolve(0, 0);
    assert(report_count == 0);
    hold_tap_permissive_resolve(0, 1);
    expect_report(MOD_LALT, 0);
    event(0, key, false); expect_report(0, 0);

    reset(); settings[1] = settings[2] = 1;
    event(0, key, true);
    hold_tap_permissive_resolve(0, 1);
    assert(report_count == 0);
    g_mock_ticks += 10; event(0, key, false);
    assert(report_count == 2 && reports[0].Modifier == 0);

    reset();
    event(0, key, true);
    g_mock_ticks += 50; event(0, key, false);
    g_mock_ticks += 50; event(0, key, true);
    expect_report(0, K_LEFT);
    g_mock_ticks += 500; hold_tap_check_timeouts(g_mock_ticks);
    expect_report(0, K_LEFT);
    event(0, key, false); expect_report(0, 0);

    reset();
    key = MT_T(MOD_RALT, K_LEFT, 80);
    event(0, key, true);
    g_mock_ticks += 79; hold_tap_check_timeouts(g_mock_ticks);
    assert(report_count == 0);
    ++g_mock_ticks; hold_tap_check_timeouts(g_mock_ticks);
    expect_report(MOD_RALT, 0);
    event(0, key, false); expect_report(0, 0);

    reset(); g_mock_ticks = UINT32_MAX - 50;
    event(0, key, true);
    g_mock_ticks += 80;
    // No timer service before release: still a hold, never an arrow tap.
    event(0, key, false);
    assert(report_count == 2 && reports[0].Modifier == MOD_RALT);
    assert(reports[0].Keycodes[0] == 0);
    expect_report(0, 0);
}

static void test_overlapping_modifiers(void) {
    uint32_t alt = MT(MOD_LALT, K_LEFT), gui = MT(MOD_RGUI, K_Z);
    reset(); settings[1] = 1;
    event(0, alt, true); event(1, alt, true); event(2, gui, true);
    hold_tap_permissive_resolve(0, 3);
    expect_report(MOD_LALT | MOD_RGUI, 0);
    event(0, alt, false); expect_report(MOD_LALT | MOD_RGUI, 0);
    event(2, gui, false); expect_report(MOD_LALT, 0);
    event(1, alt, false); expect_report(0, 0);

    // Either release order must preserve the other owner of the same modifier.
    for (unsigned physical_first = 0; physical_first < 2; ++physical_first) {
        reset();
        usb_process_key(K_LALT, true);
        event(0, alt, true);
        g_mock_ticks += 220; hold_tap_check_timeouts(g_mock_ticks);
        if (physical_first) usb_process_key(K_LALT, false);
        else event(0, alt, false);
        expect_report(MOD_LALT, 0);
        if (physical_first) event(0, alt, false);
        else usb_process_key(K_LALT, false);
        expect_report(0, 0);
    }
    reset();
    event(0, alt, true);
    g_mock_ticks += 220; hold_tap_check_timeouts(g_mock_ticks);
    usb_process_key_with_modifiers(MOD_LALT, K_Z, true);
    event(0, alt, false); expect_report(MOD_LALT, K_Z);
    usb_process_key_with_modifiers(MOD_LALT, K_Z, false); expect_report(0, 0);
    reset();
    event(0, alt, true);
    g_mock_ticks += 220; hold_tap_check_timeouts(g_mock_ticks);
    hold_tap_init(); expect_report(0, 0);
}

int main(void) {
    test_via_modifiers();
    test_explicit_masks_and_legacy();
    test_timing();
    test_overlapping_modifiers();
    puts("VIA mod-tap translation, timing and USB reports passed");
}

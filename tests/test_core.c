#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// Include mocks
#include "mocks/FreeRTOS.h"
#include "mocks/queue.h"
#include "mocks/task.h"
#include "mocks/hal_gpio.h"

// Global mock tick counter
TickType_t g_mock_ticks = 0;

// Test event recording structures
typedef struct {
    uint16_t keycode;
    bool pressed;
} key_event_record_t;

typedef struct {
    uint8_t mod_mask;
    bool pressed;
} mod_event_record_t;

#define MAX_RECORDS 64
static key_event_record_t s_recorded_keys[MAX_RECORDS];
static size_t s_recorded_keys_count = 0;

static mod_event_record_t s_recorded_mods[MAX_RECORDS];
static size_t s_recorded_mods_count = 0;

void mock_reset_records(void) {
    s_recorded_keys_count = 0;
    s_recorded_mods_count = 0;
    g_mock_ticks = 0;
}

void keyboard_send_key(uint16_t keycode, bool pressed) {
    if (s_recorded_keys_count < MAX_RECORDS) {
        s_recorded_keys[s_recorded_keys_count++] = (key_event_record_t){keycode, pressed};
    }
}

void keyboard_send_modifiers(uint8_t mod_mask, bool pressed) {
    if (s_recorded_mods_count < MAX_RECORDS) {
        s_recorded_mods[s_recorded_mods_count++] = (mod_event_record_t){mod_mask, pressed};
    }
}

void oneshot_send_lazy_mods(void) {}
void oneshot_on_tap_key(void) {}
void hook_layer_change(uint8_t layer) { (void)layer; }
void macros_run_layer_triggers(uint8_t layer) { (void)layer; }
void hook_key_sent(uint16_t keycode, bool pressed) { (void)keycode; (void)pressed; }

// --- Test 1: Hold-Tap and Priority Modes ---
#include "keys.h"
#include "hold_tap.h"
#include "hold_tap.c"

static void test_hold_tap_tap_before_timeout(void) {
    mock_reset_records();
    hold_tap_init();

    // HT key: Hold = K_LCTL (0xE0), Tap = K_A
    uint32_t ht_key = HT(K_LCTL, K_A);

    // Press key at t=100
    g_mock_ticks = 100;
    bool handled = hold_tap_process_event(0, 0, ht_key, true);
    assert(handled == true);
    assert(hold_tap_has_active() == true);
    assert(s_recorded_keys_count == 0);
    assert(s_recorded_mods_count == 0);

    // Release key at t=150 (elapsed 50ms < 200ms TAPPING_TERM)
    g_mock_ticks = 150;
    handled = hold_tap_process_event(0, 0, ht_key, false);
    assert(handled == true);
    assert(hold_tap_has_active() == false);

    // Verify tap event: K_A pressed and released
    assert(s_recorded_keys_count == 2);
    assert(s_recorded_keys[0].keycode == K_A && s_recorded_keys[0].pressed == true);
    assert(s_recorded_keys[1].keycode == K_A && s_recorded_keys[1].pressed == false);
    assert(s_recorded_mods_count == 0);

    printf("  ✔ test_hold_tap_tap_before_timeout passed\n");
}

static void test_hold_tap_hold_after_timeout(void) {
    mock_reset_records();
    hold_tap_init();

    // Mod-Tap: Hold = K_LCTL (0xE0), Tap = K_A
    uint32_t ht_key = HT(K_LCTL, K_A);

    // Press key at t=100
    g_mock_ticks = 100;
    hold_tap_process_event(0, 0, ht_key, true);

    // Advance time to t=301 (> 200ms)
    g_mock_ticks = 301;
    TickType_t next_deadline = hold_tap_check_timeouts(g_mock_ticks);
    (void)next_deadline;

    // Verify modifier activated (target >= 16 sent to keyboard_send_modifiers)
    assert(s_recorded_mods_count == 1);
    assert(s_recorded_mods[0].mod_mask == K_LCTL && s_recorded_mods[0].pressed == true);
    assert(s_recorded_keys_count == 0);

    // Release key at t=400
    g_mock_ticks = 400;
    hold_tap_process_event(0, 0, ht_key, false);

    // Verify modifier released
    assert(s_recorded_mods_count == 2);
    assert(s_recorded_mods[1].mod_mask == K_LCTL && s_recorded_mods[1].pressed == false);
    assert(s_recorded_keys_count == 0);

    printf("  ✔ test_hold_tap_hold_after_timeout passed\n");
}

static void test_hold_tap_layer_tap(void) {
    mock_reset_records();
    hold_tap_init();
    layers_init();

    // Layer-Tap: Hold = Layer 1 (< 16), Tap = K_B
    uint32_t lt_key = HT(1, K_B);

    // Press key at t=100
    g_mock_ticks = 100;
    hold_tap_process_event(0, 1, lt_key, true);

    // Advance time to t=301 (> 200ms)
    g_mock_ticks = 301;
    hold_tap_check_timeouts(g_mock_ticks);

    // Layer 1 should now be active
    assert(layers_get_active() == 1);

    // Release key at t=400
    g_mock_ticks = 400;
    hold_tap_process_event(0, 1, lt_key, false);

    // Layer 1 should be turned off, returning to Layer 0
    assert(layers_get_active() == 0);

    printf("  ✔ test_hold_tap_layer_tap passed\n");
}

static void test_hold_tap_custom_timeout(void) {
    mock_reset_records();
    hold_tap_init();

    // HT key with individual timeout of 80ms
    uint32_t ht_key = HT_T(K_LSFT, K_B, 80);

    // Press key at t=100
    g_mock_ticks = 100;
    hold_tap_process_event(0, 1, ht_key, true);

    // At t=150 (50ms elapsed): should NOT be hold yet
    g_mock_ticks = 150;
    hold_tap_check_timeouts(g_mock_ticks);
    assert(s_recorded_mods_count == 0);

    // At t=185 (85ms elapsed > 80ms): should be HOLD
    g_mock_ticks = 185;
    hold_tap_check_timeouts(g_mock_ticks);
    assert(s_recorded_mods_count == 1);
    assert(s_recorded_mods[0].mod_mask == K_LSFT && s_recorded_mods[0].pressed == true);

    // Release at t=200
    g_mock_ticks = 200;
    hold_tap_process_event(0, 1, ht_key, false);
    assert(s_recorded_mods_count == 2);
    assert(s_recorded_mods[1].mod_mask == K_LSFT && s_recorded_mods[1].pressed == false);

    printf("  ✔ test_hold_tap_custom_timeout passed\n");
}

// --- Test 2: Layers Logic ---
#define DEFINE_KEYMAP
#include "mocks/config.h"

// Define a test keymap with 3 layers
const uint32_t keymap[3][NUM_KEYS] = {
    // Layer 0 (Base): A, B, C, D, E, F
    [0] = { K_A, K_B, K_C, K_D, K_E, K_F },
    // Layer 1: 1, TRNS, 3, TRNS, 5, 6
    [1] = { K_1, K_TRNS, K_3, K_TRNS, K_5, K_6 },
    // Layer 2: TRNS, TRNS, K_X, TRNS, TRNS, TRNS
    [2] = { K_TRNS, K_TRNS, K_X, K_TRNS, K_TRNS, K_TRNS }
};
const size_t keymap_layers = 3;

#include "layers.h"
#include "layers.c"

static void test_layers_stack_and_fallthrough(void) {
    layers_init();

    assert(layers_get_active() == 0);
    assert(layers_get_state() == 1); // Bit 0 active

    // Layer 0 lookups
    assert(layers_lookup_key(0, 0) == K_A);
    assert(layers_lookup_key(0, 1) == K_B);
    assert(layers_lookup_key(0, 2) == K_C);

    // Turn on Layer 1
    layers_on(1);
    assert(layers_get_active() == 1);
    assert(layers_lookup_key(0, 0) == K_1); // Overridden by Layer 1
    assert(layers_lookup_key(0, 1) == K_B); // K_TRNS falls through to Layer 0

    // Turn on Layer 2 (higher priority)
    layers_on(2);
    assert(layers_get_active() == 2);
    assert(layers_lookup_key(0, 2) == K_X); // Layer 2 overrides Layer 1 and 0
    assert(layers_lookup_key(0, 0) == K_1); // K_TRNS on L2 falls through to L1 K_1
    assert(layers_lookup_key(0, 1) == K_B); // K_TRNS on L2 and L1 falls through to L0 K_B

    // Turn off Layer 2
    layers_off(2);
    assert(layers_get_active() == 1);
    assert(layers_lookup_key(0, 2) == K_3); // Restores Layer 1

    // Toggle Layer 1 off
    layers_toggle(1);
    assert(layers_get_active() == 0);
    assert(layers_lookup_key(0, 0) == K_A); // Restores Layer 0

    // Base layer safety: turn off all layers, layer 0 must remain active
    layers_off(0);
    assert(layers_get_active() == 0);
    assert(layers_get_state() == 1);

    printf("  ✔ test_layers_stack_and_fallthrough passed\n");
}

// --- Test 3: USB HID Report Engine (Zero-Queue) ---
#include "app_usb_hid.h"

static USB_HID_KeyboardReport_TypeDef s_last_sent_report;
static size_t s_send_report_count = 0;
static uint16_t s_last_consumer_report = 0;

bool USB_HID_Init(void) { return true; }
bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    s_last_sent_report = *report;
    s_send_report_count++;
    return true;
}
bool USB_HID_SendConsumerReport(uint16_t usage) {
    s_last_consumer_report = usage;
    return true;
}

#include "usb.h"
#include "usb.c"

static void test_usb_hid_reports_and_zero_queue(void) {
    usb_init();
    s_send_report_count = 0;

    // Press K_A
    usb_process_key(K_A, true);
    assert(s_send_report_count == 1);
    assert(s_last_sent_report.Keycodes[0] == K_A);
    assert(s_last_sent_report.Modifier == 0);

    // Press K_B (should add to report without clearing K_A)
    usb_process_key(K_B, true);
    assert(s_send_report_count == 2);
    assert(s_last_sent_report.Keycodes[0] == K_A);
    assert(s_last_sent_report.Keycodes[1] == K_B);

    // Press duplicate K_A (should not generate redundant USB report)
    usb_process_key(K_A, true);
    assert(s_send_report_count == 2);

    // Release K_A
    usb_process_key(K_A, false);
    assert(s_send_report_count == 3);
    assert(s_last_sent_report.Keycodes[0] == 0);
    assert(s_last_sent_report.Keycodes[1] == K_B);

    // Atomic composite key with modifiers (Zero-Queue batching: e.g. Ctrl+Shift+C)
    uint8_t mods = MOD_LCTRL | MOD_LSHIFT;
    usb_process_key_with_modifiers(mods, K_C, true);
    assert(s_last_sent_report.Modifier == mods);
    // Find K_C in keycodes array
    bool found_c = false;
    for (int i = 0; i < 6; i++) {
        if (s_last_sent_report.Keycodes[i] == K_C) found_c = true;
    }
    assert(found_c == true);

    // Release composite key
    usb_process_key_with_modifiers(mods, K_C, false);
    assert(s_last_sent_report.Modifier == 0);
    found_c = false;
    for (int i = 0; i < 6; i++) {
        if (s_last_sent_report.Keycodes[i] == K_C) found_c = true;
    }
    assert(found_c == false);

    printf("  ✔ test_usb_hid_reports_and_zero_queue passed\n");
}

int main(void) {
    printf("\n=== Running DMK Host-Based C Core Unit Tests ===\n\n");

    printf("[1/3] Testing Hold-Tap subsystem...\n");
    test_hold_tap_tap_before_timeout();
    test_hold_tap_hold_after_timeout();
    test_hold_tap_layer_tap();
    test_hold_tap_custom_timeout();

    printf("\n[2/3] Testing Layers subsystem...\n");
    test_layers_stack_and_fallthrough();

    printf("\n[3/3] Testing USB HID Zero-Queue subsystem...\n");
    test_usb_hid_reports_and_zero_queue();

    printf("\n🎉 All C core unit tests passed successfully!\n\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define hooks_dispatch_layer_change hook_layer_change
#define hooks_dispatch_midi_receive hook_midi_receive

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
static unsigned s_raw_mod_calls;

void mock_reset_records(void) {
    s_recorded_keys_count = 0;
    s_recorded_mods_count = 0;
    s_raw_mod_calls = 0;
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

void keyboard_send_modifier_mask(uint8_t mask, bool pressed) {
    ++s_raw_mod_calls;
    keyboard_send_modifiers(mask, pressed);
}

void oneshot_send_lazy_mods(void) {}
void oneshot_on_tap_key(void) {}
void hook_layer_change(uint8_t layer) { (void)layer; }
void macros_run_layer_triggers(uint8_t layer) { (void)layer; }
void hook_key_sent(uint16_t keycode, bool pressed) { (void)keycode; (void)pressed; }

// --- Test 1: Hold-Tap and Priority Modes ---
#define QUICK_TAP_TERM 150
#include "keys.h"
#include "hold_tap.h"
#include "hold_tap.c"


static void test_unified_hold_tap_actions(void) {
    for(unsigned h=0;h<256;++h)for(unsigned t=0;t<256;++t){
        uint32_t hold,tap;uint8_t ms;
        assert(ht_decode(HT(KEY(h),t),&hold,&tap,&ms));assert(hold==h && tap==t && !ms);
    }
    const uint32_t actions[]={K_A,K_RSFT,MO(0),MO(15),TG(2),MK(0xff,K_NULL)};
    for(unsigned h=0;h<6;++h)for(unsigned t=0;t<6;++t)for(unsigned n=0;n<256;++n){
        uint32_t hold,tap;uint8_t ms;
        uint32_t hold_action=actions[h]<=0xff?KEY(actions[h]):actions[h];
        assert(ht_decode(HT_T(hold_action,actions[t],n),&hold,&tap,&ms));
        assert(hold==actions[h] && tap==actions[t] && ms==n);
    }
    assert(HT(K_VOLU,K_A)==K_NULL);
    mock_reset_records();hold_tap_init();layers_init();
    uint32_t key=HT(K_A,K_B); // HID usage 4 must never mean layer 4.
    hold_tap_process_event(0,0,key,true);g_mock_ticks=250;hold_tap_check_timeouts(g_mock_ticks);
    assert(s_recorded_keys_count==1 && s_recorded_keys[0].keycode==K_A && s_recorded_keys[0].pressed);
    assert(layers_get_state()==1);hold_tap_process_event(0,0,key,false);
    assert(s_recorded_keys_count==2 && !s_recorded_keys[1].pressed);
    mock_reset_records();hold_tap_init();layers_init();
    key=HT(K_A,TG(1));
    hold_tap_process_event(0,0,key,true);g_mock_ticks=20;hold_tap_process_event(0,0,key,false);
    assert(layers_get_active()==1 && s_recorded_keys_count==0);
    mock_reset_records();hold_tap_init();layers_init();
    key=HT(MO(1),TG(2));hold_tap_process_event(0,0,key,true);
    g_mock_ticks=250;hold_tap_check_timeouts(g_mock_ticks);assert(layers_get_active()==1);
    hold_tap_process_event(0,0,key,false);assert(layers_get_active()==0);
    mock_reset_records();hold_tap_init();
    hold_tap_process_event(0,0,key,true);g_mock_ticks=20;hold_tap_process_event(0,0,key,false);
    assert(layers_get_active()==2);
    mock_reset_records();hold_tap_init();
    key=HT(K_A,K_B);hold_tap_process_event(0,0,key,true);g_mock_ticks=20;
    hold_tap_process_event(0,0,HT(K_C,K_D),false);
    assert(s_recorded_keys_count==2 && s_recorded_keys[0].keycode==K_B && s_recorded_keys[1].keycode==K_B);
    // Both prior EEPROM formats decode into the same generic actions.
    const uint32_t legacy[]={0x04001065,0x40001065};
    for(unsigned i=0;i<2;++i){
        mock_reset_records();hold_tap_init();
        hold_tap_process_event(0,0,legacy[i],true);g_mock_ticks=250;hold_tap_check_timeouts(g_mock_ticks);
        assert(s_raw_mod_calls==1 && s_recorded_mods[0].mod_mask==MOD_RCTRL);
        hold_tap_process_event(0,0,legacy[i],false);assert(s_raw_mod_calls==2);
    }
}
static void test_explicit_modifier_taps(void) {
    mock_reset_records();hold_tap_init();
    uint32_t timed=HT_T(MK(MOD_RCTRL,K_NULL),K_APP,80);
    hold_tap_process_event(0,0,timed,true);
    g_mock_ticks=79;hold_tap_check_timeouts(g_mock_ticks);assert(!s_raw_mod_calls);
    g_mock_ticks=80;hold_tap_check_timeouts(g_mock_ticks);assert(s_raw_mod_calls==1);
    hold_tap_process_event(0,0,timed,false);assert(s_raw_mod_calls==2);
    const uint8_t masks[] = {1,2,4,8,16,32,64,128,0xe0,0xe7,0xf0,0xff};
    for (unsigned i=0;i<sizeof(masks);++i) {
        mock_reset_records();hold_tap_init();layers_init();
        uint8_t tap=masks[i]==MOD_RSHIFT?K_RGUI:K_APP;
        uint32_t key=HT(MK(masks[i],K_NULL),tap);
        assert(hold_tap_process_event(0,0,key,true));
        g_mock_ticks=1000;hold_tap_check_timeouts(g_mock_ticks);
        assert(s_raw_mod_calls==1 && s_recorded_mods_count==1 && s_recorded_mods[0].mod_mask==masks[i] && s_recorded_mods[0].pressed);
        assert(layers_get_state()==1);
        hold_tap_process_event(0,0,key,false);
        assert(s_raw_mod_calls==2 && s_recorded_mods_count==2 && s_recorded_mods[1].mod_mask==masks[i] && !s_recorded_mods[1].pressed);
        mock_reset_records();hold_tap_init();
        hold_tap_process_event(0,0,key,true);g_mock_ticks=20;hold_tap_process_event(0,0,key,false);
        assert(s_recorded_mods_count==0 && s_recorded_keys_count==2);
        assert(s_recorded_keys[0].keycode==tap && s_recorded_keys[0].pressed && !s_recorded_keys[1].pressed);
    }
}

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
    assert(s_recorded_keys_count == 1);
    assert(s_recorded_keys[0].keycode == K_LCTL && s_recorded_keys[0].pressed == true);
    assert(s_recorded_mods_count == 0);

    // Release key at t=400
    g_mock_ticks = 400;
    hold_tap_process_event(0, 0, ht_key, false);

    // Verify modifier released
    assert(s_recorded_keys_count == 2);
    assert(s_recorded_keys[1].keycode == K_LCTL && s_recorded_keys[1].pressed == false);
    assert(s_recorded_mods_count == 0);

    printf("  ✔ test_hold_tap_hold_after_timeout passed\n");
}

static void test_hold_tap_layer_tap(void) {
    mock_reset_records();
    hold_tap_init();
    layers_init();

    // Layer-Tap: Hold = Layer 1 (< 16), Tap = K_B
    uint32_t lt_key = HT(MO(1), K_B);

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
    assert(s_recorded_keys_count == 1);
    assert(s_recorded_keys[0].keycode == K_LSFT && s_recorded_keys[0].pressed == true);

    // Release at t=200
    g_mock_ticks = 200;
    hold_tap_process_event(0, 1, ht_key, false);
    assert(s_recorded_keys_count == 2);
    assert(s_recorded_keys[1].keycode == K_LSFT && s_recorded_keys[1].pressed == false);

    printf("  ✔ test_hold_tap_custom_timeout passed\n");
}

static void test_hold_tap_quick_tap_repeat(void) {
    mock_reset_records();
    hold_tap_init();

    // Mod-Tap: Hold = K_LCTL, Tap = K_A
    uint32_t ht_key = HT(K_LCTL, K_A);

    // 1. Initial quick tap: press at t=100, release at t=150 (50ms duration)
    g_mock_ticks = 100;
    hold_tap_process_event(0, 0, ht_key, true);
    g_mock_ticks = 150;
    hold_tap_process_event(0, 0, ht_key, false);

    // Verify initial tap was sent (K_A press + release)
    assert(s_recorded_keys_count == 2);
    assert(s_recorded_keys[0].keycode == K_A && s_recorded_keys[0].pressed == true);
    assert(s_recorded_keys[1].keycode == K_A && s_recorded_keys[1].pressed == false);
    assert(s_recorded_mods_count == 0);

    // 2. Quick double tap: press again at t=230 (80ms after release <= QUICK_TAP_TERM 150ms)
    g_mock_ticks = 230;
    hold_tap_process_event(0, 0, ht_key, true);

    // Should immediately send K_A pressed (repeat mode, continuous hold)
    assert(s_recorded_keys_count == 3);
    assert(s_recorded_keys[2].keycode == K_A && s_recorded_keys[2].pressed == true);
    assert(s_recorded_mods_count == 0);

    // Advance time past tapping term to t=500: should NOT become a modifier!
    g_mock_ticks = 500;
    hold_tap_check_timeouts(g_mock_ticks);
    assert(s_recorded_mods_count == 0);

    // Release key at t=600: should release K_A
    g_mock_ticks = 600;
    hold_tap_process_event(0, 0, ht_key, false);
    assert(s_recorded_keys_count == 4);
    assert(s_recorded_keys[3].keycode == K_A && s_recorded_keys[3].pressed == false);
    assert(s_recorded_mods_count == 0);

    printf("  ✔ test_hold_tap_quick_tap_repeat passed\n");
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

    // Two momentary owners; neither release can erase a persistent toggle.
    layers_on(2); layers_on(2); layers_off(2);
    assert(layers_get_active()==2);
    layers_off(2);assert(layers_get_active()==0);
    layers_toggle(2);layers_on(2);layers_off(2);
    assert(layers_get_active()==2);
    layers_toggle(2);assert(layers_get_active()==0);
    layers_on(2);layers_toggle(2);layers_toggle(2);
    assert(layers_get_active()==2);layers_off(2);assert(layers_get_active()==0);
    layers_on(1);layers_set_state(1u<<2);layers_off(1);
    assert(layers_get_state()==4);layers_set_state(1);

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

    // Toggle latches layer 1 independently of the momentary owner.
    layers_toggle(1);
    layers_off(1);
    assert(layers_get_active() == 1);
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

    // First owner releases; the second still holds A.
    usb_process_key(K_A, false);
    assert(s_send_report_count == 2 && s_last_sent_report.Keycodes[0] == K_A);
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

    // Releasing a composite key must preserve held physical modifiers
    usb_process_modifiers(MOD_LSHIFT, true);
    assert(s_last_sent_report.Modifier == MOD_LSHIFT);
    usb_process_key_with_modifiers(MOD_LSHIFT | MOD_LCTRL, K_V, true);
    assert(s_last_sent_report.Modifier == (MOD_LSHIFT | MOD_LCTRL));
    usb_process_key_with_modifiers(MOD_LSHIFT | MOD_LCTRL, K_V, false);
    assert(s_last_sent_report.Modifier == MOD_LSHIFT);
    usb_process_modifiers(MOD_LSHIFT, false);
    assert(s_last_sent_report.Modifier == 0);

    printf("  ✔ test_usb_hid_reports_and_zero_queue passed\n");
}

#ifdef MATRIX_REPORT_BATCH
static void test_usb_matrix_batch(void) {
    usb_init();
    s_send_report_count = 0;
    usb_process_batched_key(K_A, true);
    usb_process_batched_key(K_B, true);
    assert(s_send_report_count == 0);
    usb_flush_batched_keys();
    assert(s_send_report_count == 1);
    assert(s_last_sent_report.Keycodes[0] == K_A && s_last_sent_report.Keycodes[1] == K_B);

    // Release and re-press of the same HID usage must survive batching.
    usb_process_batched_key(K_A, false);
    usb_process_batched_key(K_A, true);
    assert(s_send_report_count == 2 && s_last_sent_report.Keycodes[0] == 0);
    usb_flush_batched_keys();
    assert(s_send_report_count == 3 && s_last_sent_report.Keycodes[0] == K_A);

    // An immediate modifier must follow any pending ordinary key changes.
    usb_process_batched_key(K_B, false);
    usb_process_modifiers(MOD_LSHIFT, true);
    assert(s_send_report_count == 5);
    assert(s_last_sent_report.Modifier == MOD_LSHIFT && s_last_sent_report.Keycodes[1] == 0);
}
#endif

static void test_shared_key_owners(void) {
    usb_init();
    usb_process_key(K_A, true);
    usb_process_key_with_modifiers(MOD_RSHIFT, K_A, true);
    usb_process_key_with_modifiers(MOD_RSHIFT, K_A, false);
    assert(s_last_sent_report.Modifier == 0 && s_last_sent_report.Keycodes[0] == K_A);
    usb_process_key(K_A, false);
    assert(s_last_sent_report.Keycodes[0] == 0);
    usb_process_key(K_A, false); // Unmatched release is harmless.
    usb_process_key(K_B, true);
    assert(s_last_sent_report.Keycodes[0] == K_B);
    usb_init(); // Reset must clear all ownership.
    usb_process_key(K_B, true);usb_process_key(K_B, false);
    assert(s_last_sent_report.Keycodes[0] == 0);
}

static void test_6kro_slot_ownership(void) {
    usb_init();
    for (unsigned k=K_A;k<K_A+6;k++) usb_process_key(k,true);
    usb_process_key(K_A,true); // Second owner, even when all slots are occupied.
    usb_process_key(K_A,false);
    assert(s_last_sent_report.Keycodes[0]==K_A);
    usb_process_key(K_A,false);
    assert(s_last_sent_report.Keycodes[0]==0);
    usb_process_key(K_Z,true); // Reuse must start with one owner.
    assert(s_last_sent_report.Keycodes[0]==K_Z);
    usb_process_key(K_Z,false);assert(s_last_sent_report.Keycodes[0]==0);
    usb_process_key(K_Z,false);assert(s_last_sent_report.Keycodes[0]==0);
    for(unsigned k=K_B;k<K_A+6;k++)usb_process_key(k,false);
    for(unsigned i=0;i<6;i++)assert(s_last_sent_report.Keycodes[i]==0);
}

int main(void) {
    test_explicit_modifier_taps();
    test_unified_hold_tap_actions();
    printf("\n=== Running DMK Host-Based C Core Unit Tests ===\n\n");

    printf("[1/3] Testing Hold-Tap subsystem...\n");
    test_hold_tap_tap_before_timeout();
    test_hold_tap_hold_after_timeout();
    test_hold_tap_layer_tap();
    test_hold_tap_custom_timeout();
    test_hold_tap_quick_tap_repeat();

    printf("\n[2/3] Testing Layers subsystem...\n");
    test_layers_stack_and_fallthrough();

    printf("\n[3/3] Testing USB HID Zero-Queue subsystem...\n");
    test_usb_hid_reports_and_zero_queue();
    test_shared_key_owners();
    test_6kro_slot_ownership();
#ifdef MATRIX_REPORT_BATCH
    test_usb_matrix_batch();
#endif

    printf("\n🎉 All C core unit tests passed successfully!\n\n");
    return 0;
}

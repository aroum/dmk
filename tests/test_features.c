#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "keys.h"
#include "oneshot.h"
#include "macros.h"

TickType_t g_mock_ticks;
static unsigned layer_state, key_down, key_up, mod_down, mod_up;
void layers_on(uint8_t layer) { layer_state |= 1u << layer; }
void layers_off(uint8_t layer) { layer_state &= ~(1u << layer); }
void keyboard_send_key(uint16_t key, bool pressed) {
    assert(key == K_A);
    if (pressed) ++key_down; else ++key_up;
}
void keyboard_send_modifiers(uint8_t mask, bool pressed) {
    assert(mask == 1);
    if (pressed) ++mod_down; else ++mod_up;
}
static const MacroStep steps[] = {{KEY_DOWN, K_A}, {DELAY, 25}, {KEY_UP, K_A}};
const Macro keyboard_macros[] = {{steps, 3, MACRO_NO_LAYER}};
const uint8_t keyboard_macros_count = 1;

int main(void) {
    oneshot_init();
    assert(oneshot_process_event(OS_LAYER(2), true));
    assert(layer_state == 4);
    g_mock_ticks = 10;
    assert(oneshot_process_event(OS_LAYER(2), false));
    assert(oneshot_has_active());
    oneshot_on_key_press(K_A);
    oneshot_on_key_release();
    assert(layer_state == 0);
    assert(!oneshot_has_active());
    assert(oneshot_process_event(OS(K_LCTL), true));
    g_mock_ticks = 20;
    assert(oneshot_process_event(OS(K_LCTL), false));
    oneshot_on_key_press(K_A);
    assert(mod_down == 1 && mod_up == 1);
    assert(macros_process_key(0xC0, true));
    assert(macros_check_timeouts(100) == 25);
    assert(key_down == 1 && key_up == 0);
    assert(macros_check_timeouts(124) == 1);
    assert(macros_check_timeouts(125) == portMAX_DELAY);
    assert(key_up == 1 && !macros_has_active());
    puts("oneshot and macro tests passed");
    return 0;
}

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "keys.h"

static uint16_t state = 1;
static uint8_t system_id, modes[2];
static bool gpio[10], suspended;
uint16_t layers_get_state(void) { return state; }
void layers_set_state(uint16_t s) { state = s; }
uint8_t hhkb_get_system(void) { return system_id; }
void hhkb_set_system(uint8_t s) { system_id = s; }
uint8_t hhkb_get_led_mode(uint8_t i) { return modes[i]; }
void hal_gpio_put(int p, bool v) { gpio[p] = v; }
bool tud_suspended(void) { return suspended; }
bool tud_mounted(void) { return true; }
#include "../../keyboards/hhkb_classic/modules/indicators_system/indicators_system.c"

int main(void) {
    keyboard_platform_init();
    assert(state == 1);
    assert(!hhkb_process_key(MO(1), true));
    state = 3; // Layer 1 is held by the core via MO(1) or LT(1, KC_DEL).
    assert(!hhkb_process_key(K_SPC, true));
    assert(!hhkb_process_key(K_SPC, false));
    assert(system_id == 0 && state == 3);
    modes[0] = 3;
    keyboard_platform_tick(500);
    assert(gpio[8]);

    assert(hhkb_process_key(HHKB_SYSTEM_2, true));
    assert(system_id == 1 && state == 7);
    assert(hhkb_process_key(HHKB_SYSTEM_2, false));
    assert(system_id == 1 && state == 7);
    assert(hhkb_process_key(HHKB_SYSTEM_1, false));
    assert(system_id == 1 && state == 7);
    assert(hhkb_process_key(HHKB_SYSTEM_1, true));
    assert(system_id == 0 && state == 3);

    state = 1;
    keyboard_platform_tick(500);
    assert(!gpio[8]);
    assert(hhkb_process_key(HHKB_SYSTEM_2, true));
    assert(system_id == 1 && state == 5);
    assert(!hhkb_process_key(MO(3), true));
    modes[0] = 2;
    modes[1] = 4;
    hhkb_hid_led_change(2);
    keyboard_platform_tick(500);
    assert(gpio[8] && gpio[9]);
    suspended = true;
    keyboard_platform_tick(500);
    assert(!gpio[8] && !gpio[9]);
    return 0;
}

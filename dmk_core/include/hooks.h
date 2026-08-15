#ifndef HOOKS_H
#define HOOKS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Weak hooks for user modules and extensions
void hook_early_init(void);
void hook_layer_change(uint8_t active_layer);
void hook_matrix_change(uint8_t row, uint8_t col, bool pressed);
void hook_key_sent(uint16_t keycode, bool pressed);
void hook_hid_led_change(uint8_t led_mask);

#ifdef __cplusplus
}
#endif

#endif // HOOKS_H

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
bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed);
bool hook_process_key(uint32_t keycode, bool pressed);
void hook_key_sent(uint16_t keycode, bool pressed);
void hook_hid_led_change(uint8_t led_mask);

// Mouse and trackball motion/scroll interception hooks
bool hook_mouse_move(int8_t *dx, int8_t *dy);
bool hook_mouse_scroll(int8_t *wheel, int8_t *pan);
void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan);

// Gamepad report interception and modification hook
bool hook_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat, uint32_t *buttons);

// MIDI output hook for external modules (DIN-5 Jack, BLE MIDI, CV/Gate)
void hook_midi_send(const uint8_t *msg, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif // HOOKS_H

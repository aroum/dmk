#ifndef HOOKS_H
#define HOOKS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Diagnostic counter for failed FreeRTOS dynamic allocations since boot.
extern volatile uint32_t dmk_malloc_failure_count;

/**
 * @brief Early hardware hook called before RTOS tasks and peripherals start.
 */
void hook_early_init(void);

/**
 * @brief Hook called on active layer transition.
 * @param active_layer Highest active layer index
 */
void hook_layer_change(uint8_t active_layer);

/**
 * @brief Hook called on matrix coordinate change before debouncing/processing.
 * @param row Matrix row
 * @param col Matrix col
 * @param pressed True on press, false on release
 * @return true to suppress default matrix event processing, false to proceed
 */
bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed);

/**
 * @brief Hook called before core keycode resolution.
 * @param keycode Decoded 32-bit keycode
 * @param pressed True on press, false on release
 * @return true to intercept and suppress keycode, false to allow default handling
 */
bool hook_process_key(uint32_t keycode, bool pressed);

/**
 * @brief Hook called after a basic keycode has been queued to USB HID report.
 * @param keycode 16-bit USB HID keycode
 * @param pressed True on press, false on release
 */
void hook_key_sent(uint16_t keycode, bool pressed);

/**
 * @brief Hook called when host updates keyboard LED indicator states.
 * @param led_mask Bitmask of LED states (Caps Lock, Num Lock, Scroll Lock)
 */
void hook_hid_led_change(uint8_t led_mask);

// Mouse and trackball motion/scroll interception hooks

/**
 * @brief Intercept and modify relative mouse cursor deltas.
 * @param dx Pointer to relative X movement
 * @param dy Pointer to relative Y movement
 * @return true to send the (possibly modified) mouse movement report, false to suppress it
 */
bool hook_mouse_move(int8_t *dx, int8_t *dy);

/**
 * @brief Intercept and modify mouse scroll wheels.
 * @param wheel Pointer to vertical scroll delta
 * @param pan Pointer to horizontal pan delta
 * @return true to send the (possibly modified) scroll report, false to suppress it
 */
bool hook_mouse_scroll(int8_t *wheel, int8_t *pan);

/**
 * @brief Hook called whenever a complete mouse HID report is generated.
 * @param buttons Active mouse button bitmask
 * @param dx Relative X movement
 * @param dy Relative Y movement
 * @param wheel Vertical wheel delta
 * @param pan Horizontal pan delta
 */
void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan);

// Gamepad report interception and modification hook

/**
 * @brief Intercept and modify analog axes, hat switch, and buttons for gamepad HID report.
 * @param x Pointer to X-axis value (-127..127)
 * @param y Pointer to Y-axis value (-127..127)
 * @param z Pointer to Z-axis value (-127..127)
 * @param rz Pointer to Rz-axis value (-127..127)
 * @param rx Pointer to Rx-axis value (-127..127)
 * @param ry Pointer to Ry-axis value (-127..127)
 * @param hat Pointer to hat direction (0..8)
 * @param buttons Pointer to 32-bit button bitmask
 * @return true to send the (possibly modified) gamepad report, false to suppress it
 */
bool hook_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat,
                         uint32_t *buttons);

// MIDI hooks for external modules

/**
 * @brief Hook called when DMK core transmits raw MIDI bytes.
 * @param msg Pointer to raw MIDI message bytes
 * @param len Byte length of message
 */
void hook_midi_send(const uint8_t *msg, uint8_t len);

/**
 * @brief Hook called when a 4-byte USB-MIDI packet is received from host.
 * @param packet 4-byte USB-MIDI packet buffer
 */
void hook_midi_receive(const uint8_t packet[4]);

/* Modules may register independent subscribers without defining a global weak hook.
 * Legacy hook_* implementations continue to run first. For consuming hooks,
 * dispatch stops at the first subscriber that consumes the event. */
typedef struct {
    void (*early_init)(void);
    void (*layer_change)(uint8_t layer);
    bool (*matrix_change)(uint8_t row, uint8_t col, bool pressed);
    bool (*process_key)(uint32_t key, bool pressed);
    void (*key_sent)(uint16_t key, bool pressed);
    void (*hid_led_change)(uint8_t leds);
    bool (*mouse_move)(int8_t *x, int8_t *y);
    bool (*mouse_scroll)(int8_t *wheel, int8_t *pan);
    void (*mouse_report)(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan);
    bool (*gamepad_report)(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat, uint32_t *buttons);
    void (*midi_send)(const uint8_t *msg, uint8_t len);
    void (*midi_receive)(const uint8_t *packet);
} dmk_hook_subscriber_t;
#define DMK_HOOK_SUBSCRIBER(name, ...) \
    static const dmk_hook_subscriber_t name __attribute__((used, section("dmk_hooks"), aligned(sizeof(void *)))) = { __VA_ARGS__ }

void hooks_dispatch_early_init(void);
void hooks_dispatch_layer_change(uint8_t layer);
bool hooks_dispatch_matrix_change(uint8_t row, uint8_t col, bool pressed);
bool hooks_dispatch_process_key(uint32_t key, bool pressed);
void hooks_dispatch_key_sent(uint16_t key, bool pressed);
void hooks_dispatch_hid_led_change(uint8_t leds);
bool hooks_dispatch_mouse_move(int8_t *x, int8_t *y);
bool hooks_dispatch_mouse_scroll(int8_t *wheel, int8_t *pan);
void hooks_dispatch_mouse_report(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan);
bool hooks_dispatch_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat, uint32_t *buttons);
void hooks_dispatch_midi_send(const uint8_t *msg, uint8_t len);
void hooks_dispatch_midi_receive(const uint8_t *packet);

#ifdef __cplusplus
}
#endif

#endif // HOOKS_H

#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// D-Pad / Hat switch directions (1-based, 0 = released/centered)
#define GAMEPAD_DPAD_CENTER 0
#define GAMEPAD_DPAD_UP 1
#define GAMEPAD_DPAD_UP_RIGHT 2
#define GAMEPAD_DPAD_RIGHT 3
#define GAMEPAD_DPAD_DOWN_RIGHT 4
#define GAMEPAD_DPAD_DOWN 5
#define GAMEPAD_DPAD_DOWN_LEFT 6
#define GAMEPAD_DPAD_LEFT 7
#define GAMEPAD_DPAD_UP_LEFT 8

// Button bitmasks (1-32)
#define GAMEPAD_BUTTON_1 (1UL << 0)
#define GAMEPAD_BUTTON_2 (1UL << 1)
#define GAMEPAD_BUTTON_3 (1UL << 2)
#define GAMEPAD_BUTTON_4 (1UL << 3)
#define GAMEPAD_BUTTON_5 (1UL << 4)
#define GAMEPAD_BUTTON_6 (1UL << 5)
#define GAMEPAD_BUTTON_7 (1UL << 6)
#define GAMEPAD_BUTTON_8 (1UL << 7)
#define GAMEPAD_BUTTON_9 (1UL << 8)
#define GAMEPAD_BUTTON_10 (1UL << 9)
#define GAMEPAD_BUTTON_11 (1UL << 10)
#define GAMEPAD_BUTTON_12 (1UL << 11)
#define GAMEPAD_BUTTON_13 (1UL << 12)
#define GAMEPAD_BUTTON_14 (1UL << 13)
#define GAMEPAD_BUTTON_15 (1UL << 14)
#define GAMEPAD_BUTTON_16 (1UL << 15)
#define GAMEPAD_BUTTON_17 (1UL << 16)
#define GAMEPAD_BUTTON_18 (1UL << 17)
#define GAMEPAD_BUTTON_19 (1UL << 18)
#define GAMEPAD_BUTTON_20 (1UL << 19)
#define GAMEPAD_BUTTON_21 (1UL << 20)
#define GAMEPAD_BUTTON_22 (1UL << 21)
#define GAMEPAD_BUTTON_23 (1UL << 22)
#define GAMEPAD_BUTTON_24 (1UL << 23)
#define GAMEPAD_BUTTON_25 (1UL << 24)
#define GAMEPAD_BUTTON_26 (1UL << 25)
#define GAMEPAD_BUTTON_27 (1UL << 26)
#define GAMEPAD_BUTTON_28 (1UL << 27)
#define GAMEPAD_BUTTON_29 (1UL << 28)
#define GAMEPAD_BUTTON_30 (1UL << 29)
#define GAMEPAD_BUTTON_31 (1UL << 30)
#define GAMEPAD_BUTTON_32 (1UL << 31)

// Standard controller alias mapping
#define GAMEPAD_BUTTON_A GAMEPAD_BUTTON_1
#define GAMEPAD_BUTTON_B GAMEPAD_BUTTON_2
#define GAMEPAD_BUTTON_X GAMEPAD_BUTTON_3
#define GAMEPAD_BUTTON_Y GAMEPAD_BUTTON_4
#define GAMEPAD_BUTTON_LB GAMEPAD_BUTTON_5
#define GAMEPAD_BUTTON_RB GAMEPAD_BUTTON_6
#define GAMEPAD_BUTTON_SELECT GAMEPAD_BUTTON_7
#define GAMEPAD_BUTTON_START GAMEPAD_BUTTON_8
#define GAMEPAD_BUTTON_HOME GAMEPAD_BUTTON_9
#define GAMEPAD_BUTTON_L3 GAMEPAD_BUTTON_10
#define GAMEPAD_BUTTON_R3 GAMEPAD_BUTTON_11

/**
 * @brief Current state of the gamepad subsystem
 */
typedef struct {
    int8_t x;         // Left Stick X (-127..127)
    int8_t y;         // Left Stick Y (-127..127)
    int8_t z;         // Right Stick X (-127..127)
    int8_t rz;        // Right Stick Y (-127..127)
    int8_t rx;        // Analog Left Trigger (-127..127)
    int8_t ry;        // Analog Right Trigger (-127..127)
    uint8_t dpad;     // Hat Switch (0 = center, 1..8 = direction)
    uint32_t buttons; // Bitmask of buttons 1..32
} gamepad_state_t;

/**
 * @brief Initialize gamepad state and subsystem.
 */
void gamepad_init(void);

/**
 * @brief Set position of the left analog stick.
 * @param x Horizontal axis (-127..127)
 * @param y Vertical axis (-127..127)
 */
void gamepad_set_axis_left(int8_t x, int8_t y);

/**
 * @brief Set position of the right analog stick.
 * @param z Horizontal axis (-127..127)
 * @param rz Vertical axis (-127..127)
 */
void gamepad_set_axis_right(int8_t z, int8_t rz);

/**
 * @brief Set analog trigger values.
 * @param left Left trigger (-127..127 or 0..127)
 * @param right Right trigger (-127..127 or 0..127)
 */
void gamepad_set_triggers(int8_t left, int8_t right);

/**
 * @brief Set 8-way D-Pad (Hat switch) direction.
 * @param dpad Direction (GAMEPAD_DPAD_CENTER, UP, DOWN, etc.)
 */
void gamepad_set_dpad(uint8_t dpad);

/**
 * @brief Set state of a single button (1..32).
 * @param button Button index (1 to 32)
 * @param pressed true if pressed, false if released
 */
void gamepad_set_button(uint8_t button, bool pressed);

/**
 * @brief Press a single button (1..32).
 */
void gamepad_press_button(uint8_t button);

/**
 * @brief Release a single button (1..32).
 */
void gamepad_release_button(uint8_t button);

/**
 * @brief Send current gamepad state to host as an HID input report.
 */
void gamepad_send(void);

/**
 * @brief Process keycode in the gamepad range.
 * @param key Keycode
 * @param pressed true if pressed, false if released
 * @return true if key was handled by gamepad subsystem, false otherwise
 */
bool gamepad_process_key(uint16_t key, bool pressed);

/**
 * @brief Get pointer to read-only current state.
 */
const gamepad_state_t *gamepad_get_state(void);

#ifdef __cplusplus
}
#endif

#endif // GAMEPAD_H

#include "gamepad.h"
#include "app_usb_hid.h"
#include "hooks.h"
#include "keys.h"
#include <string.h>

static gamepad_state_t s_gamepad;

// Tracking for D-Pad cardinal keys to allow 8-way diagonal composition
static bool s_dpad_up = false;
static bool s_dpad_down = false;
static bool s_dpad_left = false;
static bool s_dpad_right = false;

// Tracking for digital stick keys
static bool s_lx_l = false, s_lx_r = false;
static bool s_ly_u = false, s_ly_d = false;
static bool s_rx_l = false, s_rx_r = false;
static bool s_ry_u = false, s_ry_d = false;
static bool s_lt = false, s_rt = false;

static uint8_t compute_dpad_state(void) {
    if (s_dpad_up && s_dpad_right) return GAMEPAD_DPAD_UP_RIGHT;
    if (s_dpad_down && s_dpad_right) return GAMEPAD_DPAD_DOWN_RIGHT;
    if (s_dpad_down && s_dpad_left) return GAMEPAD_DPAD_DOWN_LEFT;
    if (s_dpad_up && s_dpad_left) return GAMEPAD_DPAD_UP_LEFT;

    if (s_dpad_up) return GAMEPAD_DPAD_UP;
    if (s_dpad_right) return GAMEPAD_DPAD_RIGHT;
    if (s_dpad_down) return GAMEPAD_DPAD_DOWN;
    if (s_dpad_left) return GAMEPAD_DPAD_LEFT;

    return GAMEPAD_DPAD_CENTER;
}

void gamepad_init(void) {
    memset(&s_gamepad, 0, sizeof(s_gamepad));
    s_dpad_up = s_dpad_down = s_dpad_left = s_dpad_right = false;
    s_lx_l = s_lx_r = s_ly_u = s_ly_d = false;
    s_rx_l = s_rx_r = s_ry_u = s_ry_d = false;
    s_lt = s_rt = false;
}

const gamepad_state_t *gamepad_get_state(void) {
    return &s_gamepad;
}

void gamepad_set_axis_left(int8_t x, int8_t y) {
    s_gamepad.x = x;
    s_gamepad.y = y;
}

void gamepad_set_axis_right(int8_t z, int8_t rz) {
    s_gamepad.z = z;
    s_gamepad.rz = rz;
}

void gamepad_set_triggers(int8_t left, int8_t right) {
    s_gamepad.rx = left;
    s_gamepad.ry = right;
}

void gamepad_set_dpad(uint8_t dpad) {
    s_gamepad.dpad = dpad;
}

void gamepad_set_button(uint8_t button, bool pressed) {
    if (button < 1 || button > 32) return;
    uint32_t mask = (1UL << (button - 1));
    if (pressed) {
        s_gamepad.buttons |= mask;
    } else {
        s_gamepad.buttons &= ~mask;
    }
}

void gamepad_press_button(uint8_t button) {
    gamepad_set_button(button, true);
    gamepad_send();
}

void gamepad_release_button(uint8_t button) {
    gamepad_set_button(button, false);
    gamepad_send();
}

void gamepad_send(void) {
    int8_t x = s_gamepad.x;
    int8_t y = s_gamepad.y;
    int8_t z = s_gamepad.z;
    int8_t rz = s_gamepad.rz;
    int8_t rx = s_gamepad.rx;
    int8_t ry = s_gamepad.ry;
    uint8_t dpad = s_gamepad.dpad;
    uint32_t buttons = s_gamepad.buttons;

    // Allow user modules to intercept or scale axes
    if (!hook_gamepad_report(&x, &y, &z, &rz, &rx, &ry, &dpad, &buttons)) {
        return;
    }

    USB_HID_SendGamepadReport(x, y, z, rz, rx, ry, dpad, buttons);
}

bool gamepad_process_key(uint16_t key, bool pressed) {
    // 1. Buttons 1 to 32
    if (key >= GP_BTN1 && key <= GP_BTN32) {
        uint8_t btn = (uint8_t)(key - GP_BTN1 + 1);
        gamepad_set_button(btn, pressed);
        gamepad_send();
        return true;
    }

    // 2. D-Pad Direction Keys
    switch (key) {
        case GP_DPAD_UP:
            s_dpad_up = pressed;
            s_gamepad.dpad = compute_dpad_state();
            gamepad_send();
            return true;
        case GP_DPAD_DOWN:
            s_dpad_down = pressed;
            s_gamepad.dpad = compute_dpad_state();
            gamepad_send();
            return true;
        case GP_DPAD_LEFT:
            s_dpad_left = pressed;
            s_gamepad.dpad = compute_dpad_state();
            gamepad_send();
            return true;
        case GP_DPAD_RIGHT:
            s_dpad_right = pressed;
            s_gamepad.dpad = compute_dpad_state();
            gamepad_send();
            return true;
        default:
            break;
    }

    // 3. Digital to Left Analog Stick
    switch (key) {
        case GP_LX_L:
            s_lx_l = pressed;
            s_gamepad.x = (s_lx_l && !s_lx_r) ? -127 : (s_lx_r ? 127 : 0);
            gamepad_send();
            return true;
        case GP_LX_R:
            s_lx_r = pressed;
            s_gamepad.x = (s_lx_r && !s_lx_l) ? 127 : (s_lx_l ? -127 : 0);
            gamepad_send();
            return true;
        case GP_LY_U:
            s_ly_u = pressed;
            s_gamepad.y = (s_ly_u && !s_ly_d) ? -127 : (s_ly_d ? 127 : 0);
            gamepad_send();
            return true;
        case GP_LY_D:
            s_ly_d = pressed;
            s_gamepad.y = (s_ly_d && !s_ly_u) ? 127 : (s_ly_u ? -127 : 0);
            gamepad_send();
            return true;
        default:
            break;
    }

    // 4. Digital to Right Analog Stick
    switch (key) {
        case GP_RX_L:
            s_rx_l = pressed;
            s_gamepad.z = (s_rx_l && !s_rx_r) ? -127 : (s_rx_r ? 127 : 0);
            gamepad_send();
            return true;
        case GP_RX_R:
            s_rx_r = pressed;
            s_gamepad.z = (s_rx_r && !s_rx_l) ? 127 : (s_rx_l ? -127 : 0);
            gamepad_send();
            return true;
        case GP_RY_U:
            s_ry_u = pressed;
            s_gamepad.rz = (s_ry_u && !s_ry_d) ? -127 : (s_ry_d ? 127 : 0);
            gamepad_send();
            return true;
        case GP_RY_D:
            s_ry_d = pressed;
            s_gamepad.rz = (s_ry_d && !s_ry_u) ? 127 : (s_ry_u ? -127 : 0);
            gamepad_send();
            return true;
        default:
            break;
    }

    // 5. Analog Triggers
    if (key == GP_LT) {
        s_lt = pressed;
        s_gamepad.rx = s_lt ? 127 : 0;
        gamepad_send();
        return true;
    }
    if (key == GP_RT) {
        s_rt = pressed;
        s_gamepad.ry = s_rt ? 127 : 0;
        gamepad_send();
        return true;
    }

    return false;
}

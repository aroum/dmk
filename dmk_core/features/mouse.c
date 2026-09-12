#include "mouse.h"
#include "app_usb_hid.h"
#include "config.h"
#include "hooks.h"
#include "keys.h"
#include "task.h"
#include <string.h>

#ifndef MOUSEKEY_INTERVAL
#define MOUSEKEY_INTERVAL 16 // ms (~60 Hz update rate)
#endif

#ifndef MOUSEKEY_DELAY
#define MOUSEKEY_DELAY 150 // ms delay before continuous acceleration begins
#endif

#ifndef MOUSEKEY_BASE_SPEED
#define MOUSEKEY_BASE_SPEED 2 // pixels per tick at start
#endif

#ifndef MOUSEKEY_MAX_SPEED
#define MOUSEKEY_MAX_SPEED 10 // maximum pixels per tick
#endif

#ifndef MOUSEKEY_TIME_TO_MAX
#define MOUSEKEY_TIME_TO_MAX 600 // ms of continuous movement to reach max speed
#endif

#ifndef MOUSEKEY_SCROLL_DELAY
#define MOUSEKEY_SCROLL_DELAY 90 // ms between repeated scroll ticks
#endif

#define MOVE_UP (1 << 0)
#define MOVE_DOWN (1 << 1)
#define MOVE_LEFT (1 << 2)
#define MOVE_RIGHT (1 << 3)

#define SCROLL_UP (1 << 0)
#define SCROLL_DOWN (1 << 1)
#define SCROLL_LEFT (1 << 2)
#define SCROLL_RIGHT (1 << 3)

#define ACCEL_MODE_SLOW 1
#define ACCEL_MODE_NORMAL 0
#define ACCEL_MODE_FAST 2

typedef struct {
    uint8_t buttons;
    uint8_t move_keys;
    uint8_t scroll_keys;
    uint8_t accel_mode;

    TickType_t move_start_tick;
    TickType_t last_move_tick;
    TickType_t last_scroll_tick;
} mouse_state_t;

static mouse_state_t s_mouse;

void mouse_init(void) {
    memset(&s_mouse, 0, sizeof(s_mouse));
}

uint8_t mouse_get_buttons(void) {
    return s_mouse.buttons;
}

void mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan) {
    USB_HID_SendMouseReport(buttons, dx, dy, wheel, pan);
    hook_mouse_report(buttons, dx, dy, wheel, pan);
}

void mouse_move(int8_t dx, int8_t dy) {
    if (!hook_mouse_move(&dx, &dy)) {
        return; // Hook intercepted or cancelled movement
    }
    if (dx != 0 || dy != 0) {
        mouse_report(s_mouse.buttons, dx, dy, 0, 0);
    }
}

void mouse_scroll(int8_t wheel, int8_t pan) {
    if (!hook_mouse_scroll(&wheel, &pan)) {
        return; // Hook intercepted or cancelled scroll
    }
    if (wheel != 0 || pan != 0) {
        mouse_report(s_mouse.buttons, 0, 0, wheel, pan);
    }
}

void mouse_button_set(uint8_t button_mask, bool pressed) {
    uint8_t prev = s_mouse.buttons;
    if (pressed) {
        s_mouse.buttons |= button_mask;
    } else {
        s_mouse.buttons &= ~button_mask;
    }
    if (s_mouse.buttons != prev) {
        // Send button update immediately for lowest latency
        mouse_report(s_mouse.buttons, 0, 0, 0, 0);
    }
}

void mouse_button_press(uint8_t button_mask) {
    mouse_button_set(button_mask, true);
}

void mouse_button_release(uint8_t button_mask) {
    mouse_button_set(button_mask, false);
}

static uint8_t resolve_button_mask(uint16_t keycode) {
    switch (keycode) {
    case HID_KEY_MOUSE_BTN1:
        return MOUSE_BTN_LEFT;
    case HID_KEY_MOUSE_BTN2:
        return MOUSE_BTN_RIGHT;
    case HID_KEY_MOUSE_BTN3:
        return MOUSE_BTN_MIDDLE;
    case HID_KEY_MOUSE_BTN4:
        return MOUSE_BTN_BACK;
    case HID_KEY_MOUSE_BTN5:
        return MOUSE_BTN_FORWARD;
    default:
        return 0;
    }
}

bool mouse_process_key(uint32_t key, bool pressed) {
    uint16_t kc = (uint16_t)(key & 0xFFFF);
    if (kc < HID_KEY_MOUSE_UP || kc > HID_KEY_MOUSE_ACCEL2) {
        return false;
    }

    TickType_t now = xTaskGetTickCount();

    // 1. Mouse Buttons
    uint8_t btn = resolve_button_mask(kc);
    if (btn != 0) {
        mouse_button_set(btn, pressed);
        return true;
    }

    // 2. Cursor Directions
    switch (kc) {
    case HID_KEY_MOUSE_UP:
        if (pressed)
            s_mouse.move_keys |= MOVE_UP;
        else
            s_mouse.move_keys &= ~MOVE_UP;
        break;
    case HID_KEY_MOUSE_DOWN:
        if (pressed)
            s_mouse.move_keys |= MOVE_DOWN;
        else
            s_mouse.move_keys &= ~MOVE_DOWN;
        break;
    case HID_KEY_MOUSE_LEFT:
        if (pressed)
            s_mouse.move_keys |= MOVE_LEFT;
        else
            s_mouse.move_keys &= ~MOVE_LEFT;
        break;
    case HID_KEY_MOUSE_RIGHT:
        if (pressed)
            s_mouse.move_keys |= MOVE_RIGHT;
        else
            s_mouse.move_keys &= ~MOVE_RIGHT;
        break;

    // 3. Scroll Wheel
    case HID_KEY_MOUSE_WH_UP:
        if (pressed) {
            s_mouse.scroll_keys |= SCROLL_UP;
            mouse_scroll(1, 0); // Immediate tick on keydown
            s_mouse.last_scroll_tick = now;
        } else {
            s_mouse.scroll_keys &= ~SCROLL_UP;
        }
        return true;
    case HID_KEY_MOUSE_WH_DOWN:
        if (pressed) {
            s_mouse.scroll_keys |= SCROLL_DOWN;
            mouse_scroll(-1, 0);
            s_mouse.last_scroll_tick = now;
        } else {
            s_mouse.scroll_keys &= ~SCROLL_DOWN;
        }
        return true;
    case HID_KEY_MOUSE_WH_LEFT:
        if (pressed) {
            s_mouse.scroll_keys |= SCROLL_LEFT;
            mouse_scroll(0, -1);
            s_mouse.last_scroll_tick = now;
        } else {
            s_mouse.scroll_keys &= ~SCROLL_LEFT;
        }
        return true;
    case HID_KEY_MOUSE_WH_RIGHT:
        if (pressed) {
            s_mouse.scroll_keys |= SCROLL_RIGHT;
            mouse_scroll(0, 1);
            s_mouse.last_scroll_tick = now;
        } else {
            s_mouse.scroll_keys &= ~SCROLL_RIGHT;
        }
        return true;

    // 4. Acceleration Modes
    case HID_KEY_MOUSE_ACCEL0:
        s_mouse.accel_mode = pressed ? ACCEL_MODE_SLOW : ACCEL_MODE_NORMAL;
        return true;
    case HID_KEY_MOUSE_ACCEL1:
        s_mouse.accel_mode = ACCEL_MODE_NORMAL;
        return true;
    case HID_KEY_MOUSE_ACCEL2:
        s_mouse.accel_mode = pressed ? ACCEL_MODE_FAST : ACCEL_MODE_NORMAL;
        return true;
    default:
        return false;
    }

    // Manage movement acceleration start time
    if (s_mouse.move_keys != 0) {
        if (s_mouse.move_start_tick == 0) {
            s_mouse.move_start_tick = now;
            s_mouse.last_move_tick = now;
            // Send initial single-step pulse for quick responsive tapping
            int8_t init_x = (s_mouse.move_keys & MOVE_RIGHT ? 1 : 0) - (s_mouse.move_keys & MOVE_LEFT ? 1 : 0);
            int8_t init_y = (s_mouse.move_keys & MOVE_DOWN ? 1 : 0) - (s_mouse.move_keys & MOVE_UP ? 1 : 0);
            mouse_move(init_x, init_y);
        }
    } else {
        s_mouse.move_start_tick = 0;
    }

    return true;
}

static int16_t calculate_speed(uint32_t elapsed_ms) {
    if (elapsed_ms < MOUSEKEY_DELAY) {
        return MOUSEKEY_BASE_SPEED;
    }

    uint32_t active_ms = elapsed_ms - MOUSEKEY_DELAY;
    if (active_ms > MOUSEKEY_TIME_TO_MAX) {
        active_ms = MOUSEKEY_TIME_TO_MAX;
    }

    int16_t speed = MOUSEKEY_BASE_SPEED +
                    (int16_t)(((MOUSEKEY_MAX_SPEED - MOUSEKEY_BASE_SPEED) * active_ms) / MOUSEKEY_TIME_TO_MAX);

    // Apply speed modifiers
    if (s_mouse.accel_mode == ACCEL_MODE_SLOW) {
        speed = (speed > 2) ? (speed / 3) : 1;
    } else if (s_mouse.accel_mode == ACCEL_MODE_FAST) {
        speed *= 2;
    }

    return speed;
}

TickType_t mouse_check_timeouts(TickType_t now) {
    if (s_mouse.move_keys == 0 && s_mouse.scroll_keys == 0) {
        return portMAX_DELAY;
    }

    const TickType_t interval_ticks = pdMS_TO_TICKS(MOUSEKEY_INTERVAL);
    const TickType_t scroll_interval = pdMS_TO_TICKS(MOUSEKEY_SCROLL_DELAY);

    // Handle cursor movement tick
    if (s_mouse.move_keys != 0 && (now - s_mouse.last_move_tick) >= interval_ticks) {
        uint32_t elapsed_ms = (uint32_t)((now - s_mouse.move_start_tick) * portTICK_PERIOD_MS);
        int16_t speed = calculate_speed(elapsed_ms);

        int8_t dx = 0;
        int8_t dy = 0;

        if (s_mouse.move_keys & MOVE_RIGHT)
            dx += (int8_t)speed;
        if (s_mouse.move_keys & MOVE_LEFT)
            dx -= (int8_t)speed;
        if (s_mouse.move_keys & MOVE_DOWN)
            dy += (int8_t)speed;
        if (s_mouse.move_keys & MOVE_UP)
            dy -= (int8_t)speed;

        mouse_move(dx, dy);
        s_mouse.last_move_tick = now;
    }

    // Handle repeated scroll wheel tick
    if (s_mouse.scroll_keys != 0 && (now - s_mouse.last_scroll_tick) >= scroll_interval) {
        int8_t wheel = 0;
        int8_t pan = 0;

        if (s_mouse.scroll_keys & SCROLL_UP)
            wheel += 1;
        if (s_mouse.scroll_keys & SCROLL_DOWN)
            wheel -= 1;
        if (s_mouse.scroll_keys & SCROLL_RIGHT)
            pan += 1;
        if (s_mouse.scroll_keys & SCROLL_LEFT)
            pan -= 1;

        mouse_scroll(wheel, pan);
        s_mouse.last_scroll_tick = now;
    }

    return interval_ticks;
}

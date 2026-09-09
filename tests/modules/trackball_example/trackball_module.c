/**
 * @file trackball_module.c
 * @brief Reference implementation for trackball/pointing device integration in DMK.
 *
 * Demonstrates:
 * 1. Feeding sensor motion data into the USB HID mouse subsystem via mouse_move().
 * 2. Intercepting motion with hook_mouse_move() to convert movement to scrolling
 *    (e.g., drag-scroll when holding a momentary layer or specific key).
 * 3. Handling custom mouse buttons and precision mode scaling.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "hooks.h"
#include "mouse.h"
#include "layers.h"
#include <stdbool.h>
#include <stdint.h>

// Configuration: which layer triggers drag-scroll mode (0-indexed)
#ifndef TRACKBALL_SCROLL_LAYER
#define TRACKBALL_SCROLL_LAYER 2
#endif

// Scroll divider to adjust trackball sensitivity when in drag-scroll mode
#ifndef TRACKBALL_SCROLL_DIVIDER
#define TRACKBALL_SCROLL_DIVIDER 8
#endif

static bool s_drag_scroll_enabled = false;
static int16_t s_scroll_accum_x = 0;
static int16_t s_scroll_accum_y = 0;

/**
 * @brief Intercept mouse/trackball movement before USB report is generated.
 *
 * If drag-scroll mode is active, motion is redirected to mouse_scroll()
 * and cursor movement is suppressed by returning false.
 *
 * @param dx Pointer to relative X movement delta (can be modified by hook).
 * @param dy Pointer to relative Y movement delta (can be modified by hook).
 * @return true to proceed with cursor movement, false to suppress it.
 */
bool hook_mouse_move(int8_t *dx, int8_t *dy) {
    if (!s_drag_scroll_enabled) {
        // Normal cursor movement: pass through unchanged
        return true;
    }

    // Accumulate movement deltas for smooth scrolling with divisor
    s_scroll_accum_x += *dx;
    s_scroll_accum_y += *dy;

    int8_t wheel = 0;
    int8_t pan = 0;

    if (s_scroll_accum_y >= TRACKBALL_SCROLL_DIVIDER) {
        wheel = -(s_scroll_accum_y / TRACKBALL_SCROLL_DIVIDER);
        s_scroll_accum_y %= TRACKBALL_SCROLL_DIVIDER;
    } else if (s_scroll_accum_y <= -TRACKBALL_SCROLL_DIVIDER) {
        wheel = -(s_scroll_accum_y / TRACKBALL_SCROLL_DIVIDER);
        s_scroll_accum_y %= TRACKBALL_SCROLL_DIVIDER;
    }

    if (s_scroll_accum_x >= TRACKBALL_SCROLL_DIVIDER) {
        pan = (s_scroll_accum_x / TRACKBALL_SCROLL_DIVIDER);
        s_scroll_accum_x %= TRACKBALL_SCROLL_DIVIDER;
    } else if (s_scroll_accum_x <= -TRACKBALL_SCROLL_DIVIDER) {
        pan = (s_scroll_accum_x / TRACKBALL_SCROLL_DIVIDER);
        s_scroll_accum_x %= TRACKBALL_SCROLL_DIVIDER;
    }

    if (wheel != 0 || pan != 0) {
        // Direct scroll report injection
        mouse_scroll(wheel, pan);
    }

    // Suppress cursor movement while drag-scrolling
    *dx = 0;
    *dy = 0;
    return false;
}

/**
 * @brief Listen for layer switches to automatically toggle drag-scroll mode.
 * @param active_layer Index of newly activated layer.
 */
void hook_layer_change(uint8_t active_layer) {
    if (active_layer == TRACKBALL_SCROLL_LAYER) {
        s_drag_scroll_enabled = true;
        s_scroll_accum_x = 0;
        s_scroll_accum_y = 0;
    } else {
        s_drag_scroll_enabled = false;
    }
}

/**
 * @brief Public API for user keycodes or external hardware drivers to toggle drag-scroll.
 * @param enable True to enable drag-scroll, false to disable.
 */
void trackball_set_drag_scroll(bool enable) {
    s_drag_scroll_enabled = enable;
    s_scroll_accum_x = 0;
    s_scroll_accum_y = 0;
}

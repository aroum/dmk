#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "queue.h"
#include "task.h"
#include "u8g2_custom.h"

#include <stdio.h>
#include <string.h>

// Framebuffer buffer (128x32 / 8 = 512 bytes)
static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];
static oled_display_state_t display_state = {
    .current_layer = 0,
    .caps_lock = false,
    .dirty = true
};

static const char *layer_names[] = {
    "0: BASE",
    "1: LOWER",
    "2: RAISE",
    "3: ADJUST"
};

/**
 * @brief Simple rasterizer / text renderer for demo U8g2 module
 */
static void oled_render_screen(void) {
    memset(oled_buffer, 0, sizeof(oled_buffer));

    // Get layer label
    const char *name = (display_state.current_layer < 4) 
        ? layer_names[display_state.current_layer] 
        : "N: CUSTOM";

    (void)name;
    // In full U8g2 hardware mode:
    // u8g2_FirstPage(&u8g2);
    // do {
    //     u8g2_DrawStr(&u8g2, 0, 10, "DMK Keyboard");
    //     u8g2_DrawStr(&u8g2, 0, 24, name);
    //     if (display_state.caps_lock) u8g2_DrawStr(&u8g2, 90, 24, "CAPS");
    // } while (u8g2_NextPage(&u8g2));
}

/**
 * @brief FreeRTOS task handling display refresh
 */
static void oled_task(void *pvParameters) {
    (void)pvParameters;

    while (1) {
        if (display_state.dirty) {
            oled_render_screen();
            display_state.dirty = false;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Early init hook called by dmk_core before scheduler starts
 */
void hook_early_init(void) {
    xTaskCreate(oled_task, "oled", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

/**
 * @brief Layer change hook called automatically on layer updates
 */
void hook_layer_change(uint8_t active_layer) {
    if (display_state.current_layer != active_layer) {
        display_state.current_layer = active_layer;
        display_state.dirty = true;
    }
}

/**
 * @brief HID LED hook called on CapsLock / NumLock toggle
 */
void hook_hid_led_change(uint8_t led_mask) {
    bool caps = (led_mask & 0x02) != 0;
    if (display_state.caps_lock != caps) {
        display_state.caps_lock = caps;
        display_state.dirty = true;
    }
}

#include "FreeRTOS.h"
#include "config.h"
#include "debug_indicator.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "rgb.h"
#include "task.h"

// External USB mount status synced by USB stack
extern volatile bool usb_mounted;

// Default color palettes if not provided in config.h
#ifndef DEBUG_KEY_RGB_COLOR
#define DEBUG_KEY_RGB_COLOR 0x00FF00 // Green on key press
#endif

#ifndef DEBUG_USB_RGB_COLOR
#define DEBUG_USB_RGB_COLOR 0x0000FF // Blue when USB connected
#endif

static volatile uint32_t active_keys_counter = 0;
static bool last_usb_state = false;

/**
 * @brief Periodic background task monitoring USB mount state for indicators
 */
static void debug_indicator_task(void *pvParameters) {
    (void)pvParameters;

    while (1) {
        bool current_usb = usb_mounted;

        if (current_usb != last_usb_state) {
            last_usb_state = current_usb;

#ifdef DEBUG_USB_PIN
            hal_gpio_put(DEBUG_USB_PIN, current_usb);
#endif

#if defined(DEBUG_USB_RGB_INDEX) && defined(RGB_NUM)
            rgb_set_pixel_raw(DEBUG_USB_RGB_INDEX, current_usb ? DEBUG_USB_RGB_COLOR : 0x000000);
            rgb_show();
#endif
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Early init hook called before scheduler starts
 */
void hook_early_init(void) {
#ifdef DEBUG_KEY_PIN
    hal_gpio_init(DEBUG_KEY_PIN);
    hal_gpio_set_dir(DEBUG_KEY_PIN, true);
    hal_gpio_put(DEBUG_KEY_PIN, false);
#endif

#ifdef DEBUG_USB_PIN
    hal_gpio_init(DEBUG_USB_PIN);
    hal_gpio_set_dir(DEBUG_USB_PIN, true);
    hal_gpio_put(DEBUG_USB_PIN, false);
#endif

    xTaskCreate(debug_indicator_task, "dbg_ind", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
}

/**
 * @brief Matrix switch change hook called on press/release of any key
 */
void hook_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    (void)row;
    (void)col;

    if (pressed) {
        active_keys_counter++;
    } else if (active_keys_counter > 0) {
        active_keys_counter--;
    }

    bool has_active_keys = (active_keys_counter > 0);

#ifdef DEBUG_KEY_PIN
    hal_gpio_put(DEBUG_KEY_PIN, has_active_keys);
#endif

#if defined(DEBUG_KEY_RGB_INDEX) && defined(RGB_NUM)
    rgb_set_pixel_raw(DEBUG_KEY_RGB_INDEX, has_active_keys ? DEBUG_KEY_RGB_COLOR : 0x000000);
    rgb_show();
#endif
}

#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "keys.h"
#include "rgb.h"
#include "sharp_memory_lcd.h"
#include "task.h"
#include "u8g2.h"

#include <stdio.h>
#include <string.h>

extern volatile bool usb_mounted;

static u8g2_t u8g2;
static sharp_lcd_state_t lcd_state = {
    .active_layer = 0,
    .current_wpm = 0,
    .last_keycode = 0,
    .caps_lock = false,
    .num_lock = false,
    .usb_connected = false,
    .rgb_enabled = false,
    .rgb_mode = 0,
    .dirty = true
};

#ifndef SHARP_LCD_PIN_CS
#define SHARP_LCD_PIN_CS PB4
#endif

#ifndef SHARP_LCD_PIN_SCK
#define SHARP_LCD_PIN_SCK PB5
#endif

#ifndef SHARP_LCD_PIN_MOSI
#define SHARP_LCD_PIN_MOSI PB6
#endif

#ifndef SHARP_LCD_PIN_DISP
#define SHARP_LCD_PIN_DISP PB7
#endif

#ifndef SHARP_LCD_PIN_EXTCOMIN
#define SHARP_LCD_PIN_EXTCOMIN PB8
#endif

// Layer names table
static const char *layer_labels[] = {
    "BASE",
    "LOWER",
    "RAISE",
    "ADJUST"
};

// Rolling 5-second window for Words-Per-Minute (WPM) calculation
#define WPM_WINDOW_SECONDS 5
static uint8_t wpm_history[WPM_WINDOW_SECONDS] = {0};
static uint8_t wpm_current_sec_count = 0;
static uint8_t wpm_history_idx = 0;

/**
 * @brief Compute real-time typing speed (WPM) based on 5-character word standard
 */
static void wpm_update_tick(void) {
    wpm_history[wpm_history_idx] = wpm_current_sec_count;
    wpm_current_sec_count = 0;
    wpm_history_idx = (wpm_history_idx + 1) % WPM_WINDOW_SECONDS;

    uint32_t total_strokes = 0;
    for (int i = 0; i < WPM_WINDOW_SECONDS; ++i) {
        total_strokes += wpm_history[i];
    }

    // WPM = (strokes in 5 sec / 5 characters per word) * (60 sec / 5 sec window)
    uint16_t new_wpm = (uint16_t)((total_strokes * 12) / 5);
    if (new_wpm != lcd_state.current_wpm) {
        lcd_state.current_wpm = new_wpm;
        lcd_state.dirty = true;
    }
}

/**
 * @brief U8g2 GPIO and Delay callback for Sharp Memory LCD
 */
static uint8_t u8x8_gpio_and_delay_dmk(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    (void)u8x8;
    (void)arg_ptr;

    switch (msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        hal_gpio_init(SHARP_LCD_PIN_CS);
        hal_gpio_set_dir(SHARP_LCD_PIN_CS, true);
        hal_gpio_put(SHARP_LCD_PIN_CS, 0); // CS active high

        hal_gpio_init(SHARP_LCD_PIN_SCK);
        hal_gpio_set_dir(SHARP_LCD_PIN_SCK, true);
        hal_gpio_put(SHARP_LCD_PIN_SCK, 0);

        hal_gpio_init(SHARP_LCD_PIN_MOSI);
        hal_gpio_set_dir(SHARP_LCD_PIN_MOSI, true);
        hal_gpio_put(SHARP_LCD_PIN_MOSI, 0);

        hal_gpio_init(SHARP_LCD_PIN_DISP);
        hal_gpio_set_dir(SHARP_LCD_PIN_DISP, true);
        hal_gpio_put(SHARP_LCD_PIN_DISP, 1); // Display ON

        hal_gpio_init(SHARP_LCD_PIN_EXTCOMIN);
        hal_gpio_set_dir(SHARP_LCD_PIN_EXTCOMIN, true);
        hal_gpio_put(SHARP_LCD_PIN_EXTCOMIN, 0);
        break;

    case U8X8_MSG_DELAY_NANO:
        hal_sleep_us((arg_int + 999) / 1000);
        break;

    case U8X8_MSG_DELAY_MILLI:
        vTaskDelay(pdMS_TO_TICKS(arg_int > 0 ? arg_int : 1));
        break;

    case U8X8_MSG_GPIO_CS:
        hal_gpio_put(SHARP_LCD_PIN_CS, arg_int != 0);
        break;

    case U8X8_MSG_GPIO_SPI_CLOCK:
        hal_gpio_put(SHARP_LCD_PIN_SCK, arg_int != 0);
        break;

    case U8X8_MSG_GPIO_SPI_DATA:
        hal_gpio_put(SHARP_LCD_PIN_MOSI, arg_int != 0);
        break;

    default:
        return 0;
    }
    return 1;
}

/**
 * @brief Render full dashboard onto 160x68 Sharp Memory LCD
 */
void sharp_memory_lcd_render(void) {
    u8g2_ClearBuffer(&u8g2);

    // --- 1. Header Bar: Brand + USB & RGB Status ---
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 2, 9, "DMK");
    
    // USB Status icon / badge
    if (lcd_state.usb_connected) {
        u8g2_DrawStr(&u8g2, 30, 9, "[USB]");
    } else {
        u8g2_DrawStr(&u8g2, 30, 9, "[NC]");
    }

    // RGB Status badge
    if (lcd_state.rgb_enabled) {
        u8g2_DrawStr(&u8g2, 65, 9, "[RGB]");
    }

    // Last Keycode indicator (top right)
    if (lcd_state.last_keycode >= 0x04 && lcd_state.last_keycode <= 0x1D) {
        char key_buf[8];
        snprintf(key_buf, sizeof(key_buf), "'%c'", 'A' + (lcd_state.last_keycode - 0x04));
        u8g2_DrawStr(&u8g2, 136, 9, key_buf);
    } else if (lcd_state.last_keycode != 0) {
        char key_buf[8];
        snprintf(key_buf, sizeof(key_buf), "0x%02X", lcd_state.last_keycode & 0xFF);
        u8g2_DrawStr(&u8g2, 126, 9, key_buf);
    }
    u8g2_DrawHLine(&u8g2, 0, 12, SHARP_LCD_WIDTH);

    // --- 2. Left Box: Active Layer ---
    u8g2_DrawRFrame(&u8g2, 2, 16, 76, 50, 4);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 6, 26, "ACTIVE LAYER");

    u8g2_SetFont(&u8g2, u8g2_font_7x13B_tr);
    const char *name = (lcd_state.active_layer < 4) 
        ? layer_labels[lcd_state.active_layer] 
        : "CUSTOM";
    u8g2_DrawStr(&u8g2, 8, 43, name);

    char lyr_num[10];
    snprintf(lyr_num, sizeof(lyr_num), "ID: %d", lcd_state.active_layer);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 8, 59, lyr_num);

    // --- 3. Right Box: WPM Gauge & Status Badges ---
    // WPM Counter
    u8g2_DrawRFrame(&u8g2, 82, 16, 76, 30, 4);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    char wpm_buf[16];
    snprintf(wpm_buf, sizeof(wpm_buf), "WPM: %d", lcd_state.current_wpm);
    u8g2_DrawStr(&u8g2, 88, 28, wpm_buf);

    // WPM progress bar (0..120 WPM scaled to 64 pixels)
    uint8_t bar_w = (uint8_t)((lcd_state.current_wpm > 120 ? 120 : lcd_state.current_wpm) * 64 / 120);
    u8g2_DrawFrame(&u8g2, 88, 33, 64, 7);
    if (bar_w > 0) {
        u8g2_DrawBox(&u8g2, 88, 33, bar_w, 7);
    }

    // Caps Lock & Num Lock Badges (Bottom Right)
    if (lcd_state.caps_lock) {
        u8g2_DrawBox(&u8g2, 82, 50, 36, 16);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 86, 62, "CAPS");
        u8g2_SetDrawColor(&u8g2, 1);
    } else {
        u8g2_DrawFrame(&u8g2, 82, 50, 36, 16);
        u8g2_DrawStr(&u8g2, 86, 62, "caps");
    }

    if (lcd_state.num_lock) {
        u8g2_DrawBox(&u8g2, 122, 50, 36, 16);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 126, 62, "NUM");
        u8g2_SetDrawColor(&u8g2, 1);
    } else {
        u8g2_DrawFrame(&u8g2, 122, 50, 36, 16);
        u8g2_DrawStr(&u8g2, 126, 62, "num");
    }

    // Commit frame to Sharp Memory LCD
    u8g2_SendBuffer(&u8g2);
}

/**
 * @brief FreeRTOS task handling display refresh and periodic EXTCOMIN toggling
 */
static void sharp_lcd_task(void *pvParameters) {
    (void)pvParameters;

    u8g2_Setup_ls011b7dh03_160x68_f(
        &u8g2,
        U8G2_R0,
        u8x8_byte_4wire_sw_spi,
        u8x8_gpio_and_delay_dmk
    );

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    TickType_t last_vcom_toggle = xTaskGetTickCount();
    TickType_t last_wpm_tick = xTaskGetTickCount();
    bool extcomin_state = false;

    while (1) {
        TickType_t now = xTaskGetTickCount();

        // 1. Sharp MIP polarity inversion (EXTCOMIN toggle at 1 Hz)
        if ((now - last_vcom_toggle) >= pdMS_TO_TICKS(1000)) {
            last_vcom_toggle = now;
            extcomin_state = !extcomin_state;
            hal_gpio_put(SHARP_LCD_PIN_EXTCOMIN, extcomin_state);
        }

        // 2. WPM Calculation tick every 1000ms
        if ((now - last_wpm_tick) >= pdMS_TO_TICKS(1000)) {
            last_wpm_tick = now;
            wpm_update_tick();
        }

        // 3. Poll USB & RGB subsystem states
        bool current_usb = usb_mounted;
        if (lcd_state.usb_connected != current_usb) {
            lcd_state.usb_connected = current_usb;
            lcd_state.dirty = true;
        }

#if defined(RGB_NUM)
        bool rgb_on = rgb_get_enabled();
        if (lcd_state.rgb_enabled != rgb_on) {
            lcd_state.rgb_enabled = rgb_on;
            lcd_state.dirty = true;
        }
#endif

        // 4. Render display if dirty
        if (lcd_state.dirty) {
            sharp_memory_lcd_render();
            lcd_state.dirty = false;
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

/**
 * @brief Early init hook
 */
void hook_early_init(void) {
    xTaskCreate(sharp_lcd_task, "sharp_lcd", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}

/**
 * @brief Layer switch hook
 */
void hook_layer_change(uint8_t active_layer) {
    if (lcd_state.active_layer != active_layer) {
        lcd_state.active_layer = active_layer;
        lcd_state.dirty = true;
    }
}

/**
 * @brief Keycode transmission hook: records typing metrics and last active keycode
 */
void hook_key_sent(uint16_t keycode, bool pressed) {
    if (pressed) {
        lcd_state.last_keycode = keycode;
        lcd_state.dirty = true;

        // Count printable alphanumeric/punctuation keys towards WPM
        if (keycode >= 0x04 && keycode <= 0x38) {
            wpm_current_sec_count++;
        }
    }
}

/**
 * @brief Host HID keyboard lock indicator hook
 */
void hook_hid_led_change(uint8_t led_mask) {
    bool caps = (led_mask & 0x02) != 0;
    bool num = (led_mask & 0x01) != 0;

    if (lcd_state.caps_lock != caps || lcd_state.num_lock != num) {
        lcd_state.caps_lock = caps;
        lcd_state.num_lock = num;
        lcd_state.dirty = true;
    }
}

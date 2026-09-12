// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// Standard C includes
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Core modules
#include "combos.h"
#include "gamepad.h"
#include "hold_tap.h"
#include "hooks.h"
#include "keyboard.h"
#include "layers.h"
#include "macros.h"
#include "midi.h"
#include "mouse.h"
#include "oneshot.h"

// Project includes
#include "config.h"
#include "hal_gpio.h"
#include "keys.h"
#include "led.h"
#include "matrix.h"
#include "rgb.h"
#include "task_internal.h"
#include "usb.h"

#ifdef VIAL
#include "vial.h"
#endif

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "pico/bootrom.h"
#elif defined(MCU_nrf52840)
#include "nrf.h"
#include "nrf_nvic.h"
#elif defined(MCU_milandr)
#include "MDR32FxQI_bkp.h"
#include "MDR32FxQI_rst_clk.h"
#elif defined(MCU_baikal)
#include "bsp/board_api.h"
#endif

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
#include "encoder.h"
extern void encoder_update_timers(uint32_t delta_ms);
extern void encoder_process_event(uint8_t encoder_idx, bool direction);
#endif

/**
 * @brief Enter bootloader mode for firmware update.
 */
void bootloader_jump(void) {
#if defined(MCU_rp2040) || defined(MCU_rp2350)
    reset_usb_boot(0, 0);
#elif defined(MCU_nrf52840)
    // Magic value to enter Adafruit / UF2 bootloader mode upon system reset
    NRF_POWER->GPREGRET = 0x57;
    NVIC_SystemReset();
#elif defined(MCU_milandr)
    RST_CLK_PCLKcmd(RST_CLK_PCLK_BKP, ENABLE);
    MDR_BKP->REG_00 = 0xDFDB007; // DFU bootloader request signature
    NVIC_SystemReset();
#elif defined(MCU_baikal)
    board_reset_to_bootloader();
#else
    // Fallback: standard CMSIS system reset
    NVIC_SystemReset();
#endif
}

// FreeRTOS queues
extern QueueHandle_t matrix_queue;

// Tracks the exact keycode resolved when key was pressed, ensuring correct release even if layers change
static uint32_t pressed_keycodes[NUM_ROWS][NUM_COLS];

/**
 * @brief Dispatch a single key event (press/release) directly to the USB HID report engine (Zero-Queue Fast Path).
 * @param keycode 16-bit USB HID / Consumer keycode.
 * @param pressed True for keydown, false for keyup.
 */
void keyboard_send_key(uint16_t keycode, bool pressed) {
    usb_process_key(keycode, pressed);
    hook_key_sent(keycode, pressed);
}

/**
 * @brief Send modifier keys (Ctrl, Shift, Alt, GUI) based on a bitmask or HID modifier keycode.
 * @param mod_mask Bitmask (MOD_LCTRL, etc.) or HID modifier code (0xE0..0xE7).
 * @param pressed True for keydown, false for keyup.
 */
void keyboard_send_modifiers(uint8_t mod_mask, bool pressed) {
    // If given a HID modifier keycode (0xE0 - 0xE7), convert to standard bitmask
    if (mod_mask >= 0xE0 && mod_mask <= 0xE7) {
        mod_mask = 1 << (mod_mask - 0xE0);
    }

    for (uint8_t i = 0; i < 8; i++) {
        if (mod_mask & (1 << i)) {
            keyboard_send_key(0xE0 + i, pressed);
        }
    }
}

/**
 * @brief Initialize all keyboard submodules (layers, hold-tap, one-shot, combos, MIDI, Vial).
 */
void keyboard_init(void) {
    layers_init();
    hold_tap_init();
    oneshot_init();
    combos_init();
    dmk_midi_init();
    mouse_init();
    gamepad_init();
    memset(pressed_keycodes, 0, sizeof(pressed_keycodes));

#ifdef VIAL
    vial_init();
#endif
}

/**
 * @brief Process decoded 32-bit keycode actions (MIDI, Macros, Layer Switch, One-Shot, Hold-Tap, RGB, Standard HID).
 * @param row Matrix row index.
 * @param col Matrix column index.
 * @param key 32-bit composite keycode.
 * @param pressed True for press, false for release.
 */
void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed) {
    if (key == 0 || key == K_NULL)
        return;

    // 0. User module custom keycode interception hook
    if (hook_process_key(key, pressed)) {
        return;
    }

    // 1. Process MIDI keycodes (notes, CC, pitch bend, octave changes)
    if (dmk_midi_process_keycode(key, pressed)) {
        return;
    }

    // 2. Process dynamic/static macro playback
    if (macros_process_key(key, pressed)) {
        return;
    }

    // 3. Raw layer Momentary Activation (L_0 <= key <= L_15)
    if (key >= L_0 && key <= L_15) {
        uint8_t lyr = (uint8_t)(key - L_0);
        if (pressed) {
            layers_on(lyr);
        } else {
            layers_off(lyr);
        }
        return;
    }

    // 4. Explicit MO(layer) momentary layer switch
    if ((key & 0xFF000000) == DMK_MO) {
        uint8_t lyr = (uint8_t)(key & 0xFF);
        if (pressed) {
            layers_on(lyr);
        } else {
            layers_off(lyr);
        }
        return;
    }

    // 5. Toggle Layer (TG(layer)) persistent state toggle
    if ((key & 0xFF000000) == DMK_TG) {
        uint8_t lyr = (uint8_t)(key & 0xFF);
        if (pressed) {
            layers_toggle(lyr);
        }
        return;
    }

    // 6. Modified Key (e.g. LSFT(KC_A), LCTL(KC_C))
    if ((key & 0xFF000000) == DMK_MK) {
        uint8_t mod_mask = (uint8_t)((key >> 8) & 0xFF);
        uint8_t kc = (uint8_t)(key & 0xFF);
        if (pressed) {
            keyboard_send_modifiers(mod_mask, true);
            keyboard_send_key(kc, true);
        } else {
            keyboard_send_key(kc, false);
            keyboard_send_modifiers(mod_mask, false);
        }
        return;
    }

    // 7. One Shot Key (OSM modifiers / OSL layers)
    if (oneshot_process_event(key, pressed)) {
        return;
    }

    // 8. Hold-Tap Dual-Role Key (e.g. LT layer-tap, MT mod-tap)
    if (hold_tap_process_event(row, col, key, pressed)) {
        return;
    }

    // 9. Mouse keys (buttons, movement, wheel, acceleration)
    if (mouse_process_key(key, pressed)) {
        return;
    }

    // 10. Gamepad keys (buttons, D-Pad, simulated analog sticks/triggers)
    if (gamepad_process_key(key, pressed)) {
        return;
    }

    // 10. Standard HID, Consumer Media, and Lighting Controls
    if (key == K_LYRUP) {
        if (pressed && (layers_get_active() + 1 < layers_get_count())) {
            layers_on(layers_get_active() + 1);
            led_on();
        }
    } else if (key == K_LYRDWN) {
        if (pressed && (layers_get_active() > 0)) {
            layers_off(layers_get_active());
            led_off();
        }
    } else if (key >= K_RGB_TOGG && key <= K_RGB_SPD) {
        static void (*const rgb_actions[])(void) = {
            rgb_toggle,       rgb_next_theme,     rgb_prev_theme,     rgb_increase_hue,
            rgb_decrease_hue, rgb_increase_sat,   rgb_decrease_sat,   rgb_increase_val,
            rgb_decrease_val, rgb_increase_speed, rgb_decrease_speed,
        };
        if (pressed) {
            rgb_actions[key - K_RGB_TOGG]();
        }
    } else if (key == K_BOOTLOADER) {
        if (pressed)
            bootloader_jump();
    } else if ((key & 0xFF000000) == DMK_CONSUMER) {
        uint16_t consumer_usage = (uint16_t)(key & 0xFFFF);
        keyboard_send_key((uint16_t)(consumer_usage | KEY_CONSUMER_FLAG), pressed);
    } else {
        keyboard_send_key((uint16_t)key, pressed);
    }
}

/**
 * @brief Core event processing loop: checks timeouts across all subsystems and dispatches matrix events.
 */
void keyboard_check(void) {
    TickType_t now = xTaskGetTickCount();

    // 1. Process active timeouts and determine earliest wakeup deadline
    TickType_t next_deadline = portMAX_DELAY;
    TickType_t r;
    if ((r = hold_tap_check_timeouts(now)) < next_deadline)
        next_deadline = r;
    if ((r = oneshot_check_timeouts(now)) < next_deadline)
        next_deadline = r;
    if ((r = combos_check_timeouts(now)) < next_deadline)
        next_deadline = r;
    if ((r = mouse_check_timeouts(now)) < next_deadline)
        next_deadline = r;

    led_update(now);

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
    static TickType_t last_encoder_time = 0;
    if (last_encoder_time == 0)
        last_encoder_time = now;
    uint32_t delta = (now - last_encoder_time) * (1000 / configTICK_RATE_HZ);
    if (delta > 0) {
        encoder_update_timers(delta);
        last_encoder_time = now;
    }
    if (next_deadline > pdMS_TO_TICKS(10)) {
        next_deadline = pdMS_TO_TICKS(10);
    }
#else
    if (next_deadline > pdMS_TO_TICKS(50)) {
        next_deadline = pdMS_TO_TICKS(50);
    }
#endif

    if (next_deadline < 1)
        next_deadline = 1;

    // 2. Receive and process matrix events
    matrix_event_t matrix_event;
    if (pdTRUE == xQueueReceive(matrix_queue, &matrix_event, next_deadline)) {
        do {
            uint8_t row = matrix_event.row;
            uint8_t col = matrix_event.col;
            bool pressed = matrix_event.pressed;

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
            if (row == 0xFF) {
                encoder_process_event(col, pressed);
                continue;
            }
#endif

#ifdef LED_ACTIVITY_PIN
            static bool led_act_inited = false;
            static uint32_t active_keys_count = 0;
            if (!led_act_inited) {
                hal_gpio_init(LED_ACTIVITY_PIN);
                hal_gpio_set_dir(LED_ACTIVITY_PIN, true);
                hal_gpio_put(LED_ACTIVITY_PIN, false);
                led_act_inited = true;
            }
            if (pressed) {
                active_keys_count++;
            } else if (active_keys_count > 0) {
                active_keys_count--;
            }
            hal_gpio_put(LED_ACTIVITY_PIN, active_keys_count > 0);
#endif

            // Notify user modules of matrix events (if hook returns true, event was consumed)
            if (hook_matrix_change(row, col, pressed)) {
                continue;
            }

            now = xTaskGetTickCount();

            if (pressed) {
                // Check combos / chords
                if (combos_process_event(row, col, true, now)) {
                    continue;
                }

                // Permissive Hold: resolve other pending HT keys immediately
                hold_tap_permissive_resolve(row, col);

                // Resolve key from active layer stack
                uint32_t key = layers_lookup_key(row, col);
                pressed_keycodes[row][col] = key;

                // Notify One-Shot subsystem of key press
                oneshot_on_key_press(key);

                // Process action
                process_key_event(row, col, key, true);
            } else {
                // Check combos / chords
                if (combos_process_event(row, col, false, now)) {
                    continue;
                }

                uint32_t key = pressed_keycodes[row][col];
                pressed_keycodes[row][col] = 0;

                // Process action release
                process_key_event(row, col, key, false);

                // Notify One-Shot subsystem of key release
                oneshot_on_key_release();
            }
        } while (pdTRUE == xQueueReceive(matrix_queue, &matrix_event, 0));
    }
}

/**
 * @brief FreeRTOS task responsible for keyboard logic and state machine.
 */
void keyboard_task(void *pvParameters) {
    (void)pvParameters;
    keyboard_init();
    while (1) {
        keyboard_check();
    }
}

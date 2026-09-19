#define LED_C_SRC 1
#include "led.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "task.h"

#ifdef LED_PINS
// Physical LED pin mapping table
static const pin_t led_pins[] = LED_PINS;
#define LED_COUNT (sizeof(led_pins) / sizeof(led_pins[0]))
#endif

static inline void Board_LED_On(void) {
#ifdef LED_PINS
#ifdef LED_DEBUG
    if (LED_DEBUG < LED_COUNT) {
        hal_gpio_put(led_pins[LED_DEBUG], true);
    }
#endif
#endif
}
static inline void Board_LED_Off(void) {
#ifdef LED_PINS
#ifdef LED_DEBUG
    if (LED_DEBUG < LED_COUNT) {
        hal_gpio_put(led_pins[LED_DEBUG], false);
    }
#endif
#endif
}

bool led_initialized = false;
extern volatile bool usb_mounted;

static TickType_t blink_until = 0;
static bool is_blinking = false;
static TickType_t last_toggle_time = 0;
static bool led_heartbeat_state = false;

/**
 * @brief Initialize configured status and lock LED GPIO pins as outputs.
 */
void led_init(void) {
#ifdef LED_PINS
    for (uint32_t i = 0; i < LED_COUNT; i++) {
        hal_gpio_init(led_pins[i]);
        hal_gpio_set_dir(led_pins[i], true); // Output
        hal_gpio_put(led_pins[i], false);    // Default off
    }
#endif

    led_initialized = true;
}

/**
 * @brief Turn on debug LED.
 */
void led_on(void) {
    Board_LED_On();
}

/**
 * @brief Turn off debug LED.
 */
void led_off(void) {
    Board_LED_Off();
}

/**
 * @brief Periodic update called from keyboard loop to service debug blink & heartbeat.
 */
void led_update(TickType_t now) {
#ifdef LED_PINS
#ifdef LED_DEBUG
    if (LED_DEBUG >= LED_COUNT) {
        return;
    }

    if (is_blinking) {
        if (now >= blink_until) {
            Board_LED_Off();
            is_blinking = false;
        }
        return;
    }

    // Heartbeat toggle: 250ms when USB mounted, 1000ms when disconnected
    TickType_t interval = usb_mounted ? pdMS_TO_TICKS(250) : pdMS_TO_TICKS(1000);
    if ((now - last_toggle_time) >= interval) {
        last_toggle_time = now;
        led_heartbeat_state = !led_heartbeat_state;
        hal_gpio_put(led_pins[LED_DEBUG], led_heartbeat_state);
    }
#else
    (void)now;
#endif
#else
    (void)now;
#endif
}

/**
 * @brief Determine earliest wakeup deadline needed by LED subsystem (blink or heartbeat).
 * @param now Current FreeRTOS tick count.
 * @return Next timeout in ticks, or portMAX_DELAY if no LED animations/timers active.
 */
TickType_t led_check_timeouts(TickType_t now) {
#ifdef LED_PINS
#ifdef LED_DEBUG
    if (LED_DEBUG >= LED_COUNT) {
        return portMAX_DELAY;
    }
    if (is_blinking) {
        if (now >= blink_until) {
            Board_LED_Off();
            is_blinking = false;
        } else {
            return (blink_until - now);
        }
    }
    TickType_t interval = usb_mounted ? pdMS_TO_TICKS(250) : pdMS_TO_TICKS(1000);
    TickType_t elapsed = now - last_toggle_time;
    if (elapsed >= interval) {
        return 1;
    }
    return (interval - elapsed);
#else
    (void)now;
    return portMAX_DELAY;
#endif
#else
    (void)now;
    return portMAX_DELAY;
#endif
}

/**
 * @brief Update host keyboard lock LEDs (Num Lock, Caps Lock, Scroll Lock, Compose, Kana).
 * @param state Host HID LED indicator bitmask.
 */
void led_set_hid_state(uint8_t state) {
#ifdef LED_PINS
#ifdef LED_HID_NUM_LOCK
    if (LED_HID_NUM_LOCK < LED_COUNT) {
        hal_gpio_put(led_pins[LED_HID_NUM_LOCK], (state & 0x01) ? true : false);
    }
#endif
#ifdef LED_HID_CAPS_LOCK
    if (LED_HID_CAPS_LOCK < LED_COUNT) {
        hal_gpio_put(led_pins[LED_HID_CAPS_LOCK], (state & 0x02) ? true : false);
    }
#endif
#ifdef LED_HID_SCROLL_LOCK
    if (LED_HID_SCROLL_LOCK < LED_COUNT) {
        hal_gpio_put(led_pins[LED_HID_SCROLL_LOCK], (state & 0x04) ? true : false);
    }
#endif
#ifdef LED_HID_COMPOSE
    if (LED_HID_COMPOSE < LED_COUNT) {
        hal_gpio_put(led_pins[LED_HID_COMPOSE], (state & 0x08) ? true : false);
    }
#endif
#ifdef LED_HID_KANA
    if (LED_HID_KANA < LED_COUNT) {
        hal_gpio_put(led_pins[LED_HID_KANA], (state & 0x10) ? true : false);
    }
#endif
#endif
    hook_hid_led_change(state);
}

void led_activity(bool pressed) {
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
#else
    (void)pressed;
#endif
}

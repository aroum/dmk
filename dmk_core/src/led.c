#include "led.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "queue.h"
#include "task.h"

#ifdef LED_PINS
// Physical LED pin mapping table
static const pin_t led_pins[] = LED_PINS;
#define LED_COUNT (sizeof(led_pins) / sizeof(led_pins[0]))
#endif

#define Board_GPIO_Init()                                                                                              \
    do {                                                                                                               \
    } while (0)
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
extern QueueHandle_t led_queue;

/**
 * @brief Initialize configured status and lock LED GPIO pins as outputs.
 */
void led_init(void) {
    Board_GPIO_Init();

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
 * @brief Trigger a non-blocking activity blink on the debug LED.
 */
void led_blink(void) {
    if (led_initialized) {
        bool test = true;
        xQueueSend(led_queue, &test, 0);
    }
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

// Global USB mount status flag (synced from USB stack)
volatile bool usb_mounted = false;

/**
 * @brief FreeRTOS task handling LED heartbeat blink and activity indicator pulses.
 */
void led_task(void *pvParameters) {
    (void)pvParameters;
    led_init();

    TickType_t last_toggle_time = xTaskGetTickCount();
    TickType_t blink_until = 0;
    bool led_state = false;
    bool is_blinking = false;

    while (1) {
        TickType_t now = xTaskGetTickCount();

        // Handle non-blocking activity blink request
        bool trigger;
        if (pdTRUE == xQueueReceive(led_queue, &trigger, 0)) {
            Board_LED_On();
            blink_until = now + pdMS_TO_TICKS(25);
            is_blinking = true;
        }

        if (is_blinking && now >= blink_until) {
            Board_LED_Off();
            is_blinking = false;
        }

        // Heartbeat toggle: 250ms when USB mounted, 1000ms when disconnected
        TickType_t interval = usb_mounted ? pdMS_TO_TICKS(250) : pdMS_TO_TICKS(1000);
        if ((now - last_toggle_time) >= interval) {
            last_toggle_time = now;
            led_state = !led_state;
#ifdef LED_PINS
#ifdef LED_DEBUG
            if (LED_DEBUG < LED_COUNT && !is_blinking) {
                hal_gpio_put(led_pins[LED_DEBUG], led_state);
            }
#endif
#endif
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
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
}

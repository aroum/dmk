#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "led.h"
#include "split.h"

TickType_t g_mock_ticks;
volatile bool usb_mounted;
static bool pins[4], master;
static uint8_t mirrored_mask;
void hal_gpio_init(pin_t pin) { (void)pin; }
void hal_gpio_set_dir(pin_t pin, bool output) { (void)pin; (void)output; }
void hal_gpio_put(pin_t pin, bool value) { assert(pin < 4); pins[pin] = value; }
bool hal_gpio_get(pin_t pin) { (void)pin; return true; }
void hal_gpio_pull_up(pin_t pin) { (void)pin; }
void hal_sleep_us(uint32_t us) { (void)us; }
bool is_master(void) { return master; }
void split_set_remote_leds(uint8_t mask) { if (master) mirrored_mask = mask; }
void hooks_dispatch_hid_led_change(uint8_t state) { (void)state; }

int main(void) {
    master = true;
    led_init();
    led_on();
    assert(pins[1] && mirrored_mask == 1);
    led_set_hid_state(2); // Caps configured as LED_PINS[1]
    assert(pins[2] && mirrored_mask == 3);
    master = false;
    led_set_output_mask(4);
    assert(!pins[1] && !pins[2] && pins[3]);
    led_on(); // a local heartbeat must not overwrite the master's command
    assert(!pins[1] && pins[3]);
    master = true;
    led_on(); // a new master resumes local ownership of its LEDs
    assert(pins[1] && pins[3] && mirrored_mask == 5);
    puts("split physical indicator tests passed");
    return 0;
}

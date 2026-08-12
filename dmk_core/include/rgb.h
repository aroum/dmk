#ifndef RGB_H
#define RGB_H

#include <stdbool.h>
#include <stdint.h>

// Standard QMK RGBLIGHT mode definitions
#define RGBLIGHT_MODE_STATIC_LIGHT 1
#define RGBLIGHT_MODE_BREATHING 2
#define RGBLIGHT_MODE_RAINBOW_MOOD 6
#define RGBLIGHT_MODE_RAINBOW_SWIRL 9
#define RGBLIGHT_MODE_SNAKE 15
#define RGBLIGHT_MODE_KNIGHT 18
#define RGBLIGHT_MODE_CHRISTMAS 21
#define RGBLIGHT_MODE_STATIC_GRADIENT 22

void rgb_init(void);
void rgb_task(void *pvParameters);
void rgb_toggle(void);
void rgb_next_theme(void);
void rgb_prev_theme(void);

// Getters and setters for Vial GUI control
void rgb_set_enabled(bool enabled);
bool rgb_get_enabled(void);
void rgb_set_mode(uint8_t mode);
uint8_t rgb_get_mode(void);
void rgb_set_brightness(uint8_t brightness);
uint8_t rgb_get_brightness(void);
void rgb_set_speed(uint8_t speed);
uint8_t rgb_get_speed(void);
void rgb_set_color(uint8_t hue, uint8_t sat);
uint8_t rgb_get_hue(void);
uint8_t rgb_get_sat(void);

void rgb_increase_hue(void);
void rgb_decrease_hue(void);
void rgb_increase_sat(void);
void rgb_decrease_sat(void);
void rgb_increase_val(void);
void rgb_decrease_val(void);
void rgb_increase_speed(void);
void rgb_decrease_speed(void);

// Serialization for EEPROM storage
void rgb_get_config(uint8_t *dest);
void rgb_set_config(const uint8_t *src);

#endif // RGB_H

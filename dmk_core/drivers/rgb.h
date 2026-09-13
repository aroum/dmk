#ifndef RGB_H
#define RGB_H

#include "config.h"
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

#if !defined(NO_RGB) && defined(RGB_NUM) && (RGB_NUM > 0)

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

// Low-level individual LED control
void rgb_set_pixel_raw(uint32_t index, uint32_t color);
void rgb_show(void);

// Serialization for EEPROM storage
void rgb_get_config(uint8_t *dest);
void rgb_set_config(const uint8_t *src);

#else

static inline void rgb_init(void) {}
static inline void rgb_task(void *pvParameters) {
    (void)pvParameters;
}
static inline void rgb_toggle(void) {}
static inline void rgb_next_theme(void) {}
static inline void rgb_prev_theme(void) {}

static inline void rgb_set_enabled(bool enabled) {
    (void)enabled;
}
static inline bool rgb_get_enabled(void) {
    return false;
}
static inline void rgb_set_mode(uint8_t mode) {
    (void)mode;
}
static inline uint8_t rgb_get_mode(void) {
    return 0;
}
static inline void rgb_set_brightness(uint8_t brightness) {
    (void)brightness;
}
static inline uint8_t rgb_get_brightness(void) {
    return 0;
}
static inline void rgb_set_speed(uint8_t speed) {
    (void)speed;
}
static inline uint8_t rgb_get_speed(void) {
    return 0;
}
static inline void rgb_set_color(uint8_t hue, uint8_t sat) {
    (void)hue;
    (void)sat;
}
static inline uint8_t rgb_get_hue(void) {
    return 0;
}
static inline uint8_t rgb_get_sat(void) {
    return 0;
}

static inline void rgb_increase_hue(void) {}
static inline void rgb_decrease_hue(void) {}
static inline void rgb_increase_sat(void) {}
static inline void rgb_decrease_sat(void) {}
static inline void rgb_increase_val(void) {}
static inline void rgb_decrease_val(void) {}
static inline void rgb_increase_speed(void) {}
static inline void rgb_decrease_speed(void) {}

static inline void rgb_set_pixel_raw(uint32_t index, uint32_t color) {
    (void)index;
    (void)color;
}
static inline void rgb_show(void) {}

static inline void rgb_get_config(uint8_t *dest) {
    (void)dest;
}
static inline void rgb_set_config(const uint8_t *src) {
    (void)src;
}

#endif

#endif // RGB_H

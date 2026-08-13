#include "rgb.h"
#include "config.h"

#ifndef RGB_LIMIT_VAL
#define RGB_LIMIT_VAL 255
#endif

#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include <string.h>

#ifdef POWER_PIN
#if defined(MCU_nrf52840)
#include "nrf_gpio.h"
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/gpio.h"
#elif defined(MCU_milandr)
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#endif
#endif

#if defined(RGB_NUM) && (defined(MCU_rp2040) || defined(MCU_rp2350) || defined(MCU_nrf52840) || defined(MCU_milandr))

// External declarations for the platform-specific WS2812 driver wrapper
extern void ws2812_init(uint32_t pin, uint32_t length);
extern void ws2812_set_color(uint32_t index, uint32_t color);
extern void ws2812_show(void);

#ifdef POWER_PIN
static void power_pin_init(void) {
#if defined(MCU_nrf52840)
    nrf_gpio_cfg_output(POWER_PIN);
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
#elif defined(MCU_milandr)
    PORT_InitTypeDef port_init;
    port_init.PORT_Pin = 1 << (POWER_PIN & 0x0F);
    port_init.PORT_MODE = PORT_MODE_DIGITAL;
    port_init.PORT_OE = PORT_OE_OUT;
    port_init.PORT_FUNC = PORT_FUNC_PORT;
    port_init.PORT_SPEED = PORT_SPEED_FAST;
    port_init.PORT_PULL_UP = PORT_PULL_UP_OFF;
    port_init.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;

    MDR_PORT_TypeDef *port;
    uint32_t port_idx = POWER_PIN / 16;
    if (port_idx == 0) {
        port = MDR_PORTA;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA, ENABLE);
    } else if (port_idx == 1) {
        port = MDR_PORTB;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);
    } else if (port_idx == 2) {
        port = MDR_PORTC;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTC, ENABLE);
    } else if (port_idx == 3) {
        port = MDR_PORTD;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);
    } else if (port_idx == 4) {
        port = MDR_PORTE;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTE, ENABLE);
    } else {
        port = MDR_PORTF;
        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);
    }
    PORT_Init(port, &port_init);
#endif
}

static void power_pin_set(bool on) {
#if defined(MCU_nrf52840)
    if (on) {
        nrf_gpio_pin_set(POWER_PIN);
    } else {
        nrf_gpio_pin_clear(POWER_PIN);
    }
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
    gpio_put(POWER_PIN, on ? 1 : 0);
#elif defined(MCU_milandr)
    MDR_PORT_TypeDef *port;
    uint32_t port_idx = POWER_PIN / 16;
    if (port_idx == 0)
        port = MDR_PORTA;
    else if (port_idx == 1)
        port = MDR_PORTB;
    else if (port_idx == 2)
        port = MDR_PORTC;
    else if (port_idx == 3)
        port = MDR_PORTD;
    else if (port_idx == 4)
        port = MDR_PORTE;
    else
        port = MDR_PORTF;

    uint32_t pin_mask = 1 << (POWER_PIN & 0x0F);
    if (on) {
        PORT_SetBits(port, pin_mask);
    } else {
        PORT_ResetBits(port, pin_mask);
    }
#endif
}
#endif

// RGB configuration state
static bool rgb_enabled = true;
#ifdef RGB_THEMES
static uint8_t rgb_mode = 100; // Default to Theme mode (100) if themes are defined
#else
static uint8_t rgb_mode = RGBLIGHT_MODE_STATIC_LIGHT;
#endif
static uint8_t rgb_brightness = 255;
static uint8_t rgb_hue = 0;
static uint8_t rgb_sat = 255;
static uint8_t rgb_speed = 128; // Standard default speed

#ifdef RGB_THEMES
static const uint32_t themes[][RGB_NUM] = RGB_THEMES;
static const uint8_t theme_count = sizeof(themes) / sizeof(themes[0]);
static uint8_t active_theme = 0;
#endif

#ifdef RGB_MAP
static const uint8_t rgb_map[RGB_NUM] = RGB_MAP;
#endif

/**
 * @brief Fast integer-based HSV to 0x00RRGGBB color converter.
 * @param h Hue (0..255).
 * @param s Saturation (0..255).
 * @param v Value / Brightness (0..255).
 * @return 32-bit packed RGB color.
 */
static uint32_t hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    if (v > RGB_LIMIT_VAL) {
        v = RGB_LIMIT_VAL;
    }
    if (s == 0) {
        return ((uint32_t)v << 16) | ((uint32_t)v << 8) | v;
    }

    uint32_t region = h / 43;
    uint32_t remainder = (h - (region * 43)) * 6;

    uint32_t p = (v * (255 - s)) >> 8;
    uint32_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint32_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
    case 0:
        return (v << 16) | (t << 8) | p;
    case 1:
        return (q << 16) | (v << 8) | p;
    case 2:
        return (p << 16) | (v << 8) | t;
    case 3:
        return (p << 16) | (q << 8) | v;
    case 4:
        return (t << 16) | (p << 8) | v;
    default:
        return (v << 16) | (p << 8) | q;
    }
}

/**
 * @brief Set color of a specific logical LED, applying RGB_MAP physical reordering if configured.
 * @param index Logical LED index.
 * @param color 32-bit packed RGB color.
 */
static void set_led_color(uint32_t index, uint32_t color) {
#ifdef RGB_MAP
    if (index < RGB_NUM) {
        ws2812_set_color(rgb_map[index], color);
    }
#else
    ws2812_set_color(index, color);
#endif
}

/**
 * @brief Initialize power gating pin and WS2812 driver hardware (PIO/SPI/PWM/Timer).
 */
void rgb_init(void) {
#ifdef POWER_PIN
    power_pin_init();
    // Power the LED strip immediately so ws2812_init has stable VCC
    power_pin_set(true);
#endif
#ifdef RGB_PIN
    ws2812_init(RGB_PIN, RGB_NUM);
#endif
}

static uint32_t animation_tick = 0;
static uint32_t step_accum = 0;

/**
 * @brief FreeRTOS task rendering active RGB animation effects (Rainbow, Breathing, Swirl, etc.).
 */
void rgb_task(void *pvParameters) {
    (void)pvParameters;

    while (1) {
#ifdef POWER_PIN
        power_pin_set(rgb_enabled && rgb_mode != 0);
#endif
        if (rgb_enabled && rgb_mode != 0) {
            // Scale tick progression based on speed, clamped to minimum 16 to
            // prevent animation freeze from a zero-speed value in EEPROM
            uint8_t speed = rgb_speed;
            if (speed < 16)
                speed = 16;
            step_accum += speed;
            uint32_t ticks_to_advance = step_accum / 128;
            step_accum %= 128;
            animation_tick += ticks_to_advance;

            switch (rgb_mode) {
            case RGBLIGHT_MODE_STATIC_LIGHT: {
                // Solid Color Mode
                uint32_t color = hsv_to_rgb(rgb_hue, rgb_sat, rgb_brightness);
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    set_led_color(i, color);
                }
                break;
            }

            case RGBLIGHT_MODE_BREATHING: {
                // Breathing Mode
                uint8_t breath_step = (animation_tick / 4) & 0xFF;
                uint8_t duty = (breath_step < 128) ? (breath_step * 2) : ((255 - breath_step) * 2);
                // Quadratic scaling for smoother human eye response
                uint8_t breathing_brightness = ((uint32_t)duty * duty * rgb_brightness) >> 16;
                uint32_t color = hsv_to_rgb(rgb_hue, rgb_sat, breathing_brightness);
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    set_led_color(i, color);
                }
                break;
            }

            case RGBLIGHT_MODE_RAINBOW_MOOD: {
                // Rainbow Mood Mode (all LEDs cycle hue uniformly)
                uint8_t hue = (animation_tick / 8) & 0xFF;
                uint32_t color = hsv_to_rgb(hue, rgb_sat, rgb_brightness);
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    set_led_color(i, color);
                }
                break;
            }

            case RGBLIGHT_MODE_RAINBOW_SWIRL: {
                // Rainbow Swirl Mode (gradient color wheel moving across the strip)
                uint8_t base_hue = (animation_tick / 4) & 0xFF;
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    uint8_t hue = (base_hue + (i * 256 / RGB_NUM)) & 0xFF;
                    uint32_t color = hsv_to_rgb(hue, rgb_sat, rgb_brightness);
                    set_led_color(i, color);
                }
                break;
            }

            case RGBLIGHT_MODE_SNAKE: {
// Snake Mode (wrapping block of LEDs moving along the strip)
#define SNAKE_LENGTH 4
                uint32_t head = (animation_tick / 8) % RGB_NUM;
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    bool is_snake = false;
                    uint8_t intensity = 0;
                    for (uint32_t j = 0; j < SNAKE_LENGTH; j++) {
                        uint32_t pos = (head + RGB_NUM - j) % RGB_NUM;
                        if (i == pos) {
                            is_snake = true;
                            intensity = rgb_brightness * (SNAKE_LENGTH - j) / SNAKE_LENGTH;
                            break;
                        }
                    }
                    if (is_snake) {
                        set_led_color(i, hsv_to_rgb(rgb_hue, rgb_sat, intensity));
                    } else {
                        set_led_color(i, 0);
                    }
                }
                break;
            }

            case RGBLIGHT_MODE_KNIGHT: {
                // Knight Rider Scanner Mode
                uint32_t period = (RGB_NUM > 1) ? (RGB_NUM - 1) * 2 : 1;
                uint32_t step = (animation_tick / 8) % period;
                uint32_t scanner_pos = (step < RGB_NUM) ? step : (period - step);

                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    if (i == scanner_pos) {
                        set_led_color(i, hsv_to_rgb(rgb_hue, rgb_sat, rgb_brightness));
                    } else if (i == scanner_pos + 1 || (scanner_pos > 0 && i == scanner_pos - 1)) {
                        set_led_color(i, hsv_to_rgb(rgb_hue, rgb_sat, rgb_brightness / 2));
                    } else if (i == scanner_pos + 2 || (scanner_pos > 1 && i == scanner_pos - 2)) {
                        set_led_color(i, hsv_to_rgb(rgb_hue, rgb_sat, rgb_brightness / 4));
                    } else {
                        set_led_color(i, 0);
                    }
                }
                break;
            }

            case RGBLIGHT_MODE_CHRISTMAS: {
                // Christmas Mode (alternating red and green colors)
                uint8_t swap = (animation_tick / 32) & 1;
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    uint8_t color_type = (i + swap) & 1;
                    if (color_type == 0) {
                        set_led_color(i, hsv_to_rgb(0, 255, rgb_brightness)); // Red
                    } else {
                        set_led_color(i, hsv_to_rgb(85, 255, rgb_brightness)); // Green
                    }
                }
                break;
            }

            case RGBLIGHT_MODE_STATIC_GRADIENT: {
                // Static Gradient Mode
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    uint8_t hue = (rgb_hue + (i * 256 / RGB_NUM)) & 0xFF;
                    set_led_color(i, hsv_to_rgb(hue, rgb_sat, rgb_brightness));
                }
                break;
            }

#ifdef RGB_THEMES
            case 100: {
                // Theme Mode
                for (uint32_t i = 0; i < RGB_NUM; i++) {
                    uint32_t color = themes[active_theme][i];
                    uint8_t brightness = rgb_brightness;
                    if (brightness > RGB_LIMIT_VAL) {
                        brightness = RGB_LIMIT_VAL;
                    }
                    uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
                    uint8_t g = ((color >> 8) & 0xFF) * brightness / 255;
                    uint8_t b = (color & 0xFF) * brightness / 255;
                    uint32_t adjusted = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
                    set_led_color(i, adjusted);
                }
                break;
            }
#endif

            default:
                break;
            }
            ws2812_show();
        } else {
            // Turn off all LEDs
            for (uint32_t i = 0; i < RGB_NUM; i++) {
                set_led_color(i, 0);
            }
            ws2812_show();
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // run at 50fps for smooth animations
    }
}

void rgb_toggle(void) {
    rgb_enabled = !rgb_enabled;
    if (rgb_enabled) {
#ifdef RGB_THEMES
        rgb_mode = 100;
#else
        rgb_mode = RGBLIGHT_MODE_STATIC_LIGHT;
#endif
    } else {
        rgb_mode = 0;
    }
}

static uint8_t get_next_mode(uint8_t current_mode) {
    switch (current_mode) {
    case RGBLIGHT_MODE_STATIC_LIGHT:
        return RGBLIGHT_MODE_BREATHING;
    case RGBLIGHT_MODE_BREATHING:
        return RGBLIGHT_MODE_RAINBOW_MOOD;
    case RGBLIGHT_MODE_RAINBOW_MOOD:
        return RGBLIGHT_MODE_RAINBOW_SWIRL;
    case RGBLIGHT_MODE_RAINBOW_SWIRL:
        return RGBLIGHT_MODE_SNAKE;
    case RGBLIGHT_MODE_SNAKE:
        return RGBLIGHT_MODE_KNIGHT;
    case RGBLIGHT_MODE_KNIGHT:
        return RGBLIGHT_MODE_CHRISTMAS;
    case RGBLIGHT_MODE_CHRISTMAS:
        return RGBLIGHT_MODE_STATIC_GRADIENT;
    case RGBLIGHT_MODE_STATIC_GRADIENT:
#ifdef RGB_THEMES
        return 100; // Theme Mode
#else
        return RGBLIGHT_MODE_STATIC_LIGHT;
#endif
#ifdef RGB_THEMES
    case 100:
        return RGBLIGHT_MODE_STATIC_LIGHT;
#endif
    default:
        return RGBLIGHT_MODE_STATIC_LIGHT;
    }
}

static uint8_t get_prev_mode(uint8_t current_mode) {
    switch (current_mode) {
    case RGBLIGHT_MODE_STATIC_LIGHT:
#ifdef RGB_THEMES
        return 100; // Theme Mode
#else
        return RGBLIGHT_MODE_STATIC_GRADIENT;
#endif
#ifdef RGB_THEMES
    case 100:
        return RGBLIGHT_MODE_STATIC_GRADIENT;
#endif
    case RGBLIGHT_MODE_BREATHING:
        return RGBLIGHT_MODE_STATIC_LIGHT;
    case RGBLIGHT_MODE_RAINBOW_MOOD:
        return RGBLIGHT_MODE_BREATHING;
    case RGBLIGHT_MODE_RAINBOW_SWIRL:
        return RGBLIGHT_MODE_RAINBOW_MOOD;
    case RGBLIGHT_MODE_SNAKE:
        return RGBLIGHT_MODE_RAINBOW_SWIRL;
    case RGBLIGHT_MODE_KNIGHT:
        return RGBLIGHT_MODE_SNAKE;
    case RGBLIGHT_MODE_CHRISTMAS:
        return RGBLIGHT_MODE_KNIGHT;
    case RGBLIGHT_MODE_STATIC_GRADIENT:
        return RGBLIGHT_MODE_CHRISTMAS;
    default:
        return RGBLIGHT_MODE_STATIC_LIGHT;
    }
}

void rgb_next_theme(void) {
#ifdef RGB_THEMES
    if (rgb_mode == 100) {
        // If in theme mode, cycle through the actual custom themes
        active_theme = (active_theme + 1) % theme_count;
        if (active_theme == 0) {
            // Once we wrap around themes, go to the next general mode
            rgb_mode = get_next_mode(100);
        }
    } else {
        rgb_mode = get_next_mode(rgb_mode);
    }
#else
    rgb_mode = get_next_mode(rgb_mode);
#endif
    rgb_enabled = true;
}

void rgb_prev_theme(void) {
#ifdef RGB_THEMES
    if (rgb_mode == 100) {
        if (active_theme == 0) {
            // Transition back to previous general mode
            rgb_mode = get_prev_mode(100);
        } else {
            active_theme--;
        }
    } else {
        uint8_t prev = get_prev_mode(rgb_mode);
        if (prev == 100) {
            active_theme = theme_count - 1;
        }
        rgb_mode = prev;
    }
#else
    rgb_mode = get_prev_mode(rgb_mode);
#endif
    rgb_enabled = true;
}

// --- Getters and setters for Vial ---
void rgb_set_enabled(bool enabled) {
    rgb_enabled = enabled;
    if (!enabled) {
        rgb_mode = 0;
    } else if (rgb_mode == 0) {
#ifdef RGB_THEMES
        rgb_mode = 100;
#else
        rgb_mode = RGBLIGHT_MODE_STATIC_LIGHT;
#endif
    }
}

bool rgb_get_enabled(void) {
    return rgb_enabled;
}

void rgb_set_mode(uint8_t mode) {
    if (mode == 0) {
        rgb_enabled = false;
        rgb_mode = 0;
    } else {
        rgb_enabled = true;
        rgb_mode = mode;
    }
}

uint8_t rgb_get_mode(void) {
    return rgb_enabled ? rgb_mode : 0;
}

void rgb_set_brightness(uint8_t brightness) {
    if (brightness > RGB_LIMIT_VAL) {
        rgb_brightness = RGB_LIMIT_VAL;
    } else {
        rgb_brightness = brightness;
    }
}

uint8_t rgb_get_brightness(void) {
    return rgb_brightness;
}

void rgb_set_speed(uint8_t speed) {
    rgb_speed = speed;
}

uint8_t rgb_get_speed(void) {
    return rgb_speed;
}

void rgb_set_color(uint8_t hue, uint8_t sat) {
    rgb_hue = hue;
    rgb_sat = sat;
    rgb_mode = RGBLIGHT_MODE_STATIC_LIGHT; // Override active mode to Solid color
}

uint8_t rgb_get_hue(void) {
    return rgb_hue;
}

uint8_t rgb_get_sat(void) {
    return rgb_sat;
}

void rgb_increase_hue(void) {
    rgb_hue = (rgb_hue + 8) & 0xFF;
}

void rgb_decrease_hue(void) {
    rgb_hue = (rgb_hue - 8) & 0xFF;
}

void rgb_increase_sat(void) {
    rgb_sat = (rgb_sat > 255 - 17) ? 255 : (rgb_sat + 17);
}

void rgb_decrease_sat(void) {
    rgb_sat = (rgb_sat < 17) ? 0 : (rgb_sat - 17);
}

void rgb_increase_val(void) {
    uint8_t limit = RGB_LIMIT_VAL;
    rgb_brightness = (rgb_brightness > limit - 17) ? limit : (rgb_brightness + 17);
}

void rgb_decrease_val(void) {
    rgb_brightness = (rgb_brightness < 17) ? 0 : (rgb_brightness - 17);
}

void rgb_increase_speed(void) {
    rgb_speed = (rgb_speed > 255 - 16) ? 255 : (rgb_speed + 16);
}

void rgb_decrease_speed(void) {
    rgb_speed = (rgb_speed < 16) ? 0 : (rgb_speed - 16);
}

void rgb_get_config(uint8_t *dest) {
    dest[0] = rgb_enabled ? 1 : 0;
    dest[1] = rgb_mode;
    dest[2] = rgb_brightness;
    dest[3] = rgb_speed;
    dest[4] = rgb_hue;
    dest[5] = rgb_sat;
#ifdef RGB_THEMES
    dest[6] = active_theme;
#else
    dest[6] = 0;
#endif
    dest[7] = 0; // padding
}

void rgb_set_config(const uint8_t *src) {
    rgb_enabled = (src[0] != 0);
    rgb_mode = src[1];
    rgb_set_brightness(src[2]);
    rgb_speed = src[3];
    rgb_hue = src[4];
    rgb_sat = src[5];
#ifdef RGB_THEMES
    active_theme = src[6];
#endif
}

#else

// Stubs for unsupported/non-LED platforms
void rgb_init(void) {}
void rgb_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void rgb_toggle(void) {}
void rgb_next_theme(void) {}
void rgb_prev_theme(void) {}

void rgb_set_enabled(bool enabled) {}
bool rgb_get_enabled(void) {
    return false;
}
void rgb_set_mode(uint8_t mode) {}
uint8_t rgb_get_mode(void) {
    return 0;
}
void rgb_set_brightness(uint8_t brightness) {}
uint8_t rgb_get_brightness(void) {
    return 0;
}
void rgb_set_speed(uint8_t speed) {}
uint8_t rgb_get_speed(void) {
    return 0;
}
void rgb_set_color(uint8_t hue, uint8_t sat) {}
uint8_t rgb_get_hue(void) {
    return 0;
}
uint8_t rgb_get_sat(void) {
    return 0;
}
void rgb_increase_hue(void) {}
void rgb_decrease_hue(void) {}
void rgb_increase_sat(void) {}
void rgb_decrease_sat(void) {}
void rgb_increase_val(void) {}
void rgb_decrease_val(void) {}
void rgb_increase_speed(void) {}
void rgb_decrease_speed(void) {}
void rgb_get_config(uint8_t *dest) {
    memset(dest, 0, 8);
}
void rgb_set_config(const uint8_t *src) {}

#endif // defined(RGB_NUM) && (defined(MCU_rp2040) || defined(MCU_rp2350) || defined(MCU_nrf52840) ||
       // defined(MCU_milandr))

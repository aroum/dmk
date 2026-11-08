#ifndef MAGNETENO_LEDS_H
#define MAGNETENO_LEDS_H

#include <stdbool.h>
#include <stdint.h>

#define MAGNETENO_NUM_LEDS 10

typedef enum {
    UI_MODE_PIANO = 0,
    UI_MODE_SEQ,
    UI_MODE_MIXER,
    UI_MODE_LOOPER,
    UI_MODE_MENU_MAIN,
    UI_MODE_MENU_OSC,
    UI_MODE_MENU_VCF,
    UI_MODE_MENU_EG,
    UI_MODE_MENU_LFO,
    UI_MODE_MENU_ADV,
    UI_MODE_MENU_PRESETS,
    UI_MODE_MENU_FX
} MagnetenoUIMode;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize WS2812 RGB LED driver on GPIO12 via PIO/DMA.
 */
void magneteno_leds_init(void);

/**
 * @brief Set RGB color of a specific LED (0..9).
 */
void magneteno_leds_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set all 10 LEDs to a single color.
 */
void magneteno_leds_set_all(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Flush RGB buffer to WS2812 hardware via DMA.
 */
void magneteno_leds_show(void);

/**
 * @brief Update all 10 RGB LEDs according to the current UI mode, active notes,
 * sequencer state, aftertouch depths, and menu selections.
 * @param mode Current active UI mode.
 */
void magneteno_leds_update(MagnetenoUIMode mode);

#ifdef __cplusplus
}
#endif

#endif // MAGNETENO_LEDS_H

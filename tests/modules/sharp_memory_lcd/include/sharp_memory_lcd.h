#ifndef SHARP_MEMORY_LCD_H
#define SHARP_MEMORY_LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "u8g2.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display Dimensions for Sharp LS011B7DH03
#define SHARP_LCD_WIDTH 160
#define SHARP_LCD_HEIGHT 68

typedef struct {
    uint8_t active_layer;
    uint16_t current_wpm;
    uint16_t last_keycode;
    bool caps_lock;
    bool num_lock;
    bool usb_connected;
    bool rgb_enabled;
    uint8_t rgb_mode;
    bool dirty;
} sharp_lcd_state_t;

void sharp_memory_lcd_init(void);
void sharp_memory_lcd_render(void);
uint16_t wpm_get_current(void);

#ifdef __cplusplus
}
#endif

#endif // SHARP_MEMORY_LCD_H

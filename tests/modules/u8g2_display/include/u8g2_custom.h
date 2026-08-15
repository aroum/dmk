#ifndef U8G2_CUSTOM_H
#define U8G2_CUSTOM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Display geometry
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

typedef struct {
    uint8_t current_layer;
    bool caps_lock;
    bool dirty;
} oled_display_state_t;

void u8g2_display_init(void);
void u8g2_display_update(const oled_display_state_t *state);

#ifdef __cplusplus
}
#endif

#endif // U8G2_CUSTOM_H

#ifndef MAGNETENO_DISPLAY_H
#define MAGNETENO_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "../leds/magneteno_leds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Sharp Memory LCD LS011B7DH03 (160x68) on SPI1.
 */
void magneteno_display_init(void);

/**
 * @brief Render current instrument state (Status, 10-key travel, Sequencer/Oscilloscope)
 * and transfer frame buffer to Sharp Memory LCD.
 * Should be called at 25-30 Hz from UI task.
 */
void magneteno_display_update(MagnetenoUIMode mode);

#ifdef __cplusplus
}
#endif

#endif // MAGNETENO_DISPLAY_H

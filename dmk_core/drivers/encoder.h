#ifndef ENCODER_H
#define ENCODER_H

#include "FreeRTOS.h"
#include "config.h"
#include <stdbool.h>
#include <stdint.h>

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)

void encoder_init(void);
void encoder_scan(void);
void encoder_process_event(uint8_t encoder_idx, bool direction);
TickType_t encoder_check_timeouts(TickType_t now);

#else

static inline void encoder_init(void) {}
static inline void encoder_scan(void) {}
static inline void encoder_process_event(uint8_t encoder_idx, bool direction) {
    (void)encoder_idx;
    (void)direction;
}
static inline TickType_t encoder_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}

#endif

#endif // ENCODER_H

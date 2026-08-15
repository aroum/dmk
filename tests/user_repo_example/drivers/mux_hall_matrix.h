#ifndef MUX_HALL_MATRIX_H
#define MUX_HALL_MATRIX_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Default Hall ADC trigger thresholds (12-bit ADC 0..4095)
#ifndef HALL_ACTUATION_THRESHOLD
#define HALL_ACTUATION_THRESHOLD 2600
#endif

#ifndef HALL_RELEASE_THRESHOLD
#define HALL_RELEASE_THRESHOLD 2300
#endif

#ifndef HALL_POLL_INTERVAL_MS
#define HALL_POLL_INTERVAL_MS 1
#endif

void mux_hall_matrix_init(void);
void mux_hall_matrix_scan(void);

#ifdef __cplusplus
}
#endif

#endif // MUX_HALL_MATRIX_H

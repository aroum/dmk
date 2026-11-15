#ifndef CLASSIC_MATRIX_H
#define CLASSIC_MATRIX_H

#include "config.h"
#include "ec_settings.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint16_t matrix_row_t;

#define HHKB_SCAN_TIMER_TICKS_PER_US 4U

#ifdef __cplusplus
extern "C" {
#endif

void hhkb_matrix_hardware_init(void);
bool hhkb_matrix_scan_raw(matrix_row_t current_matrix[]);
#ifdef HHKB_SCAN_DIAGNOSTICS
void hhkb_get_scan_diagnostics(uint32_t *scan_us, uint32_t *scan_max_us, uint32_t *work_max_us, uint32_t *missed, uint32_t *q_full);
void hhkb_reset_scan_diagnostics_peaks(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // CLASSIC_MATRIX_H

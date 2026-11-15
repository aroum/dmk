#ifndef CAPYBARA_MATRIX_H
#define CAPYBARA_MATRIX_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint16_t matrix_row_t;

#ifdef __cplusplus
extern "C" {
#endif

void capybara_matrix_hardware_init(void);
bool capybara_matrix_scan_raw(matrix_row_t current_matrix[]);

#ifdef __cplusplus
}
#endif

#endif // CAPYBARA_MATRIX_H

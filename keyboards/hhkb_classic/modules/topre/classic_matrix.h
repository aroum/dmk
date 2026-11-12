#ifndef CLASSIC_MATRIX_H
#define CLASSIC_MATRIX_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint16_t matrix_row_t;

#define HHKB_SCAN_TIMER_TICKS_PER_US 4U

#ifdef __cplusplus
extern "C" {
#endif

void hhkb_matrix_hardware_init(void);
bool hhkb_matrix_scan_raw(matrix_row_t current_matrix[]);

// Calibration and Actuation Control
void hhkb_calibration_load(void);
void hhkb_calibration_start(void);
bool hhkb_calibration_enabled(void);
bool hhkb_settings_storage_writable(void);
void hhkb_calibration_set_enabled(bool enabled);
uint8_t hhkb_get_led_mode(uint8_t index);
void hhkb_set_led_mode(uint8_t index, uint8_t mode);
uint8_t hhkb_get_system(void);
void hhkb_set_system(uint8_t system);
void hhkb_settings_mark_dirty(void);
uint8_t hhkb_get_scan_rate(void);
void hhkb_set_scan_rate(uint8_t rate);
bool hhkb_get_oversampling(void);
void hhkb_set_oversampling(bool enabled);
uint8_t hhkb_get_hold_tap_setting(uint8_t id);
void hhkb_set_hold_tap_setting(uint8_t id, uint8_t value);
void hhkb_calibration_finish(void);
void hhkb_calibration_save_to_eeprom(void);
void hhkb_set_actuation_point_pct(uint8_t pct);
uint8_t hhkb_get_actuation_point_pct(void);
bool hhkb_is_calibrating(void);
uint8_t hhkb_rt_get(uint8_t id);
void hhkb_rt_set(uint8_t id, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // CLASSIC_MATRIX_H

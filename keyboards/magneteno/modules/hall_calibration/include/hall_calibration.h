#ifndef HALL_CALIBRATION_H
#define HALL_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HE_MAX_KEYS 32
#define HE_CALIB_MAGIC 0x48454143 // 'HEAC' (Hall Effect Analog Calibration)
#define HE_DEFAULT_REST_ADC 2000
#define HE_DEFAULT_BOTTOM_ADC 3800
#define HE_DEFAULT_ACTUATION_PCT 30 // 30% of total travel (~1.2mm on 4mm switch)
#define HE_DEFAULT_RAPID_TRIGGER 80 // Raw ADC delta for rapid trigger (~0.15mm)

#pragma pack(push, 1)
typedef struct {
    uint16_t rest_adc;          // Resting Top ADC deadzone
    uint16_t bottom_adc;        // Maximum bottom-out ADC
    uint8_t actuation_percent;  // Initial actuation point (5%..95%)
    uint8_t rapid_trigger_down; // Delta ADC required for repeat actuation
    uint8_t rapid_trigger_up;   // Delta ADC required for release
    uint8_t continuous_mode;    // 1 = Rapid Trigger enabled, 0 = Static threshold
} hall_key_calib_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t num_keys;
    hall_key_calib_t keys[HE_MAX_KEYS];
    uint32_t checksum;
} hall_calibration_storage_t;
#pragma pack(pop)

typedef struct {
    uint16_t current_adc;
    uint16_t min_seen_adc;
    uint16_t max_seen_adc;
    bool is_pressed;
} hall_key_runtime_t;

void hall_calibration_init(uint8_t key_count);
void hall_calibration_load(void);
void hall_calibration_save(void);
void hall_calibration_set_defaults(void);

// Customization API
void hall_calibration_set_actuation(uint8_t key_idx, uint8_t percent);
void hall_calibration_set_rapid_trigger(uint8_t key_idx, uint8_t down_raw, uint8_t up_raw);
void hall_calibration_set_endpoints(uint8_t key_idx, uint16_t rest_val, uint16_t bottom_val);
const hall_key_calib_t *hall_calibration_get_key(uint8_t key_idx);

// Live processing engine (returns true if switch is considered active)
bool hall_process_sample(uint8_t key_idx, uint16_t raw_adc);

// Learning / Auto-calibration mode
void hall_calibration_enter_learn_mode(void);
void hall_calibration_exit_learn_mode(void);
bool hall_calibration_is_learning(void);

#ifdef __cplusplus
}
#endif

#endif // HALL_CALIBRATION_H

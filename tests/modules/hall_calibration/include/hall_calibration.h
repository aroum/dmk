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
    uint16_t rest_adc;           // Resting Top ADC deadzone
    uint16_t bottom_adc;         // Maximum bottom-out ADC
    uint8_t  actuation_percent;  // Initial actuation point (5%..95%)
    uint8_t  rapid_trigger_down; // Delta ADC required for repeat actuation
    uint8_t  rapid_trigger_up;   // Delta ADC required for release
    uint8_t  continuous_mode;    // 1 = Rapid Trigger enabled, 0 = Static threshold
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
    bool     is_pressed;
} hall_key_runtime_t;

void hall_calibration_init(uint8_t key_count);
void hall_calibration_load(void);
void hall_calibration_save(void);
void hall_calibration_set_defaults(void);

// Customization API
void hall_calibration_set_actuation(uint8_t key_idx, uint8_t percent);
void hall_calibration_set_rapid_trigger(uint8_t key_idx, uint8_t down_raw, uint8_t up_raw);
void hall_calibration_set_endpoints(uint8_t key_idx, uint16_t rest_val, uint16_t bottom_val);
void hall_calibration_set_continuous_mode(uint8_t key_idx, bool enabled);
void hall_calibration_set_global_actuation(uint8_t percent);
void hall_calibration_set_global_rapid_trigger(uint8_t down_raw, uint8_t up_raw);
void hall_calibration_set_global_continuous_mode(bool enabled);
const hall_key_calib_t *hall_calibration_get_key(uint8_t key_idx);

// Live processing engine (returns true if switch is considered active)
bool hall_process_sample(uint8_t key_idx, uint16_t raw_adc);

// Learning / Auto-calibration mode
void hall_calibration_enter_learn_mode(void);
void hall_calibration_exit_learn_mode(void);
bool hall_calibration_is_learning(void);

// VIA v3 Custom UI Value IDs (channel 0)
enum via_hall_value_id {
    ID_HE_CONTINUOUS_MODE    = 1,  // uint8_t: 1 = Continuous RT, 0 = Static
    ID_HE_ACTUATION_PERCENT  = 2,  // uint8_t: 5..95 %
    ID_HE_RT_DOWN            = 3,  // uint8_t: 10..250 ADC delta
    ID_HE_RT_UP              = 4,  // uint8_t: 10..250 ADC delta
    ID_HE_LEARN_MODE         = 5,  // uint8_t: 1 = Learning active, 0 = Normal
    ID_HE_SAVE_FLASH         = 6,  // Button: 1 = Save to Flash
    ID_HE_RESET_DEFAULTS     = 7,  // Button: 1 = Reset to Defaults
    ID_HE_PER_KEY_ACTUATION  = 10, // Array: [key_idx] -> percent
    ID_HE_PER_KEY_RT_DOWN    = 11, // Array: [key_idx] -> down_raw
    ID_HE_PER_KEY_RT_UP      = 12, // Array: [key_idx] -> up_raw
    ID_HE_PER_KEY_CONTINUOUS = 13, // Array: [key_idx] -> 1/0
};

// Hook for VIA custom commands
bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response);

#ifdef __cplusplus
}
#endif

#endif // HALL_CALIBRATION_H

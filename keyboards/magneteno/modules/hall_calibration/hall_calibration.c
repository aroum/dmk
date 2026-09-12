#include "hall_calibration.h"
#include "FreeRTOS.h"
#include "hooks.h"
#include "task.h"

#include <string.h>

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/flash.h"
#include "hardware/sync.h"

// Reserve second to last sector on Flash for HE Calibration storage
#define HE_FLASH_STORAGE_OFFSET (PICO_FLASH_SIZE_BYTES - (2 * FLASH_SECTOR_SIZE))
#endif

static hall_calibration_storage_t storage;
static hall_key_runtime_t runtime_state[HE_MAX_KEYS];
static uint8_t configured_key_count = 10;
static bool is_learning_active = false;

/**
 * @brief Initialize calibration structure with safe default values
 */
void hall_calibration_set_defaults(void) {
    memset(&storage, 0, sizeof(storage));
    storage.magic = HE_CALIB_MAGIC;
    storage.version = 1;
    storage.num_keys = configured_key_count;

    for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
        storage.keys[i].rest_adc = HE_DEFAULT_REST_ADC;
        storage.keys[i].bottom_adc = HE_DEFAULT_BOTTOM_ADC;
        storage.keys[i].actuation_percent = HE_DEFAULT_ACTUATION_PCT;
        storage.keys[i].rapid_trigger_down = HE_DEFAULT_RAPID_TRIGGER;
        storage.keys[i].rapid_trigger_up = HE_DEFAULT_RAPID_TRIGGER;
        storage.keys[i].continuous_mode = 1; // Continuous Rapid Trigger ON by default

        runtime_state[i].current_adc = HE_DEFAULT_REST_ADC;
        runtime_state[i].min_seen_adc = HE_DEFAULT_REST_ADC;
        runtime_state[i].max_seen_adc = HE_DEFAULT_REST_ADC;
        runtime_state[i].is_pressed = false;
    }
}

/**
 * @brief Load calibration data from Flash or set defaults if invalid
 */
void hall_calibration_load(void) {
#if defined(MCU_rp2040) || defined(MCU_rp2350)
    const hall_calibration_storage_t *flash_ptr =
        (const hall_calibration_storage_t *)(XIP_BASE + HE_FLASH_STORAGE_OFFSET);

    if (flash_ptr->magic == HE_CALIB_MAGIC && flash_ptr->version == 1) {
        memcpy(&storage, flash_ptr, sizeof(storage));
        return;
    }
#endif
    hall_calibration_set_defaults();
}

/**
 * @brief Save current calibration and actuation points to non-volatile Flash memory
 */
void hall_calibration_save(void) {
#if defined(MCU_rp2040) || defined(MCU_rp2350)
    static uint8_t sector_buf[FLASH_SECTOR_SIZE];
    memset(sector_buf, 0xFF, FLASH_SECTOR_SIZE);
    memcpy(sector_buf, &storage, sizeof(storage));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(HE_FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(HE_FLASH_STORAGE_OFFSET, sector_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
#endif
}

/**
 * @brief Initialize Hall calibration subsystem
 */
void hall_calibration_init(uint8_t key_count) {
    configured_key_count = (key_count <= HE_MAX_KEYS) ? key_count : HE_MAX_KEYS;
    hall_calibration_load();
}

void hall_calibration_set_actuation(uint8_t key_idx, uint8_t percent) {
    if (key_idx < HE_MAX_KEYS) {
        storage.keys[key_idx].actuation_percent = (percent > 95) ? 95 : (percent < 5 ? 5 : percent);
    }
}

void hall_calibration_set_rapid_trigger(uint8_t key_idx, uint8_t down_raw, uint8_t up_raw) {
    if (key_idx < HE_MAX_KEYS) {
        storage.keys[key_idx].rapid_trigger_down = down_raw;
        storage.keys[key_idx].rapid_trigger_up = up_raw;
    }
}

void hall_calibration_set_endpoints(uint8_t key_idx, uint16_t rest_val, uint16_t bottom_val) {
    if (key_idx < HE_MAX_KEYS && bottom_val > (rest_val + 200)) {
        storage.keys[key_idx].rest_adc = rest_val;
        storage.keys[key_idx].bottom_adc = bottom_val;
    }
}

const hall_key_calib_t *hall_calibration_get_key(uint8_t key_idx) {
    if (key_idx < HE_MAX_KEYS) {
        return &storage.keys[key_idx];
    }
    return &storage.keys[0];
}

void hall_calibration_enter_learn_mode(void) {
    is_learning_active = true;
}

void hall_calibration_exit_learn_mode(void) {
    if (is_learning_active) {
        is_learning_active = false;
        hall_calibration_save();
    }
}

bool hall_calibration_is_learning(void) {
    return is_learning_active;
}

/**
 * @brief Continuous Rapid Trigger & Dynamic Actuation DSP engine
 * @param key_idx Key index (0..HE_MAX_KEYS-1)
 * @param raw_adc Instantaneous 12-bit ADC reading
 * @return True if key is currently considered pressed, False otherwise
 */
bool hall_process_sample(uint8_t key_idx, uint16_t raw_adc) {
    if (key_idx >= HE_MAX_KEYS) {
        return false;
    }

    hall_key_calib_t *cal = &storage.keys[key_idx];
    hall_key_runtime_t *rt = &runtime_state[key_idx];

    // In auto-learn mode, adaptively expand endpoint boundaries
    if (is_learning_active) {
        if (raw_adc < cal->rest_adc) {
            cal->rest_adc = raw_adc;
        }
        if (raw_adc > cal->bottom_adc) {
            cal->bottom_adc = raw_adc;
        }
    }

    rt->current_adc = raw_adc;

    // Calculate dynamic initial actuation threshold
    uint32_t travel_range = (cal->bottom_adc > cal->rest_adc) ? (cal->bottom_adc - cal->rest_adc) : 1000;
    uint16_t actuation_threshold = cal->rest_adc + (uint16_t)((travel_range * cal->actuation_percent) / 100);
    uint16_t rest_deadzone = cal->rest_adc + 40; // Prevent noise trigger near top

    if (!rt->is_pressed) {
        // Track local minimum while switch is ascending/resting
        if (raw_adc < rt->min_seen_adc) {
            rt->min_seen_adc = raw_adc;
        }

        // Trigger press if crossed initial actuation point or down-stroke delta exceeded
        if (raw_adc >= actuation_threshold) {
            rt->is_pressed = true;
            rt->max_seen_adc = raw_adc;
        }
    } else {
        // Switch is currently active: track local peak maximum while descending
        if (raw_adc > rt->max_seen_adc) {
            rt->max_seen_adc = raw_adc;
        }

        if (cal->continuous_mode) {
            // Rapid Trigger: Instant release upon detecting upward stroke of RT_UP threshold
            if (raw_adc <= rest_deadzone ||
                (rt->max_seen_adc > cal->rapid_trigger_up && raw_adc <= (rt->max_seen_adc - cal->rapid_trigger_up))) {
                rt->is_pressed = false;
                rt->min_seen_adc = raw_adc;
            }
        } else {
            // Standard static hysteresis release
            if (raw_adc <= (actuation_threshold - 150)) {
                rt->is_pressed = false;
            }
        }
    }

    return rt->is_pressed;
}

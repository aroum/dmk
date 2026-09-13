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

static uint16_t learn_min_adc[HE_MAX_KEYS];
static uint16_t learn_max_adc[HE_MAX_KEYS];
static bool key_learned[HE_MAX_KEYS];

void hall_calibration_enter_learn_mode(void) {
    for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
        learn_min_adc[i] = 0xFFFF;
        learn_max_adc[i] = 0;
        key_learned[i] = false;
    }
    is_learning_active = true;
}

void hall_calibration_exit_learn_mode(void) {
    if (is_learning_active) {
        is_learning_active = false;
        for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
            // Apply learned range if key was exercised with at least 100 ADC counts of travel
            if (key_learned[i] && (learn_max_adc[i] > (learn_min_adc[i] + 100))) {
                storage.keys[i].rest_adc = learn_min_adc[i];
                storage.keys[i].bottom_adc = learn_max_adc[i];
            }
        }
        hall_calibration_save();
    }
}

bool hall_calibration_is_learning(void) {
    return is_learning_active;
}

void hall_calibration_set_continuous_mode(uint8_t key_idx, bool enabled) {
    if (key_idx < HE_MAX_KEYS) {
        storage.keys[key_idx].continuous_mode = enabled ? 1 : 0;
    }
}

void hall_calibration_set_global_actuation(uint8_t percent) {
    uint8_t clamped = (percent > 95) ? 95 : (percent < 5 ? 5 : percent);
    for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
        storage.keys[i].actuation_percent = clamped;
    }
}

void hall_calibration_set_global_rapid_trigger(uint8_t down_raw, uint8_t up_raw) {
    for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
        storage.keys[i].rapid_trigger_down = down_raw;
        storage.keys[i].rapid_trigger_up = up_raw;
    }
}

void hall_calibration_set_global_continuous_mode(bool enabled) {
    for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
        storage.keys[i].continuous_mode = enabled ? 1 : 0;
    }
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

    // In auto-learn mode, record per-key min and max ADC excursions
    if (is_learning_active) {
        if (raw_adc < learn_min_adc[key_idx]) {
            learn_min_adc[key_idx] = raw_adc;
        }
        if (raw_adc > learn_max_adc[key_idx]) {
            learn_max_adc[key_idx] = raw_adc;
        }
        key_learned[key_idx] = true;
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

bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response) {
    uint8_t command_id = request[0];
    uint8_t channel_id = request[1];

    if (channel_id != 0) {
        return false;
    }

    switch (command_id) {
    case 0x08: { // VIA_CUSTOM_GET_VALUE
        uint8_t value_id = request[2];
        switch (value_id) {
        case ID_HE_CONTINUOUS_MODE:
            response[3] = storage.keys[0].continuous_mode;
            return true;
        case ID_HE_ACTUATION_PERCENT:
            response[3] = storage.keys[0].actuation_percent;
            return true;
        case ID_HE_RT_DOWN:
            response[3] = storage.keys[0].rapid_trigger_down;
            return true;
        case ID_HE_RT_UP:
            response[3] = storage.keys[0].rapid_trigger_up;
            return true;
        case ID_HE_LEARN_MODE:
            response[3] = is_learning_active ? 1 : 0;
            return true;
        case ID_HE_PER_KEY_ACTUATION: {
            uint8_t key_idx = request[3];
            if (key_idx < HE_MAX_KEYS) {
                response[4] = storage.keys[key_idx].actuation_percent;
            }
            return true;
        }
        case ID_HE_PER_KEY_RT_DOWN: {
            uint8_t key_idx = request[3];
            if (key_idx < HE_MAX_KEYS) {
                response[4] = storage.keys[key_idx].rapid_trigger_down;
            }
            return true;
        }
        case ID_HE_PER_KEY_RT_UP: {
            uint8_t key_idx = request[3];
            if (key_idx < HE_MAX_KEYS) {
                response[4] = storage.keys[key_idx].rapid_trigger_up;
            }
            return true;
        }
        case ID_HE_PER_KEY_CONTINUOUS: {
            uint8_t key_idx = request[3];
            if (key_idx < HE_MAX_KEYS) {
                response[4] = storage.keys[key_idx].continuous_mode;
            }
            return true;
        }
        default:
            return false;
        }
    }

    case 0x07: { // VIA_CUSTOM_SET_VALUE
        uint8_t value_id = request[2];
        switch (value_id) {
        case ID_HE_CONTINUOUS_MODE:
            hall_calibration_set_global_continuous_mode(request[3] != 0);
            return true;
        case ID_HE_ACTUATION_PERCENT:
            hall_calibration_set_global_actuation(request[3]);
            return true;
        case ID_HE_RT_DOWN:
            for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
                storage.keys[i].rapid_trigger_down = request[3];
            }
            return true;
        case ID_HE_RT_UP:
            for (uint8_t i = 0; i < HE_MAX_KEYS; ++i) {
                storage.keys[i].rapid_trigger_up = request[3];
            }
            return true;
        case ID_HE_LEARN_MODE:
            if (request[3]) {
                hall_calibration_enter_learn_mode();
            } else {
                hall_calibration_exit_learn_mode();
            }
            return true;
        case ID_HE_SAVE_FLASH:
            hall_calibration_save();
            return true;
        case ID_HE_RESET_DEFAULTS:
            hall_calibration_set_defaults();
            hall_calibration_save();
            return true;
        case ID_HE_PER_KEY_ACTUATION: {
            uint8_t key_idx = request[3];
            uint8_t val = request[4];
            hall_calibration_set_actuation(key_idx, val);
            return true;
        }
        case ID_HE_PER_KEY_RT_DOWN: {
            uint8_t key_idx = request[3];
            uint8_t val = request[4];
            if (key_idx < HE_MAX_KEYS) {
                storage.keys[key_idx].rapid_trigger_down = val;
            }
            return true;
        }
        case ID_HE_PER_KEY_RT_UP: {
            uint8_t key_idx = request[3];
            uint8_t val = request[4];
            if (key_idx < HE_MAX_KEYS) {
                storage.keys[key_idx].rapid_trigger_up = val;
            }
            return true;
        }
        case ID_HE_PER_KEY_CONTINUOUS: {
            uint8_t key_idx = request[3];
            uint8_t val = request[4];
            hall_calibration_set_continuous_mode(key_idx, val != 0);
            return true;
        }
        default:
            return false;
        }
    }

    case 0x09: { // VIA_CUSTOM_SAVE
        hall_calibration_save();
        return true;
    }

    default:
        return false;
    }
}


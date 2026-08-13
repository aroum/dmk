#include "encoder.h"
#include "config.h"
#include "hal_gpio.h"
#include "keyboard.h"
#include "keys.h"
#include "matrix.h"
#include "usb.h"

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)

static const pin_t encoder_pins_a[] = ENCODER_PINS_A;
static const pin_t encoder_pins_b[] = ENCODER_PINS_B;
#define NUM_ENCODERS (sizeof(encoder_pins_a) / sizeof(encoder_pins_a[0]))

// Gray code 2-bit transition state decoder table (quadrature decoding)
static const int8_t encoder_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

static uint8_t prev_states[NUM_ENCODERS];
static int8_t positions[NUM_ENCODERS];
static int8_t resolutions[NUM_ENCODERS];

// Keypress timers and active keys for automatic synthetic key release
#define ENCODER_TICK_DURATION_MS 20
static uint32_t active_encoder_keys[NUM_ENCODERS] = {0};
static int16_t encoder_release_timers[NUM_ENCODERS] = {0};

extern uint8_t layer;
extern void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);

/**
 * @brief Initialize encoder GPIO inputs with internal pull-ups and configure resolution dividers.
 */
void encoder_init(void) {
// 1. Initialize resolutions (pulses per detent)
#if defined(ENCODER_RESOLUTIONS)
    int8_t res_init[] = ENCODER_RESOLUTIONS;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        resolutions[i] = res_init[i];
    }
#elif defined(ENCODER_DETENTS) && defined(ENCODER_PULSES)
    int8_t detents[] = ENCODER_DETENTS;
    int8_t pulses[] = ENCODER_PULSES;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        resolutions[i] = detents[i] / pulses[i];
        if (resolutions[i] <= 0)
            resolutions[i] = 1;
    }
#elif defined(ENCODER_RESOLUTION)
    for (int i = 0; i < NUM_ENCODERS; i++) {
        resolutions[i] = ENCODER_RESOLUTION;
    }
#else
    for (int i = 0; i < NUM_ENCODERS; i++) {
        resolutions[i] = 4; // Standard 4 quadrature transitions per physical detent
    }
#endif

    // 2. Initialize GPIOs with pull-ups
    for (int i = 0; i < NUM_ENCODERS; i++) {
        hal_gpio_init(encoder_pins_a[i]);
        hal_gpio_set_dir(encoder_pins_a[i], false);
        hal_gpio_pull_up(encoder_pins_a[i]);

        hal_gpio_init(encoder_pins_b[i]);
        hal_gpio_set_dir(encoder_pins_b[i], false);
        hal_gpio_pull_up(encoder_pins_b[i]);

        prev_states[i] = (hal_gpio_get(encoder_pins_a[i]) << 1) | hal_gpio_get(encoder_pins_b[i]);
        positions[i] = 0;
        active_encoder_keys[i] = 0;
        encoder_release_timers[i] = 0;
    }
}

/**
 * @brief Poll quadrature encoder pins, compute Gray code transitions, and enqueue rotation events.
 */
void encoder_scan(void) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        uint8_t current_state = (hal_gpio_get(encoder_pins_a[i]) << 1) | hal_gpio_get(encoder_pins_b[i]);
        if (current_state != prev_states[i]) {
            uint8_t state_idx = (prev_states[i] << 2) | current_state;
            int8_t movement = encoder_states[state_idx & 0x0F];
            prev_states[i] = current_state;

            if (movement != 0) {
                positions[i] += movement;
                int8_t res = resolutions[i];
                if (positions[i] >= res) {
                    positions[i] -= res;

                    matrix_event_t event;
                    event.split = 0;
                    event.row = 0xFF; // Special row marker for rotary encoder events
                    event.col = i;
                    event.pressed = 0; // Clockwise (CW)
                    xQueueSend(matrix_queue, &event, 0);
                } else if (positions[i] <= -res) {
                    positions[i] += res;

                    matrix_event_t event;
                    event.split = 0;
                    event.row = 0xFF;
                    event.col = i;
                    event.pressed = 1; // Counter-Clockwise (CCW)
                    xQueueSend(matrix_queue, &event, 0);
                }
            }
        }
    }
}

/**
 * @brief Get static keycode mapped to encoder on a specific layer and direction.
 */
uint32_t encoder_keymap_get_static(uint8_t lyr, uint8_t encoder_idx, uint8_t direction) {
    extern const uint32_t encoder_keymap[][4][2];
    extern const size_t keymap_layers;
    if (lyr < keymap_layers && encoder_idx < NUM_ENCODERS) {
        return encoder_keymap[lyr][encoder_idx][direction];
    }
    return 0;
}

/**
 * @brief Get total number of hardware rotary encoders configured.
 */
uint8_t encoder_get_count(void) {
    return NUM_ENCODERS;
}

/**
 * @brief Dispatch encoder rotation event, resolving target keycode from active layer and starting pulse timer.
 * @param encoder_idx Encoder index (0..NUM_ENCODERS-1).
 * @param direction False for CW, True for CCW.
 */
void encoder_process_event(uint8_t encoder_idx, bool direction) {
    if (encoder_idx >= NUM_ENCODERS)
        return;

    // If key is already active, release it immediately first to handle fast continuous spinning
    if (active_encoder_keys[encoder_idx] != 0) {
        process_key_event(0xFF, 0xFF, active_encoder_keys[encoder_idx], false);
        active_encoder_keys[encoder_idx] = 0;
        encoder_release_timers[encoder_idx] = 0;
    }

    uint32_t key = 0;
#ifdef VIAL
#include "vial.h"
    extern uint32_t dynamic_encoder_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][4][2];
    for (int l = layer; l >= 0; l--) {
        if (encoder_idx < 4) {
            key = dynamic_encoder_keymap[l][encoder_idx][direction ? 1 : 0];
            if (key != 0) {
                break;
            }
        }
    }
#else
    for (int l = layer; l >= 0; l--) {
        key = encoder_keymap[l][encoder_idx][direction ? 1 : 0];
        if (key != 0) {
            break;
        }
    }
#endif

    if (key != 0) {
        active_encoder_keys[encoder_idx] = key;
        encoder_release_timers[encoder_idx] = ENCODER_TICK_DURATION_MS;
        process_key_event(0xFF, 0xFF, key, true);
    }
}

/**
 * @brief Update encoder synthetic key press release timers and emit release events when expired.
 * @param delta_ms Elapsed milliseconds since last check.
 */
void encoder_update_timers(uint32_t delta_ms) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (encoder_release_timers[i] > 0) {
            encoder_release_timers[i] -= delta_ms;
            if (encoder_release_timers[i] <= 0) {
                encoder_release_timers[i] = 0;
                if (active_encoder_keys[i] != 0) {
                    process_key_event(0xFF, 0xFF, active_encoder_keys[i], false);
                    active_encoder_keys[i] = 0;
                }
            }
        }
    }
}

#else
void encoder_process_event(uint8_t encoder_idx, bool direction) {
    (void)encoder_idx;
    (void)direction;
}
#endif

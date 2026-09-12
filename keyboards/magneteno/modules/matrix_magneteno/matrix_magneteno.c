#include "FreeRTOS.h"
#include "config.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hall_calibration.h"
#include "matrix.h"
#include "queue.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

extern QueueHandle_t matrix_queue;

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "mux_ctrl.pio.h"

static PIO s_mux_pio = pio0;
static uint s_mux_sm = 0;
static bool s_use_pio_mux = false;

/**
 * @brief Convert 3-bit channel (0..7) to PIO output word.
 * NOTE: On current PCB revision, S0, S1, S2 are mapped in reverse order:
 * GPIO 13 = S2 (bit 2)
 * GPIO 14 = S1 (bit 1)
 * GPIO 15 = S0 (bit 0)
 * Base pin for PIO is GPIO 13.
 * PIO 'out pins, 3' writes:
 *   bit 0 -> GPIO 13 (S2)
 *   bit 1 -> GPIO 14 (S1)
 *   bit 2 -> GPIO 15 (S0)
 * Thus, we reverse the 3 bits: bit 0 <-> bit 2.
 * In next PCB revision with normal order, this reversal can simply be removed!
 */
static inline uint32_t channel_to_pio_word(uint8_t channel) {
    uint32_t bit0 = (channel & 0x01) ? 1 : 0; // S0
    uint32_t bit1 = (channel & 0x02) ? 1 : 0; // S1
    uint32_t bit2 = (channel & 0x04) ? 1 : 0; // S2

    // Reverse order for current revision (bit0 -> pos 2, bit2 -> pos 0):
    return (bit0 << 2) | (bit1 << 1) | (bit2 << 0);
}
#endif

// Debounced switch states for the 10 keys
static bool key_states[NUM_KEYS] = {false};

/**
 * @brief Switch SN74LV4051A 8:1 multiplexer to a given channel (0..7)
 * Uses hardware PIO state machine with automatic 50us settle delay on RP2040/RP2350.
 * @param channel 0-indexed channel (0..7)
 */
static inline void sn74lv4051a_select_channel(uint8_t channel) {
#if defined(MCU_rp2040) || defined(MCU_rp2350)
    if (s_use_pio_mux) {
        // Clear previous IRQ flag if any
        pio_interrupt_clear(s_mux_pio, 0);

        // Push reversed 3-bit channel pattern to PIO
        pio_sm_put_blocking(s_mux_pio, s_mux_sm, channel_to_pio_word(channel));

        // Wait until PIO finishes 50us hardware settle delay and sets IRQ 0
        while (!pio_interrupt_get(s_mux_pio, 0)) {
            tight_loop_contents();
        }
        pio_interrupt_clear(s_mux_pio, 0);
        return;
    }
#endif

    hal_gpio_put(MUX_PIN_S0, (channel & 0x01) != 0);
    hal_gpio_put(MUX_PIN_S1, (channel & 0x02) != 0);
    hal_gpio_put(MUX_PIN_S2, (channel & 0x04) != 0);

#if defined(MUX_SETTLE_US)
    hal_sleep_us(MUX_SETTLE_US);
#endif
}

/**
 * @brief Read ADC with a dummy flush read and 4x oversampling average.
 * Flushes the internal sample-and-hold charge before averaging to avoid crosstalk.
 */
static inline uint16_t adc_read_averaged(pin_t pin) {
    (void)hal_adc_read(pin); // Dummy conversion to flush previous S&H charge
    uint32_t acc = 0;
    for (int i = 0; i < 4; i++) {
        acc += hal_adc_read(pin);
    }
    return (uint16_t)(acc / 4);
}

/**
 * @brief Initialize ADC, PIO and GPIO peripherals for Hall effect scanning
 */
static void magneteno_matrix_init(void) {
#if defined(MCU_rp2040) || defined(MCU_rp2350)
    // 1. Attempt to initialize hardware PIO for MUX control
    // Base pin is GPIO 13 (covering GPIO 13, 14, 15)
    uint base_pin = 13;
    s_mux_pio = pio0;
    int sm = pio_claim_unused_sm(s_mux_pio, false);
    if (sm < 0) {
        s_mux_pio = pio1;
        sm = pio_claim_unused_sm(s_mux_pio, false);
    }

    if (sm >= 0 && pio_can_add_program(s_mux_pio, &mux_ctrl_program)) {
        s_mux_sm = (uint)sm;
        uint offset = pio_add_program(s_mux_pio, &mux_ctrl_program);
        mux_ctrl_program_init(s_mux_pio, s_mux_sm, offset, base_pin);

        // Configure 12mA drive strength on MUX pins for sharp CMOS edges
        gpio_disable_pulls(13);
        gpio_disable_pulls(14);
        gpio_disable_pulls(15);
        gpio_set_drive_strength(13, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_drive_strength(14, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_drive_strength(15, GPIO_DRIVE_STRENGTH_12MA);

        s_use_pio_mux = true;
    } else {
        // Fallback to standard GPIO driving
        hal_gpio_init(MUX_PIN_S0);
        hal_gpio_set_dir(MUX_PIN_S0, true);
        hal_gpio_put(MUX_PIN_S0, 0);

        hal_gpio_init(MUX_PIN_S1);
        hal_gpio_set_dir(MUX_PIN_S1, true);
        hal_gpio_put(MUX_PIN_S1, 0);

        hal_gpio_init(MUX_PIN_S2);
        hal_gpio_set_dir(MUX_PIN_S2, true);
        hal_gpio_put(MUX_PIN_S2, 0);

        gpio_disable_pulls(MUX_PIN_S0);
        gpio_disable_pulls(MUX_PIN_S1);
        gpio_disable_pulls(MUX_PIN_S2);
        gpio_set_drive_strength(MUX_PIN_S0, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_drive_strength(MUX_PIN_S1, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_drive_strength(MUX_PIN_S2, GPIO_DRIVE_STRENGTH_12MA);
    }
#else
    // Fallback for non-RP platforms
    hal_gpio_init(MUX_PIN_S0);
    hal_gpio_set_dir(MUX_PIN_S0, true);
    hal_gpio_put(MUX_PIN_S0, 0);

    hal_gpio_init(MUX_PIN_S1);
    hal_gpio_set_dir(MUX_PIN_S1, true);
    hal_gpio_put(MUX_PIN_S1, 0);

    hal_gpio_init(MUX_PIN_S2);
    hal_gpio_set_dir(MUX_PIN_S2, true);
    hal_gpio_put(MUX_PIN_S2, 0);
#endif

    // 2. Initialize ADC inputs via universal HAL ADC
    hal_adc_init(MUX_ADC_PIN);
    hal_adc_init(DIRECT_KEY9_PIN);
    hal_adc_init(DIRECT_KEY10_PIN);

    // 3. Initialize Hall Calibration and Rapid Trigger engine
    hall_calibration_init(NUM_KEYS);
}

/**
 * @brief Process single key press/release event using Rapid Trigger calibration engine
 */
static inline void update_key_state(uint8_t key_idx, uint16_t adc_val) {
    bool current = key_states[key_idx];
    bool next = hall_process_sample(key_idx, adc_val);

    if (next != current) {
        key_states[key_idx] = next;
        matrix_event_t event = {.split = 0, .row = 0, .col = key_idx, .pressed = next ? 1 : 0};
        xQueueSend(matrix_queue, &event, 0);
    }
}

/**
 * @brief Scan all 10 Hall sensors (8 via MUX + 2 direct)
 */
static void magneteno_matrix_scan(void) {
    // 1. Scan 8 keys connected via SN74LV4051A Multiplexer -> MUX_ADC_PIN
    // MUX channel mapping:
    // Channel 0 -> Key 1 (idx 1), Channel 1 -> Key 3 (idx 3), Channel 2 -> Key 2 (idx 2),
    // Channel 3 -> Key 0 (idx 0), Channel 4 -> Key 6 (idx 6), Channel 5 -> Key 5 (idx 5),
    // Channel 6 -> Key 7 (idx 7), Channel 7 -> Key 4 (idx 4)
    static const uint8_t mux_to_he_map[8] = MUX_CHANNEL_MAP;
    for (uint8_t ch = 0; ch < 8; ++ch) {
        sn74lv4051a_select_channel(ch);
        uint16_t val = adc_read_averaged(MUX_ADC_PIN);
        uint8_t key_idx = mux_to_he_map[ch];
        update_key_state(key_idx, val);
    }

    // 2. Scan Key 9 directly on DIRECT_KEY9_PIN (ADC2 / GPIO28)
#if defined(DIRECT_ADC_SETTLE_US)
    hal_sleep_us(DIRECT_ADC_SETTLE_US);
#endif
    uint16_t val_key9 = adc_read_averaged(DIRECT_KEY9_PIN);
    update_key_state(8, val_key9);

    // 3. Scan Key 10 directly on DIRECT_KEY10_PIN (ADC1 / GPIO27)
#if defined(DIRECT_ADC_SETTLE_US)
    hal_sleep_us(DIRECT_ADC_SETTLE_US);
#endif
    uint16_t val_key10 = adc_read_averaged(DIRECT_KEY10_PIN);
    update_key_state(9, val_key10);
}

/**
 * @brief FreeRTOS task handling periodic matrix scanning
 */
void matrix_task(void *pvParameters) {
    (void)pvParameters;
    magneteno_matrix_init();

    while (1) {
        magneteno_matrix_scan();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1000Hz polling rate
    }
}

/**
 * @brief Matrix state lookup
 */
bool matrix_is_pressed(unsigned char row, unsigned char col) {
    (void)row;
    if (col < NUM_KEYS) {
        return key_states[col];
    }
    return false;
}

// split for Raspberry Pi (RP2040/RP2350)
#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/structs/usb.h"
#include "pico/stdlib.h"
#include "queue.h"
#include "split.pio.h"
#include "task.h"
#include "task_internal.h"
#include "tusb.h"

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

#ifndef SPLIT_CONNECTION_TYPE
#define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX
#endif

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX)
#if (SPLIT_TX_PIN < 0 || SPLIT_TX_PIN > 47)
#error "DMK RP2040/RP2350 Error: Invalid SPLIT_TX_PIN! Pin must be a valid GPIO (GPIO0..GPIO29)."
#endif
#elif (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)
#if (SPLIT_TX_PIN == SPLIT_RX_PIN)
#error                                                                                                                 \
    "DMK RP2040/RP2350 HW UART Error: HW_FULL_DUPLEX requires separate SPLIT_TX_PIN and SPLIT_RX_PIN! For 1-wire PIO, use SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX."
#endif
#endif

bool is_master(void) {
    return tud_mounted();
}

#if (SPLIT_CONNECTION_TYPE == SOFT)

void split_send_event(matrix_event_t *event) {
    split_soft_send_event(event);
}

void split_init(void) {
    split_soft_init();
}

#else // PIO Hardware Half-Duplex Mode (HW_HALF_DUPLEX / HW_FULL_DUPLEX)

static PIO split_pio = pio0;
static uint split_sm = 0;
static uint tx_program_offset;
static uint rx_program_offset;
static int current_role = -1; // -1 = unconfigured, 0 = slave (TX), 1 = master (RX)

static void configure_split_role(bool master) {
    if (current_role == (master ? 1 : 0)) {
        return; // Already configured
    }

    uint pin = SPLIT_TX_PIN;

    // Disable state machine before reconfiguring
    pio_sm_set_enabled(split_pio, split_sm, false);

    if (master) {
        pio_sm_config c = split_rx_program_get_default_config(rx_program_offset);

        // Input pin and jump pin
        sm_config_set_in_pins(&c, pin);
        sm_config_set_jmp_pin(&c, pin);

        // Setup shifting: shift right, no autopush, 32-bit shift
        sm_config_set_in_shift(&c, true, false, 32);

        // Join FIFOs to make a deeper RX FIFO
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);

        float div = (float)clock_get_hz(clk_sys) / (8.0f * SPLIT_UART_BAUD);
        sm_config_set_clkdiv(&c, div);

        pio_sm_init(split_pio, split_sm, rx_program_offset, &c);

        // Set pin direction to input
        pio_sm_set_consecutive_pindirs(split_pio, split_sm, pin, 1, false);

        pio_sm_set_enabled(split_pio, split_sm, true);
    } else {
        pio_sm_config c = split_tx_program_get_default_config(tx_program_offset);

        // Out pin and side-set pin
        sm_config_set_out_pins(&c, pin, 1);
        sm_config_set_sideset_pins(&c, pin);

        // Setup shifting: shift right, no autopull, 32-bit shift
        sm_config_set_out_shift(&c, true, false, 32);

        // Join FIFOs to make a deeper TX FIFO
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

        float div = (float)clock_get_hz(clk_sys) / (8.0f * SPLIT_UART_BAUD);
        sm_config_set_clkdiv(&c, div);

        pio_sm_init(split_pio, split_sm, tx_program_offset, &c);

        // Set initial pin direction to input (high-impedance)
        pio_sm_set_consecutive_pindirs(split_pio, split_sm, pin, 1, false);

        pio_sm_set_enabled(split_pio, split_sm, true);
    }

    current_role = master ? 1 : 0;
}

void split_send_event(matrix_event_t *event) {
    split_packet_t pkt;
    pkt.header = 0xA5;
    pkt.row = event->row;
    pkt.col = event->col;
    pkt.pressed = event->pressed;

    uint8_t *pkt_ptr = (uint8_t *)&pkt;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        // Data is inverted because open-drain drives low for 0 and floats high for 1
        pio_sm_put_blocking(split_pio, split_sm, (uint32_t)~pkt_ptr[i]);
    }
}

void split_task(void *pvParameters) {
    (void)pvParameters;

    while (1) {
        bool master = is_master();
        if (master) {
            if (current_role != 1) {
                configure_split_role(true);
            }

            if (!pio_sm_is_rx_fifo_empty(split_pio, split_sm)) {
                uint8_t byte = (pio_sm_get(split_pio, split_sm) >> 24) & 0xFF;
                split_process_received_byte(byte);
            } else {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        } else {
            if (current_role != 0) {
                configure_split_role(false);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void split_init(void) {
    uint pin = SPLIT_TX_PIN;

    gpio_init(pin);
    gpio_pull_up(pin);

    // Load programs and claim the state machine once
    tx_program_offset = pio_add_program(split_pio, &split_tx_program);
    rx_program_offset = pio_add_program(split_pio, &split_rx_program);
    split_sm = pio_claim_unused_sm(split_pio, true);

    pio_gpio_init(split_pio, pin);

    // Start with Slave (TX) role by default
    configure_split_role(false);

    xTaskCreate(split_task, "split", 1024, NULL, TASK_PRIO_DEF, NULL);
}

#endif // SOFT / PIO

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

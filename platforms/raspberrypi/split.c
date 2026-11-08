// split for Raspberry Pi (RP2040/RP2350)
#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/structs/usb.h"
#include "pico/stdlib.h"
#include "queue.h"
#include "semphr.h"
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

#if (SPLIT_CONNECTION_TYPE != SOFT) // PIO Hardware Half-Duplex Mode (HW_HALF_DUPLEX / HW_FULL_DUPLEX)

static PIO split_pio = pio0;
static uint split_sm = 0;
static uint tx_program_offset;
static uint rx_program_offset;
static int current_role = -1; // -1 = unconfigured, 0 = transmit, 1 = receive
static StaticSemaphore_t s_pio_mutex_storage;
static SemaphoreHandle_t s_pio_mutex;

static void configure_split_role(bool master) {
    if (current_role == (master ? 1 : 0)) {
        return; // Already configured
    }

    uint pin = master ? SPLIT_RX_PIN : SPLIT_TX_PIN;
    current_role = -1;

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

void split_transport_send_packet(const split_packet_t *packet) {
    xSemaphoreTake(s_pio_mutex, portMAX_DELAY);
    configure_split_role(false);
    const uint8_t *pkt_ptr = (const uint8_t *)packet;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        pio_sm_put_blocking(split_pio, split_sm, (uint32_t)~pkt_ptr[i]);
    }
    // FIFO empty means the final byte has been pulled, not yet shifted out.
    while (!pio_sm_is_tx_fifo_empty(split_pio, split_sm)) vTaskDelay(1);
    sleep_us(20 * BIT_TIME_US);
    configure_split_role(true);
    xSemaphoreGive(s_pio_mutex);
    if (!is_master()) vTaskDelay(pdMS_TO_TICKS(12));
}

void split_transport_reply_packet(const split_packet_t *packet) {
    split_transport_send_packet(packet);
}

static bool split_pio_receive_one(uint8_t *byte) {
    bool received = false;
    xSemaphoreTake(s_pio_mutex, portMAX_DELAY);
    if (current_role == 1 && !pio_sm_is_rx_fifo_empty(split_pio, split_sm)) {
        *byte = (pio_sm_get(split_pio, split_sm) >> 24) & 0xFF;
        received = true;
    }
    xSemaphoreGive(s_pio_mutex);
    return received;
}

#ifdef DMK_SPLIT_TEST
bool split_test_pio_receive_one(uint8_t *byte) { return split_pio_receive_one(byte); }
#endif

void split_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        uint8_t byte;
        // The parser can transmit a reply and therefore must run outside the mutex.
        if (split_pio_receive_one(&byte)) split_process_received_byte(byte);
        else vTaskDelay(pdMS_TO_TICKS(1));
    }
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)
static StaticTask_t s_split_task_tcb;
static StackType_t s_split_task_stack[256];
#endif

void split_init(void) {
    s_pio_mutex = xSemaphoreCreateMutexStatic(&s_pio_mutex_storage);
    configASSERT(s_pio_mutex != NULL);
    uint pin = SPLIT_TX_PIN;

    gpio_init(pin);
    gpio_pull_up(pin);

    // Load programs and claim the state machine once
    tx_program_offset = pio_add_program(split_pio, &split_tx_program);
    rx_program_offset = pio_add_program(split_pio, &split_rx_program);
    split_sm = pio_claim_unused_sm(split_pio, true);

    pio_gpio_init(split_pio, pin);
    if (SPLIT_RX_PIN != pin) {
        gpio_init(SPLIT_RX_PIN);
        gpio_pull_up(SPLIT_RX_PIN);
        pio_gpio_init(split_pio, SPLIT_RX_PIN);
    }

    // Both halves listen except while transmitting their assigned slot.
    configure_split_role(true);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    xTaskCreateStatic(split_task, "split", 256, NULL, TASK_PRIO_DEF, s_split_task_stack, &s_split_task_tcb);
#else
    xTaskCreate(split_task, "split", 256, NULL, TASK_PRIO_DEF, NULL);
#endif
}

#endif // SPLIT_CONNECTION_TYPE != SOFT

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

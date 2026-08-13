#include "config.h"

#ifdef MIDI_JACK
#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef PIN_MIDI_JACK_OUT
#define PIN_MIDI_JACK_OUT 0
#endif
#ifndef PIN_MIDI_JACK_IN
#define PIN_MIDI_JACK_IN 1
#endif

static uart_inst_t *midi_uart;

void hal_midi_jack_hw_init(void) {
    int tx_pin = PIN_MIDI_JACK_OUT;
    if (tx_pin == 0 || tx_pin == 12 || tx_pin == 16 || tx_pin == 1 || tx_pin == 13 || tx_pin == 17) {
        midi_uart = uart0;
    } else {
        midi_uart = uart1;
    }

    uart_init(midi_uart, 31250);
    gpio_set_function(PIN_MIDI_JACK_OUT, GPIO_FUNC_UART);
    gpio_set_function(PIN_MIDI_JACK_IN, GPIO_FUNC_UART);

    uart_set_hw_flow(midi_uart, false, false);
    uart_set_format(midi_uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(midi_uart, true);
}

bool hal_midi_jack_readable(void) {
    return uart_is_readable(midi_uart);
}

uint8_t hal_midi_jack_getc(void) {
    return uart_getc(midi_uart);
}

void hal_midi_jack_putc(uint8_t byte) {
    uart_putc(midi_uart, byte);
}

#endif // MIDI_JACK

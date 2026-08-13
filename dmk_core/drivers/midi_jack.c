#include "config.h"

#ifdef MIDI_JACK
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

// Platform low-level hardware UART prototypes
extern void hal_midi_jack_hw_init(void);
extern bool hal_midi_jack_readable(void);
extern uint8_t hal_midi_jack_getc(void);
extern void hal_midi_jack_putc(uint8_t byte);

static void midi_rx_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        if (hal_midi_jack_readable()) {
            uint8_t byte = hal_midi_jack_getc();
#ifdef MIDI_THRU
            hal_midi_jack_putc(byte);
#endif
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void midi_jack_init(void) {
    hal_midi_jack_hw_init();
    xTaskCreate(midi_rx_task, "midi_rx", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void midi_jack_write(uint8_t byte) {
    hal_midi_jack_putc(byte);
}

#endif // MIDI_JACK

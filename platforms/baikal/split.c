#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "queue.h"
#include "task.h"
#include "task_internal.h"
#include "tusb.h"

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

bool is_master(void) {
    return tud_mounted();
}

void split_send_event(matrix_event_t *event) {
    split_soft_send_event(event);
}

void split_init(void) {
    split_soft_init();
}

#endif

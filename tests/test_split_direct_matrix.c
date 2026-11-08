#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "matrix.h"
#include "split.h"

TickType_t g_mock_ticks;
QueueHandle_t matrix_queue = (void *)1;
static uint32_t gpio_levels = UINT32_MAX;
static matrix_event_t last_event;
static unsigned event_count;
void hal_gpio_init(pin_t pin) { (void)pin; }
void hal_gpio_set_dir(pin_t pin, bool output) { (void)pin; (void)output; }
void hal_gpio_put(pin_t pin, bool value) { (void)pin; (void)value; }
bool hal_gpio_get(pin_t pin) { return (gpio_levels & (1u << pin)) != 0; }
void hal_gpio_pull_up(pin_t pin) { (void)pin; }
void hal_gpio_pull_down(pin_t pin) { (void)pin; }
void hal_sleep_us(uint32_t us) { (void)us; }
hal_gpio_snapshot_t hal_gpio_snapshot(void) { return (hal_gpio_snapshot_t){gpio_levels, 0, 0}; }
bool hal_gpio_snapshot_get(hal_gpio_snapshot_t snapshot, pin_t pin) { return (snapshot.p0 & (1u << pin)) != 0; }
bool is_master(void) { return true; }
void split_send_event(matrix_event_t *event) { (void)event; assert(false); }
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait) {
    (void)wait; assert(queue == matrix_queue); last_event = *(const matrix_event_t *)item; ++event_count; return pdTRUE;
}
void matrix_init(void);
void matrix_scan(void);
#ifdef TEST_COL2ROW
#define TEST_ROW 1
#define TEST_COL 0
#else
#define TEST_ROW 0
#define TEST_COL 1
#endif

int main(void) {
    matrix_init();
    matrix_scan();
    assert(!matrix_is_pressed(TEST_ROW, TEST_COL) && !matrix_is_pressed(1, 1));
    gpio_levels &= ~(1u << 1);
    matrix_scan();
    assert(event_count == 1 && last_event.row == TEST_ROW && last_event.col == TEST_COL && last_event.pressed);
    assert(matrix_is_pressed(TEST_ROW, TEST_COL));
    assert(!matrix_is_pressed(1, 1)); // split snapshots must not invent another row
    gpio_levels |= 1u << 1;
    for (unsigned i = 0; i < 7; ++i) matrix_scan();
    assert(event_count == 2 && !last_event.pressed && !matrix_is_pressed(TEST_ROW, TEST_COL));
    puts("split matrix debounce and snapshot tests passed");
    return 0;
}

#include "FreeRTOS.h"
#include "task.h"
#include "bounded_heap.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

extern char _heap_start;
extern char _heap_end;

// newlib's nosys _sbrk starts at _end (after the IRQ stack), outside SRAM.
// Keep libc allocations inside the dedicated linker-reserved region instead.
void *_sbrk(ptrdiff_t increment) {
    static uintptr_t cursor;
    uintptr_t previous = 0;
    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (running) taskENTER_CRITICAL();
    if (!cursor) cursor = (uintptr_t)&_heap_start;
    bool ok = dmk_bounded_heap_adjust((uintptr_t)&_heap_start, (uintptr_t)&_heap_end, &cursor, increment, &previous);
    if (running) taskEXIT_CRITICAL();
    if (!ok) {
        errno = ENOMEM;
        return (void *)-1;
    }
    return (void *)previous;
}

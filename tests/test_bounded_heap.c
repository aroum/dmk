#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "bounded_heap.h"

int main(void) {
    uintptr_t cursor = 100, previous = 0;
    assert(dmk_bounded_heap_adjust(100, 200, &cursor, 50, &previous));
    assert(previous == 100 && cursor == 150);
    assert(!dmk_bounded_heap_adjust(100, 200, &cursor, 51, &previous) && cursor == 150);
    assert(dmk_bounded_heap_adjust(100, 200, &cursor, 50, &previous) && cursor == 200);
    assert(!dmk_bounded_heap_adjust(100, 200, &cursor, 1, &previous) && cursor == 200);
    assert(dmk_bounded_heap_adjust(100, 200, &cursor, -100, &previous) && cursor == 100);
    assert(!dmk_bounded_heap_adjust(100, 200, &cursor, PTRDIFF_MIN, &previous) && cursor == 100);
    assert(!dmk_bounded_heap_adjust(200, 100, &cursor, 0, &previous));
    puts("bounded libc heap tests passed");
    return 0;
}

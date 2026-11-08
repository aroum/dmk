#ifndef DMK_BOUNDED_HEAP_H
#define DMK_BOUNDED_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Adjust a break within [start, end], without signed overflow or wraparound.
static inline bool dmk_bounded_heap_adjust(uintptr_t start, uintptr_t end, uintptr_t *cursor,
                                           ptrdiff_t increment, uintptr_t *previous) {
    if (start > end || *cursor < start || *cursor > end) return false;
    uintptr_t available;
    uintptr_t amount;
    if (increment >= 0) {
        amount = (uintptr_t)increment;
        available = end - *cursor;
        if (amount > available) return false;
        *previous = *cursor;
        *cursor += amount;
    } else {
        // Avoid negating PTRDIFF_MIN.
        amount = (uintptr_t)(-(increment + 1)) + 1u;
        available = *cursor - start;
        if (amount > available) return false;
        *previous = *cursor;
        *cursor -= amount;
    }
    return true;
}
#endif

#ifndef HHKB_SETTINGS_STORAGE_GUARD_H
#define HHKB_SETTINGS_STORAGE_GUARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline size_t hhkb_settings_owned_size(uint32_t magic, size_t current_size,
                                               size_t old_a_size, size_t old_b_size,
                                               size_t old_c_size) {
    switch (magic) {
    case 0: return 0;
    case 0x484b4341U: return old_a_size;
    case 0x484b4342U: return old_b_size;
    case 0x484b4343U: return old_c_size;
    case 0x484b4344U:
    case 0x484b4345U: return current_size;
    default: return SIZE_MAX;
    }
}

/* On migration, all bytes newly occupied by the current format must be zero. */
static inline bool hhkb_settings_region_writable(volatile const uint8_t *bytes,
                                                 size_t current_size,
                                                 size_t old_a_size,
                                                 size_t old_b_size,
                                                 size_t old_c_size) {
    uint32_t magic = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
                     ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
    size_t owned = hhkb_settings_owned_size(magic, current_size, old_a_size, old_b_size, old_c_size);
    if (owned > current_size) return false;
    for (size_t i = owned; i < current_size; i++) {
        if (bytes[i] != 0) return false;
    }
    return true;
}
#endif

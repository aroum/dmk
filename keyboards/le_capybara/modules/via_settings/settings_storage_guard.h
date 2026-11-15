#ifndef HHKB_SETTINGS_STORAGE_GUARD_H
#define HHKB_SETTINGS_STORAGE_GUARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A recognised record owns only its original length. On migration, all bytes
 * newly occupied by the current format must still be zero. */
static inline bool hhkb_settings_region_writable(volatile const uint8_t *bytes,
                                                 size_t current_size,
                                                 size_t old_a_size,
                                                 size_t old_b_size,
                                                 size_t old_c_size) {
    uint32_t magic = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
                     ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
    size_t owned;
    switch (magic) {
    case 0: owned = 0; break;
    case 0x484b4341U: owned = old_a_size; break;
    case 0x484b4342U: owned = old_b_size; break;
    case 0x484b4343U: owned = old_c_size; break;
    case 0x484b4344U: owned = current_size; break;
    default: return false;
    }
    if (owned > current_size) return false;
    for (size_t i = owned; i < current_size; i++) {
        if (bytes[i] != 0) return false;
    }
    return true;
}
#endif

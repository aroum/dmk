#ifndef DMK_HOLD_TAP_KEYCODE_H
#define DMK_HOLD_TAP_KEYCODE_H
#include <stdbool.h>
#include <stdint.h>
#include "keys.h"

static inline bool ht_decode_action(unsigned kind, uint8_t value, uint32_t *action) {
    switch (kind) {
    case 0: *action = value; return true;
    case 1:
        if (value < 16) { *action = MO(value); return true; }
        if (value < 32) { *action = TG(value & 15); return true; }
        return false;
    case 2: *action = MK(value, K_NULL); return true;
    default: return false;
    }
}
static inline bool ht_decode(uint32_t key, uint32_t *hold, uint32_t *tap, uint8_t *timeout) {
    *timeout = (key >> 16) & 0xFF;
    if ((key & 0xF0000000u) == DMK_HT)
        return ht_decode_action((key >> 26) & 3, (key >> 8) & 0xFF, hold) &&
               ht_decode_action((key >> 24) & 3, key & 0xFF, tap);
    uint32_t type = key & 0xFF000000u;
    if (type != DMK_HT_LEGACY && type != DMK_HT_MOD_LEGACY) return false;
    uint8_t target = (key >> 8) & 0xFF;
    *tap = key & 0xFF;
    if (type == DMK_HT_MOD_LEGACY) *hold = MK(target, K_NULL);
    else if (target < 16) *hold = MO(target);
    else if (target >= 0xE0 && target <= 0xE7) *hold = target;
    else *hold = MK(target, K_NULL);
    return true;
}
#endif

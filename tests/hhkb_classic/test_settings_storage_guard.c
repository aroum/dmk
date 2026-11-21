#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "settings_storage_guard.h"

static void set_magic(uint8_t *p, uint32_t value) {
    p[0] = value;
    p[1] = value >> 8;
    p[2] = value >> 16;
    p[3] = value >> 24;
}
static bool writable(uint8_t *bytes) {
    return hhkb_settings_region_writable(bytes, 0x484b4345, 264, 248, 260, 264);
}
int main(void) {
    uint8_t bytes[264] = {0};
    assert(writable(bytes));
    bytes[220] = 1;
    assert(!writable(bytes));
    memset(bytes, 0, sizeof bytes);
    set_magic(bytes, 0x12345678);
    assert(!writable(bytes));
    set_magic(bytes, 0x484b4341);
    bytes[100] = 3;
    assert(writable(bytes));
    bytes[260] = 1;
    assert(!writable(bytes));
    bytes[260] = 0;
    set_magic(bytes, 0x484b4342);
    bytes[263] = 1;
    assert(!writable(bytes));
    bytes[263] = 0;
    set_magic(bytes, 0x484b4343);
    bytes[263] = 1;
    assert(writable(bytes));
    set_magic(bytes, 0x484b4344);
    assert(writable(bytes));
    set_magic(bytes, 0x484b4345);
    assert(writable(bytes));
    set_magic(bytes, 0x484b4a45);
    assert(!writable(bytes)); // ANSI must not own JIS data.
    assert(hhkb_settings_region_writable(bytes, 0x484b4a45, 264, 248, 260, 264));
    assert(hhkb_settings_owned_size(0x484b4a45, 0x484b4a45, 324, 248, 260, 264) == 324);
    set_magic(bytes, 0x484b4345);
    assert(!hhkb_settings_region_writable(bytes, 0x484b4a45, 264, 248, 260, 264));
    set_magic(bytes, 0x484b4341);
    assert(!hhkb_settings_region_writable(bytes, 0x484b4a45, 264, 248, 260, 264));
    return 0;
}

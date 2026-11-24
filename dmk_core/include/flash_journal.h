#ifndef DMK_FLASH_JOURNAL_H
#define DMK_FLASH_JOURNAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Page-programmed, two-bank snapshot journal. A header is committed only after
// its entire payload. The active bank is never erased before its replacement
// commits. Torn/hash-invalid records occupy space but are never loaded.
#define DMK_JOURNAL_PAGE 256u
#define DMK_JOURNAL_MAGIC UINT32_C(0x314a4d44)
typedef struct {
    const uint8_t *bank[2];
    size_t bank_size;
    bool (*erase)(unsigned bank);
    bool (*program)(unsigned bank, size_t offset, const uint8_t *page);
} dmk_journal_t;
typedef struct {
    uint32_t magic, generation, length, hash, inverse_hash, header_hash;
} dmk_journal_header_t;
_Static_assert(sizeof(dmk_journal_header_t) == 24, "Journal header layout changed");
typedef struct {
    const uint8_t *data;
    uint32_t generation;
    unsigned bank;
} dmk_journal_record_t;

static inline uint32_t dmk_journal_hash(const void *data, size_t size) {
    const uint8_t *p = data;
    uint32_t hash = UINT32_C(2166136261);
    while (size--) hash = (hash ^ *p++) * UINT32_C(16777619);
    return hash;
}
static inline size_t dmk_journal_stride(size_t size) {
    return DMK_JOURNAL_PAGE + (size + DMK_JOURNAL_PAGE - 1) / DMK_JOURNAL_PAGE * DMK_JOURNAL_PAGE;
}
static inline dmk_journal_record_t dmk_journal_latest(const dmk_journal_t *j, size_t size) {
    dmk_journal_record_t latest = {0};
    size_t stride = dmk_journal_stride(size);
    for (unsigned bank = 0; bank < 2; ++bank) {
        for (size_t off = 0; off + stride <= j->bank_size; off += stride) {
            dmk_journal_header_t h;
            memcpy(&h, j->bank[bank] + off, sizeof(h));
            const uint8_t *data = j->bank[bank] + off + DMK_JOURNAL_PAGE;
            if (h.magic != DMK_JOURNAL_MAGIC || h.length != size || h.inverse_hash != ~h.hash ||
                h.header_hash != dmk_journal_hash(&h, offsetof(dmk_journal_header_t, header_hash)) ||
                h.hash != dmk_journal_hash(data, size)) continue;
            if (!latest.data || (int32_t)(h.generation - latest.generation) > 0)
                latest = (dmk_journal_record_t){data, h.generation, bank};
        }
    }
    return latest;
}
static inline bool dmk_journal_blank(const uint8_t *p, size_t size) {
    while (size--) if (*p++ != 0xff) return false;
    return true;
}
static inline bool dmk_journal_save(const dmk_journal_t *j, const void *data, size_t size) {
    size_t stride = dmk_journal_stride(size);
    if (!size || stride > j->bank_size) return false;
    dmk_journal_record_t latest = dmk_journal_latest(j, size);
    if (latest.data && memcmp(latest.data, data, size) == 0) return true;
    unsigned bank = latest.data ? latest.bank : 0;
    size_t off;
    for (off = 0; off + stride <= j->bank_size; off += stride)
        if (dmk_journal_blank(j->bank[bank] + off, stride)) break;
    if (off + stride > j->bank_size) {
        bank = latest.data ? latest.bank ^ 1u : bank;
        if (!j->erase(bank)) return false;
        off = 0;
    }
    uint8_t page[DMK_JOURNAL_PAGE];
    for (size_t pos = 0; pos < size; pos += sizeof(page)) {
        size_t n = size - pos < sizeof(page) ? size - pos : sizeof(page);
        memset(page, 0xff, sizeof(page));
        memcpy(page, (const uint8_t *)data + pos, n);
        if (!j->program(bank, off + DMK_JOURNAL_PAGE + pos, page)) return false;
    }
    // Read back before publishing the commit header.
    if (memcmp(j->bank[bank] + off + DMK_JOURNAL_PAGE, data, size)) return false;
    dmk_journal_header_t h = {
        .magic = DMK_JOURNAL_MAGIC, .generation = latest.data ? latest.generation + 1 : 1,
        .length = (uint32_t)size, .hash = dmk_journal_hash(data, size),
    };
    h.inverse_hash = ~h.hash;
    h.header_hash = dmk_journal_hash(&h, offsetof(dmk_journal_header_t, header_hash));
    memset(page, 0xff, sizeof(page));
    memcpy(page, &h, sizeof(h));
    return j->program(bank, off, page) && memcmp(j->bank[bank] + off, page, sizeof(page)) == 0;
}
#endif

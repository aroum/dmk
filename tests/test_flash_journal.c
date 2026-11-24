#include <assert.h>
#include <stdio.h>
#include "flash_journal.h"

static uint8_t flash[2][8192], saved[2][8192], old[4096], next[4096];
static int budget = -1;
static unsigned erases, programs;
static bool spend(void) {
    if (budget < 0) return true;
    if (!budget) return false;
    --budget;
    return true;
}
static bool erase(unsigned bank) {
    ++erases;
    for (unsigned i = 0; i < sizeof(flash[bank]); ++i) {
        if (!spend()) return false;
        flash[bank][i] = 0xff;
    }
    return true;
}
static bool program(unsigned bank, size_t offset, const uint8_t *page) {
    assert(offset % DMK_JOURNAL_PAGE == 0);
    assert(offset + DMK_JOURNAL_PAGE <= sizeof(flash[bank]));
    ++programs;
    for (unsigned i = 0; i < DMK_JOURNAL_PAGE; ++i) {
        if (!spend()) return false;
        assert((flash[bank][offset + i] & page[i]) == page[i]);
        flash[bank][offset + i] &= page[i];
    }
    return true;
}
static const dmk_journal_t journal = {{flash[0], flash[1]}, 8192, erase, program};
static void power_cuts(size_t size, bool rollover) {
    memset(flash, 0xff, sizeof(flash)); budget = -1;
    memset(old, 0x31, sizeof(old)); memset(next, 0x72, sizeof(next));
    size_t records = rollover ? sizeof(flash[0]) / dmk_journal_stride(size) : 1;
    for (unsigned i = 0; i < records; ++i) {
        old[0] = i;
        assert(dmk_journal_save(&journal, old, size));
    }
    memcpy(saved, flash, sizeof(flash));
    size_t writes = (rollover ? 8192 : 0) + dmk_journal_stride(size);
    // Interrupt at every byte of every page, including erase and commit header.
    for (unsigned cut = 0; cut <= writes; ++cut) {
        memcpy(flash, saved, sizeof(flash)); budget = cut;
        (void)dmk_journal_save(&journal, next, size);
        dmk_journal_record_t r = dmk_journal_latest(&journal, size);
        assert(r.data);
        assert(!memcmp(r.data, old, size) || !memcmp(r.data, next, size));
        // Reboot and retry must work even with a half-written/erased record.
        budget = -1;
        assert(dmk_journal_save(&journal, next, size));
        r = dmk_journal_latest(&journal, size);
        assert(r.data && !memcmp(r.data, next, size));
    }
}
int main(void) {
    power_cuts(700, false);
    power_cuts(700, true);
    power_cuts(4096, true);
    budget = -1; erases = programs = 0;
    dmk_journal_record_t r = dmk_journal_latest(&journal, 4096);
    assert(dmk_journal_save(&journal, r.data, 4096));
    assert(!erases && !programs); // identical snapshots never wear flash
    memset(flash, 0xff, sizeof(flash));
    for (unsigned i = 0; i < 100; ++i) {
        next[0] = i;
        assert(dmk_journal_save(&journal, next, 700));
    }
    assert(erases < 15);
    r = dmk_journal_latest(&journal, 700);
    assert(r.generation == 100 && r.data[0] == 99);
    // Corrupted latest payload is rejected; the previous committed record lives.
    ((uint8_t *)r.data)[1] ^= 1;
    r = dmk_journal_latest(&journal, 700);
    assert(r.data && r.data[0] == 98);
    puts("Flash journal power-cut, rollover, unchanged-save and corruption tests passed");
}

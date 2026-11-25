#ifndef HHKB_SCAN_CACHE_H
#define HHKB_SCAN_CACHE_H

#include "rapid_trigger.h"

#define HHKB_SCAN_CACHE_CELLS (MATRIX_COLS * MATRIX_ROWS)

/* Rebuilding one cell runs two 12-step threshold searches from flash; a
 * full-matrix rebuild costs milliseconds and used to run inside a single
 * scan, missing 1 ms deadlines in a burst whenever calibration finished or a
 * VIA slider moved. Spread it over scans instead: cells below rebuild_cursor
 * are rebuilt for the current configuration, the rest take the uncached
 * reference path, whose outputs the cache reproduces bit-for-bit. */
#ifndef HHKB_RT_REBUILD_KEYS_PER_SCAN
#define HHKB_RT_REBUILD_KEYS_PER_SCAN 2
#endif

typedef struct {
    rt_raw_cache_t key[MATRIX_COLS][MATRIX_ROWS];
    uint32_t range_revision, curve_revision;
    uint16_t act_pm, release_pm, reset_pm;
    uint8_t rebuild_cursor; /* cells below this match the current cfg */
    bool valid, fast_active;
} hhkb_scan_cache_t;

static inline bool hhkb_scan_cache_entry_valid(const hhkb_scan_cache_t *cache,
                                               uint8_t col, uint8_t row) {
    return (unsigned)(col * MATRIX_ROWS + row) < cache->rebuild_cursor;
}

// Matrix task owns this cache. Rebuild only when calibration, curve or relevant
// settings change. No cache data is stored in EEPROM or shared with USB writers.
static __attribute__((noinline)) void hhkb_scan_cache_prepare(hhkb_scan_cache_t *cache,
        rt_key_t keys[MATRIX_COLS][MATRIX_ROWS], const rt_scan_cfg_t *scan,
        uint32_t range_revision, uint32_t curve_revision) {
    bool same_ranges = cache->valid && cache->range_revision == range_revision &&
                       cache->curve_revision == curve_revision;
    if (cache->fast_active && scan->cfg.rt_enabled && same_ranges) {
        for (unsigned col = 0; col < MATRIX_COLS; ++col)
            for (unsigned row = 0; row < MATRIX_ROWS; ++row)
                keys[col][row].ext = rt_scan_normalize(&keys[col][row], cache->key[col][row].last_raw, scan);
    }
    bool fast = !scan->cfg.rt_enabled;
    if (fast != cache->fast_active || !same_ranges || cache->act_pm != scan->cfg.act_pm ||
                 cache->release_pm != scan->classic_release || cache->reset_pm != scan->cfg.reset_pm) {
        // Invalidate: every cell falls back to the uncached path until the
        // cursor sweeps past it again. last_raw is reseeded for the whole
        // matrix here, atomically, exactly as the full rebuild did.
        cache->rebuild_cursor = 0;
        if (!same_ranges) {
            for (unsigned col = 0; col < MATRIX_COLS; ++col)
                for (unsigned row = 0; row < MATRIX_ROWS; ++row)
                    cache->key[col][row].last_raw = keys[col][row].base;
        }
    }
    if (cache->rebuild_cursor < HHKB_SCAN_CACHE_CELLS) {
        unsigned budget = HHKB_RT_REBUILD_KEYS_PER_SCAN ? HHKB_RT_REBUILD_KEYS_PER_SCAN
                                                        : HHKB_SCAN_CACHE_CELLS;
        do {
            unsigned idx = cache->rebuild_cursor;
            rt_prepare_raw_cache(&cache->key[idx / MATRIX_ROWS][idx % MATRIX_ROWS],
                                 &keys[idx / MATRIX_ROWS][idx % MATRIX_ROWS], scan);
            cache->rebuild_cursor = (uint8_t)(idx + 1);
        } while (--budget && cache->rebuild_cursor < HHKB_SCAN_CACHE_CELLS);
    }
    cache->range_revision = range_revision;
    cache->curve_revision = curve_revision;
    cache->act_pm = scan->cfg.act_pm;
    cache->release_pm = scan->classic_release;
    cache->reset_pm = scan->cfg.reset_pm;
    cache->valid = true;
    cache->fast_active = fast;
}
#endif

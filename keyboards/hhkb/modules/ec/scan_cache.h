#ifndef HHKB_SCAN_CACHE_H
#define HHKB_SCAN_CACHE_H

#include "rapid_trigger.h"
#ifdef HHKB_SCAN_DIAGNOSTICS
#include "FreeRTOS.h"
#include "task.h"
#endif

typedef struct {
    rt_raw_cache_t key[MATRIX_COLS][MATRIX_ROWS];
    uint32_t range_revision, curve_revision;
    uint16_t act_pm, release_pm, reset_pm;
    bool valid, fast_active;
} hhkb_scan_cache_t;

// Matrix task owns this cache. Rebuild only when calibration, curve or relevant
// settings change. No cache data is stored in EEPROM or shared with USB writers.
static __attribute__((noinline)) void hhkb_scan_cache_prepare(hhkb_scan_cache_t *cache,
        rt_key_t keys[MATRIX_COLS][MATRIX_ROWS], const rt_scan_cfg_t *scan,
        uint32_t range_revision, uint32_t curve_revision) {
    bool same_ranges = cache->valid && cache->range_revision == range_revision &&
                       cache->curve_revision == curve_revision;
    if (cache->fast_active && scan->cfg.rt_enabled && same_ranges) {
        for (unsigned col = 0; col < MATRIX_COLS; ++col)
            for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
                uint16_t v=rt_scan_normalize(&keys[col][row],cache->key[col][row].last_raw,scan);
                uint16_t bottom=scan->curve ? scan->curve[125] : RT_FULL_SCALE;
                keys[col][row].ext=v>bottom ? bottom : v;
            }
    }
    bool fast = !scan->cfg.rt_enabled;
    if (fast != cache->fast_active || !same_ranges || cache->act_pm != scan->cfg.act_pm ||
                 cache->release_pm != scan->classic_release || cache->reset_pm != scan->cfg.reset_pm) {
#ifdef HHKB_SCAN_DIAGNOSTICS
        TickType_t rebuild_start = xTaskGetTickCount();
#endif
        for (unsigned col = 0; col < MATRIX_COLS; ++col) {
            for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
                rt_prepare_raw_cache(&cache->key[col][row], &keys[col][row], scan);
                if (!same_ranges) cache->key[col][row].last_raw = keys[col][row].base;
            }
        }
#ifdef HHKB_SCAN_DIAGNOSTICS
        extern void debug_console_printf(const char *, ...);
        debug_console_printf("[CFG] RT rebuild=%lu ms\r\n",
            (unsigned long)((xTaskGetTickCount() - rebuild_start) * portTICK_PERIOD_MS));
#endif
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

#ifndef RAPID_TRIGGER_H
#define RAPID_TRIGGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Rapid Trigger for analog (EC capacitive) keys.
 *
 * All thresholds are in PERMILLE (1/1000) of the per-key calibrated signal
 * range (baseline = key at rest, peak = key bottomed out). 10 = 1.0 %.
 * The EC signal is not linear in travel, so these are NOT millimetres.
 */

#define RT_FULL_SCALE    1000  /* calibrated range maps to 0..1000 permille */
#define RT_MAX_PM        1100  /* clamp for over-travel                     */
#define RT_MIN_RANGE      150  /* below this the calibration is not trusted */
#define RT_DEFAULT_RANGE 1000  /* fallback range in ADC counts              */

typedef struct {
    bool     rt_enabled;  /* false: fixed actuation point with hysteresis    */
    uint16_t act_pm;      /* initial actuation point (from rest)             */
    uint16_t press_pm;    /* RT: rise from the local minimum that re-presses */
    uint16_t release_pm;  /* RT: drop from the local maximum that releases   */
    uint16_t reset_pm;    /* at/below this the key is up and RT is disarmed  */
} rt_cfg_t;

typedef struct {
    uint16_t base;     /* ADC counts at rest                          */
    uint32_t scale;    /* (1000 << 16) / range, avoids division in scan */
    int16_t  ext;      /* local extreme (max while down, min while up) */
    bool     down;     /* current logical state                        */
    bool     latched;  /* crossed act_pm since last visit to reset zone */
} rt_key_t;

/* Prepared once per scan; classic thresholds are inverted only on settings or
 * calibrated-range changes. Raw cache targets the HHKB 12-bit ADC (0..4095). */
typedef struct {
    rt_cfg_t cfg;
    const uint16_t *curve;
    uint16_t classic_release;
} rt_scan_cfg_t;
typedef struct {
    uint16_t press_raw, release_raw, last_raw; // RT uses release_raw as the first raw above reset.
} rt_raw_cache_t;
extern uint32_t rt_range_revision;
void rt_prepare_scan(rt_scan_cfg_t *scan, const rt_cfg_t *cfg, const uint16_t *curve);
void rt_prepare_raw_cache(rt_raw_cache_t *cache, const rt_key_t *key, const rt_scan_cfg_t *scan);
uint16_t rt_scan_normalize(const rt_key_t *key, uint16_t raw, const rt_scan_cfg_t *scan);
bool rt_update_scan_travel(rt_key_t *key, const rt_scan_cfg_t *scan, uint16_t travel);
bool rt_update_scan_sample(rt_key_t *key, const rt_scan_cfg_t *scan, uint16_t raw, uint16_t *travel);
bool rt_update_scan_cached(rt_key_t *key, const rt_raw_cache_t *cache, const rt_scan_cfg_t *scan, uint16_t raw, uint16_t *travel);

static inline __attribute__((always_inline))
bool rt_update_raw_cached(rt_key_t *key, rt_raw_cache_t *cache, uint16_t raw) {
    cache->last_raw = raw;
    key->down = raw >= (key->down ? cache->release_raw : cache->press_raw);
    key->latched = key->down;
    // ext is unused in classic mode. Restore from last_raw before enabling RT.
    return key->down;
}

void     rt_cfg_defaults(rt_cfg_t *c);
void     rt_key_reset(rt_key_t *k);
void     rt_key_set_range(rt_key_t *k, uint16_t base, uint16_t peak);
uint16_t rt_normalize(const rt_key_t *k, uint16_t raw);

/* Feed one raw ADC sample, returns the logical key state (true = pressed). */
bool     rt_update(rt_key_t *k, const rt_cfg_t *c, uint16_t raw);
bool     rt_update_curve(rt_key_t *k, const rt_cfg_t *c, uint16_t raw, const uint16_t *lut);
uint16_t rt_normalize_curve(const rt_key_t *k, uint16_t raw, const uint16_t *lut);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_TRIGGER_H */

#include "rapid_trigger.h"
#include "ec_curve.h"

uint32_t rt_range_revision;

void rt_cfg_defaults(rt_cfg_t *c) {
    c->rt_enabled = false;
    c->act_pm     = 750;  /* 75 % of range                   */
    c->press_pm   = 30;   /* 3 %                             */
    c->release_pm = 30;   /* 3 %                             */
    c->reset_pm   = 50;   /* 5 % dead zone near the top      */
}

void rt_key_reset(rt_key_t *k) {
    k->ext     = 0;
    k->down    = false;
    k->latched = false;
}

void rt_key_set_range(rt_key_t *k, uint16_t base, uint16_t peak) {
    uint32_t range = (peak > base) ? (uint32_t)(peak - base) : 0;
    if (range < RT_MIN_RANGE) {
        range = RT_DEFAULT_RANGE;
    }
    k->base  = base;
    k->scale = ((uint32_t)RT_FULL_SCALE << 16) / range;
    ++rt_range_revision;
}

static inline __attribute__((always_inline)) uint16_t normalize(const rt_key_t *k, uint16_t raw, const uint16_t *lut) {
    if (raw <= k->base) {
        return 0;
    }
    uint32_t d = (uint32_t)(raw - k->base);
    // Split the Q16 scale so both products fit in 32 bits, even for a full uint16_t sample.
    uint32_t pm = d * (k->scale >> 16) + ((d * (k->scale & UINT16_MAX) + (lut ? 32768u : 0u)) >> 16);
    if (lut) return ec_curve_apply(lut, pm > 1000 ? 1000 : (uint16_t)pm);
    return (pm > RT_MAX_PM) ? (uint16_t)RT_MAX_PM : (uint16_t)pm;
}

uint16_t rt_normalize(const rt_key_t *k, uint16_t raw) {
    return normalize(k, raw, 0);
}

uint16_t rt_normalize_curve(const rt_key_t *k, uint16_t raw, const uint16_t *lut) {
    return normalize(k, raw, lut);
}

void rt_prepare_scan(rt_scan_cfg_t *scan, const rt_cfg_t *cfg, const uint16_t *curve) {
    scan->cfg = *cfg;
    scan->curve = curve;
    uint16_t release = cfg->act_pm > cfg->release_pm ? cfg->act_pm - cfg->release_pm : cfg->reset_pm;
    scan->classic_release = release > cfg->reset_pm ? release : cfg->reset_pm;
}

/* Find the first raw sample whose normalized travel is >= threshold. 4096 is
 * an unreachable sentinel. Binary search preserves every rounding/LUT plateau. */
static uint16_t raw_threshold(const rt_key_t *key, const rt_scan_cfg_t *scan, uint16_t threshold) {
    uint16_t low = 0, high = 4096;
    while (low < high) {
        uint16_t middle = low + (high - low) / 2;
        if (normalize(key, middle, scan->curve) >= threshold) high = middle;
        else low = middle + 1;
    }
    return low;
}
void rt_prepare_raw_cache(rt_raw_cache_t *cache, const rt_key_t *key, const rt_scan_cfg_t *scan) {
    uint16_t press = scan->cfg.act_pm > scan->cfg.reset_pm ? scan->cfg.act_pm : scan->cfg.reset_pm + 1;
    cache->press_raw = raw_threshold(key, scan, press);
    cache->release_raw = raw_threshold(key, scan, (scan->cfg.rt_enabled ? scan->cfg.reset_pm : scan->classic_release) + 1);
}

#ifdef __arm__
#define RT_SCAN_CODE __attribute__((section(".RamFunc"), optimize("O2")))
#else
#define RT_SCAN_CODE
#endif
static inline __attribute__((always_inline)) uint16_t scan_normalize(const rt_key_t *key, uint16_t raw, const rt_scan_cfg_t *scan) {
    uint32_t delta = raw > key->base ? (uint32_t)(raw - key->base) : 0;
    // range>=150 and raw<=4095: the complete Q16 product fits uint32_t.
    uint32_t pm = (delta * key->scale + (scan->curve ? 32768u : 0u)) >> 16;
    if (scan->curve) return ec_curve_apply(scan->curve, pm > 1000 ? 1000 : (uint16_t)pm);
    return pm > RT_MAX_PM ? RT_MAX_PM : (uint16_t)pm;
}

RT_SCAN_CODE uint16_t rt_scan_normalize(const rt_key_t *key, uint16_t raw, const rt_scan_cfg_t *scan) {
    return scan_normalize(key, raw, scan);
}

static inline __attribute__((always_inline))
bool update_travel(rt_key_t *k, const rt_scan_cfg_t *scan, int16_t v) {
    const rt_cfg_t *c = &scan->cfg;
    // Pressure beyond calibrated bottom-out must not deepen the RT release anchor.
    // Keep EEPROM/config layout unchanged: this is a zero-width bottom dead zone.
    uint16_t bottom = scan->curve ? scan->curve[125] : RT_FULL_SCALE;
    if (c->rt_enabled && v > (int16_t)bottom) v = (int16_t)bottom;
    /* Top dead zone: key is definitely up, RT disarmed until act_pm is crossed again. */
    if (v <= (int16_t)c->reset_pm) {
        k->down    = false;
        k->latched = false;
        k->ext     = v;
        return false;
    }

    /* Classic mode: fixed actuation point with release hysteresis. */
    if (!c->rt_enabled) {
        int16_t rel = (int16_t)scan->classic_release;
        if (k->down) {
            if (v <= rel) {
                k->down = false;
            }
        } else if (v >= (int16_t)c->act_pm) {
            k->down = true;
        }
        k->latched = k->down;
        k->ext     = v;
        return k->down;
    }

    /* Rapid Trigger, not armed yet: needs the normal actuation point once. */
    if (!k->latched) {
        if (v >= (int16_t)c->act_pm) {
            k->latched = true;
            k->down    = true;
            k->ext     = v;
            return true;
        }
        k->down = false;
        k->ext  = v;
        return false;
    }

    /* Armed: follow the direction of motion. */
    if (k->down) {
        if (v > k->ext) {
            k->ext = v;                                   /* deepest point so far  */
        } else if ((int16_t)(k->ext - v) >= (int16_t)c->release_pm) {
            k->down = false;                              /* moved up enough       */
            k->ext  = v;
        }
    } else {
        if (v < k->ext) {
            k->ext = v;                                   /* highest point so far  */
        } else if ((int16_t)(v - k->ext) >= (int16_t)c->press_pm) {
            k->down = true;                               /* moved down enough     */
            k->ext  = v;
        }
    }
    return k->down;
}

RT_SCAN_CODE bool rt_update_scan_travel(rt_key_t *key, const rt_scan_cfg_t *scan, uint16_t travel) {
    return update_travel(key, scan, (int16_t)travel);
}

RT_SCAN_CODE bool rt_update_scan_sample(rt_key_t *key, const rt_scan_cfg_t *scan, uint16_t raw, uint16_t *travel) {
    *travel = rt_scan_normalize(key, raw, scan);
    return rt_update_scan_travel(key, scan, *travel);
}

// In the reset zone no previous RT anchor survives. Zero is a canonical
// anchor until actuation arms the key again; telemetry still gets exact travel.
RT_SCAN_CODE bool rt_update_scan_cached(rt_key_t *key, const rt_raw_cache_t *cache,
        const rt_scan_cfg_t *scan, uint16_t raw, uint16_t *travel) {
    if (raw < cache->release_raw) {
        key->down = false;
        key->latched = false;
        key->ext = 0;
        if (travel) *travel = scan_normalize(key, raw, scan);
        return false;
    }
    uint16_t value = scan_normalize(key, raw, scan);
    if (travel) *travel = value;
    return update_travel(key, scan, (int16_t)value);
}

bool rt_update_curve(rt_key_t *key, const rt_cfg_t *cfg, uint16_t raw, const uint16_t *curve) {
    rt_scan_cfg_t scan;
    rt_prepare_scan(&scan, cfg, curve);
    return update_travel(key, &scan, (int16_t)normalize(key, raw, curve));
}

bool rt_update(rt_key_t *k, const rt_cfg_t *c, uint16_t raw) {
    return rt_update_curve(k, c, raw, 0);
}

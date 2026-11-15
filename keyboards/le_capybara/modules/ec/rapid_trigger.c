#include "rapid_trigger.h"

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
}

static inline __attribute__((always_inline)) uint16_t normalize(const rt_key_t *k, uint16_t raw) {
    if (raw <= k->base) {
        return 0;
    }
    uint32_t d = (uint32_t)(raw - k->base);
    // Split the Q16 scale so both products fit in 32 bits, even for a full uint16_t sample.
    uint32_t pm = d * (k->scale >> 16) + ((d * (k->scale & UINT16_MAX)) >> 16);
    return (pm > RT_MAX_PM) ? (uint16_t)RT_MAX_PM : (uint16_t)pm;
}

uint16_t rt_normalize(const rt_key_t *k, uint16_t raw) {
    return normalize(k, raw);
}

#ifdef HHKB_SCAN_1KHZ
__attribute__((section(".RamFunc"), optimize("O2")))
#endif
bool rt_update(rt_key_t *k, const rt_cfg_t *c, uint16_t raw) {
    int16_t v = (int16_t)normalize(k, raw);

    /* Top dead zone: key is definitely up, RT disarmed until act_pm is crossed again. */
    if (v <= (int16_t)c->reset_pm) {
        k->down    = false;
        k->latched = false;
        k->ext     = v;
        return false;
    }

    /* Classic mode: fixed actuation point with release hysteresis. */
    if (!c->rt_enabled) {
        int16_t rel = (c->act_pm > c->release_pm) ? (int16_t)(c->act_pm - c->release_pm) : (int16_t)c->reset_pm;
        if (rel < (int16_t)c->reset_pm) {
            rel = (int16_t)c->reset_pm;
        }
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

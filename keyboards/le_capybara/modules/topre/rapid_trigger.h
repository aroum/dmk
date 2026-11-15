#ifndef RAPID_TRIGGER_H
#define RAPID_TRIGGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Rapid Trigger for analog (Topre capacitive) keys.
 *
 * All thresholds are in PERMILLE (1/1000) of the per-key calibrated signal
 * range (baseline = key at rest, peak = key bottomed out). 10 = 1.0 %.
 * The Topre signal is not linear in travel, so these are NOT millimetres.
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

void     rt_cfg_defaults(rt_cfg_t *c);
void     rt_key_reset(rt_key_t *k);
void     rt_key_set_range(rt_key_t *k, uint16_t base, uint16_t peak);
uint16_t rt_normalize(const rt_key_t *k, uint16_t raw);

/* Feed one raw ADC sample, returns the logical key state (true = pressed). */
bool     rt_update(rt_key_t *k, const rt_cfg_t *c, uint16_t raw);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_TRIGGER_H */

#pragma once
// SOF-locked scan scheduling math. Units: TIM2 ticks (4 per us), 16-bit wrapping counter.
#include <stdint.h>
#include <stdbool.h>

// Division-free (Cortex-M0+ has no divider; inputs are bounded to a few periods).
static inline int32_t sof_wrap(int32_t e, int32_t p) {
    while (e >= p / 2) e -= p;
    while (e < -p / 2) e += p;
    return e;
}

// Phase of timestamp t within the frame that started at sof (sof may be just after t).
static inline uint16_t sof_phase(uint16_t t, uint16_t sof, uint16_t p) {
    int32_t ph = (int16_t)(uint16_t)(t - sof);
    while (ph < 0) ph += p;
    while (ph >= p) ph -= p;
    return (uint16_t)ph;
}

// Per-period tick correction (gain 1/4, clamped to +-max_step).
static inline int16_t sof_lock_correction(uint16_t fired, uint16_t sof, uint16_t p, uint16_t target, uint16_t max_step) {
    int32_t err = sof_wrap((int32_t)sof_phase(fired, sof, p) - (int32_t)target, p);
    int32_t corr = -(err + (err >= 0 ? 2 : -2)) / 4;
    if (corr > (int32_t)max_step) corr = max_step;
    if (corr < -(int32_t)max_step) corr = -(int32_t)max_step;
    return (int16_t)corr;
}

static inline uint16_t sof_lock_target(uint16_t p, int32_t in_offset, uint16_t lead) {
    int32_t t = in_offset - (int32_t)lead;
    while (t < 0) t += p;
    while (t >= p) t -= p;
    return (uint16_t)t;
}

#ifndef SOF_LOCK_HOST_TEST
uint16_t sof_lock_step(uint16_t fired, uint16_t nominal);
void sof_lock_init(void);
bool sof_lock_enabled(void);
bool sof_lock_set_enabled(bool enabled);
void sof_lock_reset(void);
void sof_lock_note_work(uint16_t fire);
uint16_t sof_lock_last_fire(void);
void sof_lock_note_publish(void);
void sof_lock_note_fire(uint16_t fired);
void usb_keyboard_report_armed(void);
void usb_xfer_complete_from_isr(void);
void hhkb_get_latency_diagnostics(uint32_t *avg_us, uint32_t *max_us, int32_t *err_us,
                                  uint32_t *locked, uint32_t *lead_us, int32_t *in_us);
#endif

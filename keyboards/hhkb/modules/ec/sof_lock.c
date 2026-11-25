// Locks the 1 kHz matrix scan to the USB frame so reports are armed just
// before the host's IN poll. Uses the CRS SYNC interrupt (CRS already tracks USB SOF).
#include "config.h"
#include "stm32l072xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sof_lock.h"

#if HHKB_SOF_LOCK || defined(HHKB_SCAN_DIAGNOSTICS)
static volatile bool s_report_pending;
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
static volatile uint16_t s_publish_compare;
static volatile uint16_t s_age_from;
static volatile uint32_t s_age_sum, s_age_cnt, s_age_max;
#endif
#if HHKB_SOF_LOCK || defined(HHKB_SCAN_DIAGNOSTICS)
static volatile uint16_t s_last_compare;
void sof_lock_note_fire(uint16_t fired) { s_last_compare = fired; }
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
static volatile int16_t s_last_err;
void sof_lock_note_publish(void) { s_publish_compare = s_last_compare; }
#endif

#if HHKB_SOF_LOCK || defined(HHKB_SCAN_DIAGNOSTICS)
void usb_keyboard_report_armed(void) {
    if (!s_report_pending) {
#ifdef HHKB_SCAN_DIAGNOSTICS
        s_age_from = s_publish_compare;
#endif
        s_report_pending = true;
    }
}
#endif

#if HHKB_SOF_LOCK
static volatile bool s_enabled = true;
static volatile uint16_t s_sof_cnt;
static volatile uint32_t s_sof_p_q4 = 4000U * 16U;
static volatile uint8_t s_sof_good;
static volatile uint16_t s_lead = 600U * 4U;
static volatile int16_t s_in_off;
static volatile uint8_t s_in_n;
static volatile uint16_t s_work_peak;

void RCC_CRS_IRQHandler(void) {
    uint32_t f = CRS->ISR & (CRS_ISR_SYNCOKF | CRS_ISR_SYNCWARNF);
    if (!f) return;
    uint16_t now = (uint16_t)TIM2->CNT;
    CRS->ICR = CRS_ICR_SYNCOKC | CRS_ICR_SYNCWARNC;
    if (!s_enabled) return;
    uint16_t d = (uint16_t)(now - s_sof_cnt);
    if (d > 3800 && d < 4200) {
        if (s_sof_good) s_sof_p_q4 = (uint32_t)(s_sof_p_q4 + ((((int32_t)d << 4) - (int32_t)s_sof_p_q4) >> 3));
        if (s_sof_good < 255) s_sof_good++;
    } else {
        s_sof_good = 0;
    }
    s_sof_cnt = now;
}

void sof_lock_init(void) {
    taskENTER_CRITICAL();
    CRS->ICR = 0xFU;
    CRS->CR |= CRS_CR_SYNCOKIE | CRS_CR_SYNCWARNIE;
    taskEXIT_CRITICAL();
    NVIC_SetPriority(RCC_CRS_IRQn, 1);
    NVIC_EnableIRQ(RCC_CRS_IRQn);
}

static bool fresh(uint16_t now) {
    return s_enabled && s_sof_good >= 8 && (uint16_t)(now - s_sof_cnt) < 12000;
}

uint16_t sof_lock_step(uint16_t fired, uint16_t nominal) {
    s_last_compare = fired;
    if (!s_enabled || !fresh(fired)) return nominal;
    uint32_t pq = s_sof_p_q4;
    uint16_t p = (uint16_t)((pq + 8) >> 4);
    uint16_t ms = (uint16_t)((nominal + 2000U) / 4000U);
    if (!ms) ms = 1;
    uint16_t base = (uint16_t)(((uint32_t)ms * pq + 8U) >> 4);
    int16_t corr = sof_lock_correction(fired, s_sof_cnt, p, sof_lock_target(p, s_in_off, s_lead), 100);
#ifdef HHKB_SCAN_DIAGNOSTICS
    s_last_err = (int16_t)(-4 * corr);
#endif
    return (uint16_t)(base + corr);
}

void sof_lock_note_work(void) {
    if (!s_enabled) return;
    uint16_t w = (uint16_t)(TIM2->CNT - s_last_compare);
    uint16_t pk = s_work_peak;
    pk = w > pk ? w : (uint16_t)(pk - ((pk - w) >> 6));
    s_work_peak = pk;
    uint32_t lead = (uint32_t)pk + HHKB_SOF_MARGIN_US * 4U;
    if (lead < 200U) lead = 200U;
    // The scan takes most of a frame, so work+margin exceeds one frame and the
    // lock must fire before the *preceding* IN poll for the publish to beat the
    // next one. sof_lock_target() wraps modulo the frame period; cap just under
    // two frames so a work spike cannot push the target a whole frame away.
    if (lead > 7800U) lead = 7800U;
    s_lead = (uint16_t)lead;
}

void sof_lock_reset(void) {
    s_report_pending = false;
    s_sof_good = 0;
    s_in_n = 0;
}
#endif

#if HHKB_SOF_LOCK || defined(HHKB_SCAN_DIAGNOSTICS)
void usb_xfer_complete_from_isr(void) {
    if (!s_report_pending) return;
    uint16_t now = (uint16_t)TIM2->CNT;
#if HHKB_SOF_LOCK
    if (fresh(now)) {
        static uint8_t s_in_rejects;
        int32_t p = (int32_t)((s_sof_p_q4 + 8) >> 4);
        int32_t dlt = sof_wrap((int32_t)sof_phase(now, s_sof_cnt, (uint16_t)p) - s_in_off, p);
        int32_t off = s_in_off;
        if (s_in_n < 4) {
            off += dlt >> s_in_n; // 1, 1/2, 1/4, 1/8: fast acquire without division
            s_in_n++;
        } else if (dlt <= 400 && dlt >= -400) {
            off += dlt >> 3;
        } else {
            // This hook fires for every endpoint's completion. Once the IN
            // phase is acquired, a completion this far off is a console/VIA/EP0
            // transfer finishing while the keyboard report is still in flight:
            // consuming it would clear the pending flag early and skew the age
            // statistics. Leave the report pending for the keyboard completion,
            // which lands near s_in_off by construction. If every completion is
            // rejected the acquired offset itself is wrong (host phase moved
            // without a reset); reacquire instead of staying deaf forever.
            if (++s_in_rejects >= 8) {
                s_in_rejects = 0;
                s_in_n = 0;
            }
            return;
        }
        s_in_rejects = 0;
        if (off >= p) off -= p;
        if (off < 0) off += p;
        s_in_off = (int16_t)off;
    }
#endif
    s_report_pending = false;
#ifdef HHKB_SCAN_DIAGNOSTICS
    uint32_t age = (uint16_t)(now - s_age_from);
    s_age_sum += age;
    s_age_cnt++;
    if (age > s_age_max) s_age_max = age;
#endif
}
#endif

#ifdef HHKB_SCAN_DIAGNOSTICS
void hhkb_get_latency_diagnostics(uint32_t *avg_us, uint32_t *max_us, int32_t *err_us,
                                  uint32_t *locked, uint32_t *lead_us, int32_t *in_us) {
    taskENTER_CRITICAL();
    uint32_t sum = s_age_sum, cnt = s_age_cnt, mx = s_age_max;
    s_age_sum = 0; s_age_cnt = 0; s_age_max = 0;
    taskEXIT_CRITICAL();
    if (avg_us) *avg_us = cnt ? sum / cnt / 4U : 0;
    if (max_us) *max_us = mx / 4U;
#if HHKB_SOF_LOCK
    if (err_us) *err_us = s_last_err / 4;
    if (locked) *locked = fresh((uint16_t)TIM2->CNT) ? 1U : 0U;
    if (lead_us) *lead_us = s_lead / 4U;
    if (in_us) *in_us = s_in_off / 4;
#else
    if (err_us) *err_us = 0;
    if (locked) *locked = 0;
    if (lead_us) *lead_us = 0;
    if (in_us) *in_us = 0;
#endif
}
#endif

// Session-only control: no settings snapshot/EEPROM writes.
bool sof_lock_enabled(void) {
#if HHKB_SOF_LOCK
    return s_enabled;
#else
    return false;
#endif
}
bool sof_lock_set_enabled(bool enabled) {
#if HHKB_SOF_LOCK
    taskENTER_CRITICAL();
    if (s_enabled != enabled) {
        s_enabled = enabled;
        sof_lock_reset();
#ifdef HHKB_SCAN_DIAGNOSTICS
        s_last_err = 0;
#endif
    }
    taskEXIT_CRITICAL();
    return true;
#else
    return !enabled;
#endif
}

#pragma once
#include <stdint.h>

// Cross-task calibration requests. Other tasks only post a request; the matrix
// task alone applies it at a scan boundary, so scan state is never changed
// mid-scan. A newer request replaces a pending one in the same slot (last
// wins). ENABLE/DISABLE carry the set_enabled value and use their own slot.
// The including file supplies CAL_REQ_ENTER()/CAL_REQ_EXIT() (critical section).
typedef enum { CAL_REQ_NONE = 0, CAL_REQ_START, CAL_REQ_FINISH, CAL_REQ_ENABLE, CAL_REQ_DISABLE } cal_request_t;

static inline void cal_request_post(volatile uint8_t *slot, cal_request_t req) {
    CAL_REQ_ENTER();
    *slot = (uint8_t)req;
    CAL_REQ_EXIT();
}

static inline cal_request_t cal_request_take(volatile uint8_t *slot) {
    if (*slot == CAL_REQ_NONE) return CAL_REQ_NONE;
    CAL_REQ_ENTER();
    cal_request_t req = (cal_request_t)*slot;
    *slot = CAL_REQ_NONE;
    CAL_REQ_EXIT();
    return req;
}

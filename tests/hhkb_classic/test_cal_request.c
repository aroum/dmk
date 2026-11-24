#include <assert.h>
#include "stdint.h"
static int s_depth, s_max_depth;
#define CAL_REQ_ENTER() (++s_depth, s_max_depth = s_depth)
#define CAL_REQ_EXIT() (--s_depth)
#include "cal_request.h"

int main(void) {
    volatile uint8_t slot = 0;
    assert(cal_request_take(&slot) == CAL_REQ_NONE);
    cal_request_post(&slot, CAL_REQ_START);
    assert(cal_request_take(&slot) == CAL_REQ_START);
    assert(cal_request_take(&slot) == CAL_REQ_NONE); // consumed once
    cal_request_post(&slot, CAL_REQ_START);
    cal_request_post(&slot, CAL_REQ_FINISH); // last wins
    assert(cal_request_take(&slot) == CAL_REQ_FINISH);
    cal_request_post(&slot, CAL_REQ_FINISH);
    cal_request_post(&slot, CAL_REQ_START);
    assert(cal_request_take(&slot) == CAL_REQ_START);
    cal_request_post(&slot, CAL_REQ_ENABLE);
    cal_request_post(&slot, CAL_REQ_DISABLE); // last wins
    assert(cal_request_take(&slot) == CAL_REQ_DISABLE);
    cal_request_post(&slot, CAL_REQ_DISABLE);
    cal_request_post(&slot, CAL_REQ_ENABLE);
    assert(cal_request_take(&slot) == CAL_REQ_ENABLE);
    assert(cal_request_take(&slot) == CAL_REQ_NONE);
    { // set_enabled uses its own slot: does not displace start/finish
        volatile uint8_t en = 0;
        cal_request_post(&slot, CAL_REQ_START);
        cal_request_post(&en, CAL_REQ_DISABLE);
        assert(cal_request_take(&slot) == CAL_REQ_START);
        assert(cal_request_take(&en) == CAL_REQ_DISABLE);
    }
    assert(s_depth == 0 && s_max_depth == 1);
    return 0;
}

#include <assert.h>
#include <stdlib.h>
#include "sof_lock.h"

static int jit(int a) { return a ? (rand() % (2 * a + 1)) - a : 0; }

// Simulates the TIM2 ISR loop against a SOF stream with a given real frame period.
static int simulate(int frame_real, int ms, int start, int iters, int *max_late_err) {
    uint16_t p = (uint16_t)frame_real; // measured period
    uint16_t target = sof_lock_target(p, 1200, 600 * 4);
    long t = start;           // true time of compare
    long sof0 = 0;            // true SOF k at sof0 + k*frame_real
    int last_err = 0;
    *max_late_err = 0;
    for (int i = 0; i < iters; i++) {
        long k = (t - sof0) / frame_real;
        long sof_t = sof0 + k * frame_real + jit(30);
        uint16_t fired = (uint16_t)t, sof = (uint16_t)sof_t;
        uint16_t ph = sof_phase(fired, sof, p);
        // True phase error against the jitter-free SOF.
        last_err = (int)sof_wrap((int)sof_phase(fired, (uint16_t)(sof0 + k * frame_real), p) - (int)target, p);
        (void)ph;
        int16_t c = sof_lock_correction(fired, sof, p, target, 100);
        assert(c >= -100 && c <= 100);
        if (i >= 60 && abs(last_err) > *max_late_err) *max_late_err = abs(last_err);
        t += (long)ms * frame_real + c;
    }
    return last_err;
}

int main(void) {
    // (a) phase across wrap and SOF after fired
    assert(sof_phase(10, 65530, 4000) == 16);
    assert(sof_phase(65530, 10, 4000) == 4000 - 16);
    assert(sof_phase(100, 100, 4000) == 0);
    // (e) target wrap
    assert(sof_lock_target(4000, 100, 2400) == 1700);
    assert(sof_lock_target(4000, 3000, 600) == 2400);
    // (b) clamp
    assert(sof_lock_correction(2000, 0, 4000, 0, 25) == 25);
    assert(sof_lock_correction(0, 0, 4000, 0, 25) == 0);
    srand(1);
    int frames[] = {4040, 3960, 4000};
    int mss[] = {1, 2, 5, 10, 11};
    for (unsigned a = 0; a < 3; a++)
        for (unsigned b = 0; b < 5; b++)
            for (int start = 0; start < 4000 * 3; start += 997) {
                int m;
                simulate(frames[a], mss[b], start + 70000, 500, &m);
                assert(m < 40);
            }
    return 0;
}

/* Host test; run via tests/run_all_tests.sh. */
#include "rapid_trigger.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BASE 800
#define PEAK 1800                       /* range 1000 counts -> 1 count == 1 permille */

static rt_key_t key;
static rt_cfg_t cfg;

static uint16_t raw(int pm) { return (uint16_t)(BASE + pm); }
static bool feed(int pm) { return rt_update(&key, &cfg, raw(pm)); }

static void setup(bool rt) {
    rt_cfg_defaults(&cfg);
    cfg.rt_enabled = rt;
    rt_key_set_range(&key, BASE, PEAK);
    rt_key_reset(&key);
}

int main(void) {
    int events;

    /* 1. Slow press: nothing before act point (500), pressed right at it. */
    setup(true);
    for (int pm = 0; pm < 500; pm += 5) assert(!feed(pm));
    assert(feed(500));

    /* 2. Mid-travel release: 20 up is ignored, 30 up releases. */
    for (int pm = 500; pm <= 1000; pm += 10) assert(feed(pm));
    assert(feed(980));
    assert(feed(971));
    assert(!feed(970));

    /* 3. Re-press without returning to the top: 20 down ignored, 30 down presses. */
    assert(!feed(980));
    assert(!feed(990));
    assert(feed(1000));

    /* 4. Rapid wiggle in 600..700 produces many transitions. */
    setup(true);
    for (int pm = 0; pm <= 650; pm += 10) feed(pm);
    events = 0;
    bool prev = feed(650);
    for (int i = 0; i < 10; i++) {
        for (int pm = 650; pm >= 600; pm -= 5) { bool s = feed(pm); if (s != prev) { events++; prev = s; } }
        for (int pm = 600; pm <= 650; pm += 5) { bool s = feed(pm); if (s != prev) { events++; prev = s; } }
    }
    assert(events == 20);

    /* 5. Going to the top dead zone disarms RT; partial press below act does not trigger. */
    feed(40);
    assert(!feed(400));
    assert(!feed(499));
    assert(feed(500));

    /* 6. Noise +-8 around a held key does not chatter (needs 30 drop from max). */
    setup(true);
    for (int pm = 0; pm <= 800; pm += 10) feed(pm);
    srand(1);
    for (int i = 0; i < 100000; i++) assert(feed(800 + (rand() % 17) - 8));

    /* 7. Noise +-8 below the actuation point never presses. */
    setup(true);
    for (int i = 0; i < 100000; i++) assert(!feed(470 + (rand() % 17) - 8));

    /* 8. Classic mode: fixed act 500, hysteresis = release_pm (30). */
    setup(false);
    assert(!feed(499));
    assert(feed(500));
    assert(feed(471));
    assert(!feed(469));
    assert(!feed(480));
    assert(feed(500));

    /* 9. Calibration fallback: peak <= base+150 does not divide by ~0. */
    rt_key_set_range(&key, BASE, BASE + 10);
    assert(rt_normalize(&key, BASE + 500) == 500);

    puts("rapid trigger: all tests passed");
    return 0;
}

#include "ec_curve.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(ec_curve_apply(NULL, 1100) == 1100);
    assert(ec_curve_set(EC_CURVE_P1, 25));
    assert(ec_curve_set(EC_CURVE_P2, 50));
    assert(ec_curve_set(EC_CURVE_P3, 75));
    assert(ec_curve_set(EC_CURVE_ENABLE, 1));
    ec_curve_rebuild();
    assert(ec_curve_lut);
    for (unsigned i = 0; i <= 1000; ++i)
        assert(ec_curve_apply(ec_curve_lut, i) == i);

    assert(ec_curve_set(EC_CURVE_DEPTH, 45));
    assert(ec_curve_set(EC_CURVE_SMOOTH, 0));
    ec_curve_rebuild();
    assert(ec_curve_lut);
    assert(ec_curve_apply(ec_curve_lut, 1000) == 1125);
    assert(ec_curve_apply(ec_curve_lut, 1100) == 1125);
    assert(ec_curve_apply(ec_curve_lut, 880) == 945);
    for (unsigned i = 1; i <= 1000; ++i)
        assert(ec_curve_apply(ec_curve_lut, i) >= ec_curve_apply(ec_curve_lut, i - 1));

    assert(ec_curve_set(EC_CURVE_P3, 98));
    ec_curve_rebuild();
    assert(!ec_curve_lut);
    assert(ec_curve_set(EC_CURVE_P3, 50));
    ec_curve_rebuild();
    assert(!ec_curve_lut);
    assert(ec_curve_set(EC_CURVE_ENABLE, 0));
    ec_curve_rebuild();
    assert(!ec_curve_lut);
    puts("EC travel curve tests passed");
}

#include "ec_curve.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ec_curve_reference.h"

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
    unsigned seed=17, max_error=0, boundary_cases=0;
    for(unsigned trial=0;trial<50000;++trial) {
        uint8_t settings[6]={1,0,0,0,0,0};
        for(unsigned k=1;k<=3;++k) {seed=seed*1664525U+1013904223U;settings[k]=2+(seed%97);}
        for(unsigned k=1;k<=3;++k)for(unsigned j=k+1;j<=3;++j)
            if(settings[k]>settings[j]) {uint8_t t=settings[k];settings[k]=settings[j];settings[j]=t;}
        settings[4]=31+trial%15;settings[5]=trial%11;
        uint16_t reference[126];
        bool expected=reference_curve(settings,reference);
        memcpy(ec_curve_settings,settings,6);ec_curve_rebuild();
        if(expected!=(ec_curve_lut!=NULL)) {
            unsigned step=0;for(unsigned k=8;k<126;++k)
                if(reference[k]-reference[k-1]>step)step=reference[k]-reference[k-1];
            assert(step==32 || step==33);++boundary_cases;
        }
        if(!ec_curve_lut) continue;
        for(unsigned k=0;k<126;++k) {
            unsigned error=ec_curve_lut[k]>reference[k]?ec_curve_lut[k]-reference[k]:reference[k]-ec_curve_lut[k];
            assert(error<=1);if(error>max_error)max_error=error;
            if(k)assert(ec_curve_lut[k]>=ec_curve_lut[k-1]);
            if(k>=8)assert(ec_curve_lut[k]-ec_curve_lut[k-1]<=32);
        }
    }
    printf("fixed-point curve: max error %u/1000, slope-boundary rounding cases %u\n",max_error,boundary_cases);
    puts("EC travel curve tests passed");
}

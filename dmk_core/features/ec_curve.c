#include "ec_curve.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

uint8_t ec_curve_settings[EC_CURVE_FIELDS] = {0, 15, 50, 80, 40, 10};
const uint16_t *ec_curve_lut;
uint32_t ec_curve_revision;
const char *ec_curve_status = "Disabled";
static uint16_t s_curve[126];

static void disable_curve(const char *status) {
    taskENTER_CRITICAL();
    if (ec_curve_lut) {
        ec_curve_lut = 0;
        ++ec_curve_revision;
    }
    taskEXIT_CRITICAL();
    ec_curve_status = status;
}

bool ec_curve_set(uint8_t id, uint8_t value) {
    switch (id) {
    case EC_CURVE_ENABLE: if (value > 1) return false; break;
    case EC_CURVE_P1: if (value < 2 || value > 94) return false; break;
    case EC_CURVE_P2: if (value < 4 || value > 96) return false; break;
    case EC_CURVE_P3: if (value < 6 || value > 98) return false; break;
    case EC_CURVE_DEPTH: if (value < 31 || value > 45) return false; break;
    case EC_CURVE_SMOOTH: if (value > 10) return false; break;
    default: return false;
    }
    ec_curve_settings[id - EC_CURVE_ENABLE] = value;
    return true;
}

/* Q20 is used only while constructing the table. All products fit int64_t:
 * x<=1000, y<=1125 and point spacing>=20. No math runs in the scan path. */
#define CURVE_Q 20
#define CURVE_ONE (INT64_C(1) << CURVE_Q)
static int64_t edge(int32_t h0, int32_t h1, int64_t d0, int64_t d1) {
    int64_t slope = ((2*h0+h1)*d0-h0*d1)/(h0+h1);
    return slope > 0 ? slope : 0;
}

void ec_curve_rebuild(void) {
    static uint8_t applied[EC_CURVE_FIELDS];
    if (memcmp(applied, ec_curve_settings, sizeof(applied)) == 0) return;
    memcpy(applied, ec_curve_settings, sizeof(applied));
    // Settings are rebuilt from the USB task or during initialization.
    if (!ec_curve_settings[0]) {
        disable_curve("Disabled");
        return;
    }

    const int32_t x[] = {0, ec_curve_settings[1]*10, ec_curve_settings[2]*10,
                         ec_curve_settings[3]*10, 1000};
    const int32_t y[] = {0, 250, 500, 750, ec_curve_settings[4]*25};
    const unsigned smooth = ec_curve_settings[5];
    int32_t h[4];
    int64_t d[4], m[5];
    for (unsigned i=0; i<4; ++i) {
        h[i]=x[i+1]-x[i];
        if (h[i]<20) { disable_curve("Points need 2% spacing"); return; }
        d[i]=(y[i+1]-y[i])*CURVE_ONE/h[i];
    }
    for (unsigned i=1; i<4; ++i) {
        int32_t left=2*h[i]+h[i-1], right=h[i]+2*h[i-1];
        m[i]=(left+right)*d[i-1]*d[i]/(left*d[i]+right*d[i-1]);
    }
    m[0]=edge(h[0],h[1],d[0],d[1]);
    m[4]=edge(h[3],h[2],d[3],d[2]);
    uint16_t candidate[126];
    unsigned segment=0;
    for (unsigned i=0; i<126; ++i) {
        int32_t signal=i*8;
        if (segment<3 && signal>=x[segment+1]) ++segment;
        int64_t u=(signal-x[segment])*CURVE_ONE/h[segment];
        int64_t delta=(y[segment+1]-y[segment])*CURVE_ONE;
        int64_t c=h[segment]*m[segment];
        int64_t a=-2*delta+c+h[segment]*m[segment+1];
        int64_t b=3*delta-2*c-h[segment]*m[segment+1];
        int64_t linear=y[segment]*CURVE_ONE+(u*delta)/CURVE_ONE;
        int64_t cubic=y[segment]*CURVE_ONE+
            u*(c+u*(b+u*a/CURVE_ONE)/CURVE_ONE)/CURVE_ONE;
        int64_t value=linear+(cubic-linear)*smooth/10;
        candidate[i]=(uint16_t)((value+CURVE_ONE/2)/CURVE_ONE);
        if (i && candidate[i]<candidate[i-1]) candidate[i]=candidate[i-1];
        if (i>=8 && candidate[i]-candidate[i-1]>32) {
            disable_curve("Curve too steep (max 4x)"); return;
        }
    }
    // Called at startup or from the USB task, below matrix task priority.
    // Publish the complete table atomically with respect to the scanner.
    taskENTER_CRITICAL();
    memcpy(s_curve, candidate, sizeof(s_curve));
    ec_curve_lut = s_curve;
    ++ec_curve_revision;
    taskEXIT_CRITICAL();
    ec_curve_status = "Active";
}

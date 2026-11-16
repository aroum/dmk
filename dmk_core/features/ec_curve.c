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

static float edge(float h0, float h1, float d0, float d1) {
    float slope = ((2.0f * h0 + h1) * d0 - h0 * d1) / (h0 + h1);
    return slope > 0.0f ? slope : 0.0f;
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

    const float x[] = {0, ec_curve_settings[1] * 10, ec_curve_settings[2] * 10,
                       ec_curve_settings[3] * 10, 1000};
    const float y[] = {0, 250, 500, 750, ec_curve_settings[4] * 25};
    const float smooth = ec_curve_settings[5] / 10.0f;
    float h[4], d[4], m[5];
    for (unsigned i = 0; i < 4; ++i) {
        h[i] = x[i + 1] - x[i];
        if (h[i] < 20) {
            disable_curve("Points need 2% spacing");
            return;
        }
        d[i] = (y[i + 1] - y[i]) / h[i];
    }
    for (unsigned i = 1; i < 4; ++i) {
        float left = 2.0f * h[i] + h[i - 1];
        float right = h[i] + 2.0f * h[i - 1];
        m[i] = (left + right) / (left / d[i - 1] + right / d[i]);
    }
    m[0] = edge(h[0], h[1], d[0], d[1]);
    m[4] = edge(h[3], h[2], d[3], d[2]);

    uint16_t candidate[126];
    unsigned segment = 0;
    for (unsigned i = 0; i < 126; ++i) {
        float signal = i * 8;
        if (segment < 3 && signal >= x[segment + 1]) ++segment;
        float u = (signal - x[segment]) / h[segment];
        float a = y[segment], b = y[segment + 1];
        float linear = a + u * (b - a);
        float cubic = a + u * u * (3.0f - 2.0f * u) * (b - a) +
            h[segment] * u * (1.0f - u) *
            ((1.0f - u) * m[segment] - u * m[segment + 1]);
        candidate[i] = (uint16_t)(linear + smooth * (cubic - linear) + 0.5f);
        if (i && candidate[i] < candidate[i - 1]) candidate[i] = candidate[i - 1];
        if (i >= 8 && candidate[i] - candidate[i - 1] > 32) {
            disable_curve("Curve too steep (max 4x)");
            return;
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

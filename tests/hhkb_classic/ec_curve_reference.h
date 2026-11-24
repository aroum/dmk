/* Float reference retained for host-only regression tests. Never linked into firmware. */
static float reference_edge(float h0, float h1, float d0, float d1) {
    float slope = ((2.0f * h0 + h1) * d0 - h0 * d1) / (h0 + h1);
    return slope > 0.0f ? slope : 0.0f;
}

static bool reference_curve(const uint8_t settings[6], uint16_t candidate[126]) {
    bool valid=true;
    const float x[] = {0, settings[1] * 10, settings[2] * 10,
                       settings[3] * 10, 1000};
    const float y[] = {0, 250, 500, 750, settings[4] * 25};
    const float smooth = settings[5] / 10.0f;
    float h[4], d[4], m[5];
    for (unsigned i = 0; i < 4; ++i) {
        h[i] = x[i + 1] - x[i];
        if (h[i] < 20) {
            return false;
        }
        d[i] = (y[i + 1] - y[i]) / h[i];
    }
    for (unsigned i = 1; i < 4; ++i) {
        float left = 2.0f * h[i] + h[i - 1];
        float right = h[i] + 2.0f * h[i - 1];
        m[i] = (left + right) / (left / d[i - 1] + right / d[i]);
    }
    m[0] = reference_edge(h[0], h[1], d[0], d[1]);
    m[4] = reference_edge(h[3], h[2], d[3], d[2]);

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
            valid = false;
        }
    }
    return valid;
}

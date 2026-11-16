#ifndef DMK_EC_CURVE_H
#define DMK_EC_CURVE_H

#include <stdbool.h>
#include <stdint.h>

enum { EC_CURVE_ENABLE = 30, EC_CURVE_P1, EC_CURVE_P2, EC_CURVE_P3,
       EC_CURVE_DEPTH, EC_CURVE_SMOOTH, EC_CURVE_STATUS };
#define EC_CURVE_FIELDS 6

extern uint8_t ec_curve_settings[EC_CURVE_FIELDS];
extern const uint16_t *ec_curve_lut;
extern uint32_t ec_curve_revision;
extern const char *ec_curve_status;
void ec_curve_rebuild(void);
bool ec_curve_set(uint8_t id, uint8_t value);

static inline __attribute__((always_inline))
uint16_t ec_curve_apply(const uint16_t *lut, uint16_t pm) {
    if (!lut) return pm;
    if (pm >= 1000) return lut[125];
    unsigned index = pm >> 3;
    return lut[index] + (((lut[index + 1] - lut[index]) * (pm & 7u)) >> 3);
}

#endif

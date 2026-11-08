#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const int16_t lut_basic[8][4][64][256];
extern const int16_t lut_pulse[8][16][16][256];
extern const float lut_pam4[128][16];
extern const float lut_norm_gain[128][128];
extern const float lut_norm_bias[128][128];

#ifdef __cplusplus
}
#endif

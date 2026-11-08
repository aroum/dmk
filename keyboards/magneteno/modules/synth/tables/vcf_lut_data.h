#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FILTER_MODE_3_TABLES 2
#define CFG_FILTER_MODE FILTER_MODE_3_TABLES

// High-resolution 128-step static LUTs stored in Flash
extern const float g_vcf_res_q_lut[32];
extern const uint16_t g_eg_curve_lut[128][256];

#if CFG_FILTER_MODE == FILTER_MODE_3_TABLES
extern const int32_t g_vcf_lpf_lut[32][128][5];
extern const int32_t g_vcf_bpf_lut[32][128][5];
extern const int32_t g_vcf_hpf_lut[32][128][5];
extern const int32_t g_vcf_bsf_lut[32][128][5];
extern const int32_t g_vcf_apf_lut[32][128][5];
#endif

#ifdef __cplusplus
}
#endif

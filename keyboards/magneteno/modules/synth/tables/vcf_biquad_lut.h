#ifndef VCF_BIQUAD_LUT_H
#define VCF_BIQUAD_LUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Precomputed Biquad Low-Pass Filter coefficients for 32 resonances x 128 cutoffs
// Stored in Flash: each entry is [b0, b1, b2, a1, a2]
extern const float g_vcf_biquad_lut[32][128][5];

#ifdef __cplusplus
}
#endif

#endif // VCF_BIQUAD_LUT_H

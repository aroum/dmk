#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const float lfo_freq_lut[128];
extern const float eg_inc_lut[128];
extern const float glide_time_lut[128];
extern const float pan_l_lut[128];
extern const float pan_r_lut[128];
extern const float detune_factor_lut[128];
extern const float smooth_alpha_lut[128];
extern const float mix_volume_lut[128];
extern const float midi_to_freq_lut[128];

#ifdef __cplusplus
}
#endif

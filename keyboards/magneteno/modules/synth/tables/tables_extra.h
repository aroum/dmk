#ifndef TABLES_EXTRA_H
#define TABLES_EXTRA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t g_note_phase_inc[128];
extern const uint8_t g_note_mipmap_level[128];
extern const float g_soft_clip_lut[1025];
extern const float g_sqrt_lut[128];

#ifdef __cplusplus
}
#endif

#endif // TABLES_EXTRA_H

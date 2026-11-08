#ifndef OMSK_WAVETABLES_H
#define OMSK_WAVETABLES_H

#include <stdint.h>

#define WAVETABLE_SIZE 4096
#define MIPMAP_LEVELS 8

extern const float wt_sin[WAVETABLE_SIZE];
extern const float wt_saw[MIPMAP_LEVELS][WAVETABLE_SIZE];
extern const float wt_square[MIPMAP_LEVELS][WAVETABLE_SIZE];

// PAM4 Patterns
extern const float pam4_patterns[5][16];

// Detune Table
extern const float detune_table[128];

#endif // OMSK_WAVETABLES_H

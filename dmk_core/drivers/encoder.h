#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)

void encoder_init(void);
void encoder_scan(void);

#else

static inline void encoder_init(void) {}
static inline void encoder_scan(void) {}

#endif

#endif // ENCODER_H

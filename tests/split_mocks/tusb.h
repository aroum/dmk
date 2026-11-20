#pragma once
#include <stdbool.h>

/* Host matrix/transport fixtures model an awake USB bus. */
static inline bool tud_suspended(void) { return false; }

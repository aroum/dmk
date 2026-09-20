#ifndef MOCK_CONFIG_H
#define MOCK_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_ROWS 2
#define NUM_COLS 3
#define NUM_KEYS 6

#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, \
    {1, 0}, {1, 1}, {1, 2}  \
}

#define TAPPING_TERM_DEFAULT 200

#endif // MOCK_CONFIG_H

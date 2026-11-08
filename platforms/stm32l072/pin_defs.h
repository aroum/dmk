#ifndef PIN_DEFS_H
#define PIN_DEFS_H

#include <stdint.h>

// STM32L072: Port A (0..15), Port B (16..31), Port C (32..47), Port D (48..63), Port E (64..79), Port H (80..81)
typedef uint8_t pin_t;

// Port A
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7
#define PA8 8
#define PA9 9
#define PA10 10
#define PA11 11
#define PA12 12
#define PA13 13
#define PA14 14
#define PA15 15

// Port B
#define PB0 16
#define PB1 17
#define PB2 18
#define PB3 19
#define PB4 20
#define PB5 21
#define PB6 22
#define PB7 23
#define PB8 24
#define PB9 25
#define PB10 26
#define PB11 27
#define PB12 28
#define PB13 29
#define PB14 30
#define PB15 31

// Port C
#define PC0 32
#define PC1 33
#define PC2 34
#define PC3 35
#define PC4 36
#define PC5 37
#define PC6 38
#define PC7 39
#define PC8 40
#define PC9 41
#define PC10 42
#define PC11 43
#define PC12 44
#define PC13 45
#define PC14 46
#define PC15 47

// Port D
#define PD0 48
#define PD1 49
#define PD2 50

// Short aliases (A0..A15, B0..B15, C0..C15, D0..D2)
#define A0 PA0
#define A1 PA1
#define A2 PA2
#define A3 PA3
#define A4 PA4
#define A5 PA5
#define A6 PA6
#define A7 PA7
#define A8 PA8
#define A9 PA9
#define A10 PA10
#define A11 PA11
#define A12 PA12
#define A13 PA13
#define A14 PA14
#define A15 PA15

#define B0 PB0
#define B1 PB1
#define B2 PB2
#define B3 PB3
#define B4 PB4
#define B5 PB5
#define B6 PB6
#define B7 PB7
#define B8 PB8
#define B9 PB9
#define B10 PB10
#define B11 PB11
#define B12 PB12
#define B13 PB13
#define B14 PB14
#define B15 PB15

#define C0 PC0
#define C1 PC1
#define C2 PC2
#define C3 PC3
#define C4 PC4
#define C5 PC5
#define C6 PC6
#define C7 PC7
#define C8 PC8
#define C9 PC9
#define C10 PC10
#define C11 PC11
#define C12 PC12
#define C13 PC13
#define C14 PC14
#define C15 PC15

#define D0 PD0
#define D1 PD1
#define D2 PD2

#define NO_PIN 0xFF

#endif // PIN_DEFS_H

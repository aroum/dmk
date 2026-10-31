#ifndef PLATFORM_PINS_H
#define PLATFORM_PINS_H

/* ProMicro footprint pin mapping for Baikal (be-u1000) */
/* XP2 connector (left side of Pro Micro) */
#define PM_1 PA6   /* TX (D1) */
#define PM_0 PA7   /* RX (D0) */
/* GND */
/* GND */
#define PM_2 PA13  /* Pin 1 (D2) */
#define PM_3 PA12  /* Pin 2 (D3) */
#define PM_4 PA5   /* Pin 3 (D4) */
#define PM_5 PC9   /* Pin 4 (D5) */
#define PM_6 PC8   /* Pin 5 (D6) */
#define PM_7 PA4   /* Pin 6 (D7) */
#define PM_8 PC6   /* Pin 7 (D8) */
#define PM_9 PC7   /* Pin 8 (D9) */

/* XP1 connector (right side of Pro Micro) */
#define PM_10 PA8  /* Pin 9 (D10) */
#define PM_16 PA10 /* Pin 10 (D16 / MOSI) */
#define PM_14 PA11 /* Pin 11 (D14 / MISO) */
#define PM_15 PA9  /* Pin 12 (D15 / SCK) */
#define PM_18 PA3  /* Pin 13 (D18 / A0) */
#define PM_19 PA2  /* Pin 14 (D19 / A1) */
#define PM_20 PA1  /* Pin 15 (D20 / A2) */
#define PM_21 PA0  /* Pin 16 (D21 / A3) */

#define PM_VCC PC15
#define PM_LED PA15

#endif /* PLATFORM_PINS_H */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "pin_defs.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a hardware pin as an analog ADC input
 * @param pin Hardware pin identifier (pin_t)
 */
void hal_adc_init(pin_t pin);

/**
 * @brief Read 12-bit normalized analog value from pin (0..4095)
 * @param pin Hardware pin identifier (pin_t)
 * @return 12-bit unsigned ADC reading (0..4095)
 */
uint16_t hal_adc_read(pin_t pin);

#ifdef __cplusplus
}
#endif

#endif // HAL_ADC_H

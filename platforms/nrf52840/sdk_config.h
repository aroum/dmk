/*
 * Minimal SDK configuration for nRF52840 support in DMK with TinyUSB.
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "config.h"

/* Clock configuration */
#define NRF_CLOCK_ENABLED 1
#define NRF_CLOCK_CONFIG_LF_SRC 1
#define CLOCK_CONFIG_LF_SRC 1
#define NRFX_CLOCK_CONFIG_LF_SRC 1
#define NRFX_CLOCK_ENABLED 1
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6

/* Power management configuration */
#define NRF_DRV_POWER_ENABLED 1
#define POWER_ENABLED 1
#define NRFX_POWER_ENABLED 1
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 0
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV 0
#define NRFX_POWER_CONFIG_IRQ_PRIORITY 6
#define NRFX_POWER_CLOCK_CONFIG_IRQ_PRIORITY 6
#define POWER_CONFIG_IRQ_PRIORITY 6

/* Logging – disabled */
#define NRF_LOG_ENABLED 0
#define NRF_LOG_DEFAULT_LEVEL 0
#define NRF_LOG_DEFERRED 0

/* Enable SPIM driver and SPIM1 instance */
#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM1_ENABLED 1
#define NRFX_SPIM_MISO_PULL_CFG 1
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 7

#endif // SDK_CONFIG_H

#ifndef DMK_FLASH_STORAGE_H
#define DMK_FLASH_STORAGE_H

// Reserved RP2040/RP2350 erase sectors, counted backwards from flash end.
// Add every persistent owner here so allocations remain disjoint.
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/flash.h"
#define DMK_FLASH_SECTOR_VIAL 1u
#define DMK_FLASH_SECTOR_HALL_CALIBRATION 2u
#define DMK_FLASH_RESERVED_SECTORS 2u
#define DMK_FLASH_OFFSET(sector) (PICO_FLASH_SIZE_BYTES - (sector) * FLASH_SECTOR_SIZE)
#if DMK_FLASH_SECTOR_VIAL == DMK_FLASH_SECTOR_HALL_CALIBRATION
#error "Flash storage sectors overlap"
#endif
#endif

#endif

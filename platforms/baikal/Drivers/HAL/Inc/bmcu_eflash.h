/**
 * *****************************************************************************
 *  @file       bmcu_eflash.h
 *  @author     Baikal electronics SDK team
 *  @brief      MCU embedded flash memory driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_EFLASH_H
#define __BMCU_EFLASH_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (BMCU_U)
#define IS_EFLASH_ADDRESS(ADDRESS)      (((ADDRESS) >= 0xA0000000UL) && ((ADDRESS) <= 0xA003FFFFUL))
#elif defined (BE_U1000)
#define IS_EFLASH_ADDRESS(ADDRESS)      (((ADDRESS) >= 0xA0000000UL) && ((ADDRESS) <= 0xA003FFFFUL))
#endif

typedef enum
{
    EFLASH_COMPLETE = 0,    /*!< Operation complete */
    EFLASH_BUSY,            /*!< Busy */
    EFLASH_TIMEOUT,         /*!< Operation timeout */
    EFLASH_ERROR_CFG        /*!< Configuration error */
} EFLASH_Status_TypeDef;

typedef enum
{
    EFLASH_MAIN_ARRAY = 0,                  /*!< Main array */
    EFLASH_NVR        = EFLASH_CR_NVR     /*!< NVR */
} EFLASH_Region_TypeDef;

/**
 * @brief Writes a value in EFLASH register.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define EFLASH_WriteReg(REG, VALUE)     WRITE_REG(EFLASH->REG, (VALUE))

/**
 * @brief Reads a value from EFLASH register.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define EFLASH_ReadReg(REG)             READ_REG(EFLASH->REG)

/**
 * @brief Checks if the BUSY flag is set.
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t EFLASH_IsActiveFlag_BUSY(void)
{
    return ((READ_BIT(EFLASH->SR, EFLASH_SR_BUSY) != 0x0UL) ? 1UL : 0UL);
}

EFLASH_Status_TypeDef EFLASH_Init(uint32_t CCLK_Freq);
EFLASH_Status_TypeDef EFLASH_ProgramWord(uint32_t Address, uint32_t Data, EFLASH_Region_TypeDef Region);
EFLASH_Status_TypeDef EFLASH_ReadWord(uint32_t Address, uint32_t * Data, EFLASH_Region_TypeDef Region);
EFLASH_Status_TypeDef EFLASH_EraseSector(uint32_t Sector_Address, EFLASH_Region_TypeDef Region);
EFLASH_Status_TypeDef EFLASH_EraseBlock(uint32_t Block_Address, EFLASH_Region_TypeDef Region);
EFLASH_Status_TypeDef EFLASH_EraseAll(EFLASH_Region_TypeDef Region);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_EFLASH_H */

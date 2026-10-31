/**
 * *****************************************************************************
 *  @file       bmcu_eflash.h
 *  @author     Baikal electronics SDK team
 *  @brief      MCU embedded flash memory driver header file
 *  @version    2.4.0
 *  @date       2026-10-06
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

/* EFLASH size (Flash 0 + Flash 1) */
#define EFLASH_SIZE                     (131072UL * 2U) /*!< EFLASH size */
#define EFLASH_BLOCK_SIZE               (4096UL * 2U)   /*!< EFLASH block size */
#define EFLASH_SECTOR_SIZE              (512UL * 2U)    /*!< EFLASH sector size */
#define EFLASH_PAGE_SIZE                (256UL * 2U)    /*!< EFLASH page size */

#define EFLASH_CFG_REG_NUMBER           8U                  /*!< The number of EFLASH configuration registers */
#define EFLASH_CFG_DATA_FLAG            0xF0A55A0FUL        /*!< EFLASH configuration data flag */

#define EFLASH_NVR_SECTOR_NUMBER        4U                  /*!< The number of EFLASH NVR sectors */
#define EFLASH_ARRDN_SECTOR_NUMBER      4U                  /*!< The number of EFLASH redundancy sectors */

#define EFLASH_NVR_SECTOR1_ADDR(addr)   (0x7800U | (addr))  /*!< EFLASH NVR sector 1 address (word number) */
#define EFLASH_NVR_SECTOR2_ADDR(addr)   (0x7900U | (addr))  /*!< EFLASH NVR sector 2 address (word number) */
#define EFLASH_NVR_SECTOR3_ADDR(addr)   (0xF800U | (addr))  /*!< EFLASH NVR sector 3 address (word number) */
#define EFLASH_NVR_SECTOR4_ADDR(addr)   (0xF900U | (addr))  /*!< EFLASH NVR sector 4 address (word number) */

#define EFLASH_ARRDN_SECTOR1_ADDR(addr) (0x7800U | (addr))  /*!< EFLASH redundancy sector 1 address (word number) */
#define EFLASH_ARRDN_SECTOR2_ADDR(addr) (0x7900U | (addr))  /*!< EFLASH redundancy sector 2 address (word number) */
#define EFLASH_ARRDN_SECTOR3_ADDR(addr) (0xF800U | (addr))  /*!< EFLASH redundancy sector 3 address (word number) */
#define EFLASH_ARRDN_SECTOR4_ADDR(addr) (0xF900U | (addr))  /*!< EFLASH redundancy sector 4 address (word number) */

/* Flash 0 */
#define EFLASH_CFG_DATA_FLAG_ADDR_0     EFLASH_NVR_SECTOR1_ADDR(0x80U)  /*!< Flash 0 Configuration Data Flag address (word number) */

#define EFLASH_CFG_WORD0_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xF0U)  /*!< Flash 0 Configuration Word 0 address (word number) */
#define EFLASH_CFG_WORD1_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xF2U)  /*!< Flash 0 Configuration Word 1 address (word number) */
#define EFLASH_CFG_WORD2_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xF4U)  /*!< Flash 0 Configuration Word 2 address (word number) */
#define EFLASH_CFG_WORD3_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xF6U)  /*!< Flash 0 Configuration Word 3 address (word number) */
#define EFLASH_CFG_WORD4_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xF8U)  /*!< Flash 0 Configuration Word 4 address (word number) */
#define EFLASH_CFG_WORD5_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xFAU)  /*!< Flash 0 Configuration Word 5 address (word number) */
#define EFLASH_CFG_WORD6_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xFCU)  /*!< Flash 0 Configuration Word 6 address (word number) */
#define EFLASH_CFG_WORD7_ADDR_0         EFLASH_NVR_SECTOR1_ADDR(0xFEU)  /*!< Flash 0 Configuration Word 7 address (word number) */

#define EFLASH_ARRDN0_ADDR_0            EFLASH_NVR_SECTOR1_ADDR(0xE0U)  /*!< Flash 0 Redundancy Storage 0 address (word number) */
#define EFLASH_ARRDN1_ADDR_0            EFLASH_NVR_SECTOR1_ADDR(0xE2U)  /*!< Flash 0 Redundancy Storage 1 address (word number) */
#define EFLASH_ARRDN2_ADDR_0            EFLASH_NVR_SECTOR1_ADDR(0xE4U)  /*!< Flash 0 Redundancy Storage 2 address (word number) */
#define EFLASH_ARRDN3_ADDR_0            EFLASH_NVR_SECTOR1_ADDR(0xE6U)  /*!< Flash 0 Redundancy Storage 3 address (word number) */

/* Flash 1 */
#define EFLASH_CFG_DATA_FLAG_ADDR_1     EFLASH_NVR_SECTOR1_ADDR(0x81U)  /*!< Flash 1 Configuration Data Flag address (word number) */

#define EFLASH_CFG_WORD0_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xF1U)  /*!< Flash 1 Configuration Word 0 address (word number) */
#define EFLASH_CFG_WORD1_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xF3U)  /*!< Flash 1 Configuration Word 1 address (word number) */
#define EFLASH_CFG_WORD2_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xF5U)  /*!< Flash 1 Configuration Word 2 address (word number) */
#define EFLASH_CFG_WORD3_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xF7U)  /*!< Flash 1 Configuration Word 3 address (word number) */
#define EFLASH_CFG_WORD4_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xF9U)  /*!< Flash 1 Configuration Word 4 address (word number) */
#define EFLASH_CFG_WORD5_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xFBU)  /*!< Flash 1 Configuration Word 5 address (word number) */
#define EFLASH_CFG_WORD6_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xFDU)  /*!< Flash 1 Configuration Word 6 address (word number) */
#define EFLASH_CFG_WORD7_ADDR_1         EFLASH_NVR_SECTOR1_ADDR(0xFFU)  /*!< Flash 1 Configuration Word 7 address (word number) */

#define EFLASH_ARRDN0_ADDR_1            EFLASH_NVR_SECTOR1_ADDR(0xE0U)  /*!< Flash 1 Redundancy Storage 0 address (word number) */
#define EFLASH_ARRDN1_ADDR_1            EFLASH_NVR_SECTOR1_ADDR(0xE2U)  /*!< Flash 1 Redundancy Storage 1 address (word number) */
#define EFLASH_ARRDN2_ADDR_1            EFLASH_NVR_SECTOR1_ADDR(0xE4U)  /*!< Flash 1 Redundancy Storage 2 address (word number) */
#define EFLASH_ARRDN3_ADDR_1            EFLASH_NVR_SECTOR1_ADDR(0xE6U)  /*!< Flash 1 Redundancy Storage 3 address (word number) */

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

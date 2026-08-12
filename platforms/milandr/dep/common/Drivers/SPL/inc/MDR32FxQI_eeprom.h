/**
  ******************************************************************************
  * @file    MDR32FxQI_eeprom.h
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    09/12/2024
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          firmware library.
  ******************************************************************************
  * <br><br>
  *
  * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
  * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDR32FxQI_EEPROM_H
#define __MDR32FxQI_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup EEPROM
  * @{
  */

/** @defgroup EEPROM_Exported_Types EEPROM Exported Types
  * @{
  */

/**
  * @brief EEPROM Latency Cycles
  */
typedef enum
{
    EEPROM_Latency_0 = ((uint32_t)0x00), /*!< EEPROM 0 Latency cycle  (MCU Clock up to  25 MHz) */
    EEPROM_Latency_1 = ((uint32_t)0x08), /*!< EEPROM 1 Latency cycle  (MCU Clock up to  50 MHz) */
    EEPROM_Latency_2 = ((uint32_t)0x10), /*!< EEPROM 2 Latency cycles (MCU Clock up to  75 MHz) */
    EEPROM_Latency_3 = ((uint32_t)0x18), /*!< EEPROM 3 Latency cycles (MCU Clock up to 100 MHz) */
    EEPROM_Latency_4 = ((uint32_t)0x20), /*!< EEPROM 4 Latency cycles (MCU Clock up to 125 MHz) */
    EEPROM_Latency_5 = ((uint32_t)0x28), /*!< EEPROM 5 Latency cycles (MCU Clock up to 150 MHz) */
    EEPROM_Latency_6 = ((uint32_t)0x30), /*!< EEPROM 6 Latency cycles (MCU Clock up to 175 MHz) */
    EEPROM_Latency_7 = ((uint32_t)0x38)  /*!< EEPROM 7 Latency cycles (MCU Clock up to 200 MHz) */
} EEPROM_Latency_Cycles;

#define EEPROM_Latency_MSK                ((uint32_t)0x38)

#define IS_EEPROM_LATENCY(EEPROM_Latency) (((EEPROM_Latency) & (~EEPROM_Latency_MSK)) == 0x0)

/**
  * @brief EEPROM Bank Selector
  */
typedef enum
{
    EEPROM_Main_Bank_Select = ((uint32_t)0x00), /*!< EEPROM Main Bank Selector */
    EEPROM_Info_Bank_Select = ((uint32_t)0x01), /*!< EEPROM Information Bank Selector */
    EEPROM_All_Banks_Select = ((uint32_t)0x02)  /*!< EEPROM Main and Information Banks Selector (for EEPROM_EraseAllPages()) */
} EEPROM_Mem_Bank;

#define IS_EEPROM_BANK_SELECTOR(SEL)    (((SEL) == EEPROM_Main_Bank_Select) || \
                                         ((SEL) == EEPROM_Info_Bank_Select))

#define IS_EEPROM_ERASE_SELECTOR(SEL)   (((SEL) == EEPROM_Main_Bank_Select) || \
                                         ((SEL) == EEPROM_All_Banks_Select))

#if defined (USE_MDR32FG16S1QI)
/**
  * @brief EEPROM Cache Selection
  */
typedef enum
{
    EEPROM_Cache_None  = ((uint32_t)0x00), /*!< EEPROM Cache is not used */
    EEPROM_Cache_Instr = ((uint32_t)0x01), /*!< EEPROM Instructions Cache */
    EEPROM_Cache_Data  = ((uint32_t)0x02), /*!< EEPROM Data Cache */
    EEPROM_Cache_All   = ((uint32_t)0x03)  /*!< EEPROM Instructions Cache and Data Cache */
} EEPROM_Cache_Type;

#define EEPROM_Cache_Msk        ((uint32_t)0x03)

#define IS_EEPROM_CACHE(SEL)    (((SEL) == EEPROM_Cache_None)  || \
                                 ((SEL) == EEPROM_Cache_Instr) || \
                                 ((SEL) == EEPROM_Cache_Data)  || \
                                 ((SEL) == EEPROM_Cache_All))

#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group EEPROM_Exported_Types */

/** @defgroup EEPROM_Exported_Defines EEPROM Exported Defines
  * @{
  */

/**
  * @brief EEPROM parameters with a safe margin of 25% (HSI spread)
  */
#define EEPROM_TNVS_US   (7)
#define EEPROM_TNVH_US   (7)
#define EEPROM_TNVH1_US  (125)
#define EEPROM_TPGS_US   (13)
#define EEPROM_TRCV_US   (2)
#define EEPROM_THV_US    (5000)

#define EEPROM_TPROG_US  (25)
#define EEPROM_TERASE_US (25000)
#define EEPROM_TME_US    (25000)

/** @} */ /* End of group EEPROM_Exported_Defines */

/** @defgroup EEPROM_Exported_Functions EEPROM Exported Functions
  * @{
  */

/** @defgroup EEPROM_Functions_allowed_to_be_located_in_Flash Functions allowed to be located in Flash
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
    void EEPROM_SetCacheState(EEPROM_Cache_Type EEPROM_Cache);
    EEPROM_Cache_Type EEPROM_GetCacheState(void);
#endif /* #if defined (USE_MDR32FG16S1QI) */

void EEPROM_SetLatency(EEPROM_Latency_Cycles EEPROM_Latency);
EEPROM_Latency_Cycles EEPROM_GetLatency(void);

/** @} */ /* End of group EEPROM_Functions_allowed_to_be_located_in_Flash */

/** @defgroup EEPROM_Functions_not_allowed_to_be_located_in_Flash Functions not allowed to be located in Flash
  * @{
  */

__RAMFUNC uint8_t EEPROM_ReadByte(uint32_t Address, EEPROM_Mem_Bank BankSelector);
__RAMFUNC uint16_t EEPROM_ReadHalfWord(uint32_t Address, EEPROM_Mem_Bank BankSelector);
__RAMFUNC uint32_t EEPROM_ReadWord(uint32_t Address, EEPROM_Mem_Bank BankSelector);
__RAMFUNC void EEPROM_ReadWordArrayBurst(const uint32_t* PtrAddressArray, uint32_t* PtrDataArray, uint32_t ArraySize, EEPROM_Mem_Bank BankSelector);

__RAMFUNC void EEPROM_ErasePage(uint32_t Address, EEPROM_Mem_Bank BankSelector);
__RAMFUNC void EEPROM_EraseAllPages(EEPROM_Mem_Bank BankSelector);

__RAMFUNC void EEPROM_ProgramByte(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data);
__RAMFUNC void EEPROM_ProgramHalfWord(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data);
__RAMFUNC void EEPROM_ProgramWord(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data);
__RAMFUNC void EEPROM_ProgramWordArrayBurst(const uint32_t* PtrAddressArray, const uint32_t* PtrDataArray, uint32_t ArraySize, EEPROM_Mem_Bank BankSelector);

__RAMFUNC void EEPROM_UpdateDCache(void);

/** @} */ /* End of group EEPROM_Functions_not_allowed_to_be_located_in_Flash */

/** @} */ /* End of group EEPROM_Exported_Functions */

/** @} */ /* End of group EEPROM */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_EEPROM_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_eeprom.h */



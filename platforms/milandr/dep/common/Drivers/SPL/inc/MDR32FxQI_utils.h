/**
  ******************************************************************************
  * @file    MDR32FxQI_utils.h
  * @author  Milandr Application Team
  * @version V1.1.1i
  * @date    09/12/2024
  * @brief   This file contains all prototypes of the firmware library
  *          utility functions.
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
#ifndef __MDR32FxQI_UTILS_H
#define __MDR32FxQI_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup UTILS UTILS
  * @{
  */

/** @defgroup UTILS_Exported_Types UTILS Exported Types
  * @{
  */

/** @defgroup UTILS_DELAY_Exported_Types UTILS DELAY
  * @{
  */

/**
  * @brief DELAY Mode
  */
typedef enum {
    DELAY_MODE_PROGRAM = ((uint32_t)0x0), /*!< Selects the delay module mode based on the program cycle. */
    DELAY_MODE_SYSTICK = ((uint32_t)0x1), /*!< Selects the delay module mode based on the System Tick Timer (SysTick). */
    DELAY_MODE_TIMER1  = ((uint32_t)0x2), /*!< Selects the delay module mode based on the TIMER1. */
    DELAY_MODE_TIMER2  = ((uint32_t)0x3), /*!< Selects the delay module mode based on the TIMER2. */
    DELAY_MODE_TIMER3  = ((uint32_t)0x4), /*!< Selects the delay module mode based on the TIMER3. */
#if (defined (USE_K1986VE1xI))
    DELAY_MODE_TIMER4  = ((uint32_t)0x5)  /*!< Selects the delay module mode based on the TIMER4. */
#elif ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
    DELAY_MODE_DWT     = ((uint32_t)0x5)  /*!< Selects the delay module mode based on the DWT clock cycle counter (CYCCNT). */
#endif
} DELAY_Mode_TypeDef;

#if (defined (USE_K1986VE1xI))
#define IS_DELAY_MODE(MODE) (((MODE) == DELAY_MODE_PROGRAM) || \
                             ((MODE) == DELAY_MODE_SYSTICK) || \
                             ((MODE) == DELAY_MODE_TIMER1) || \
                             ((MODE) == DELAY_MODE_TIMER2) || \
                             ((MODE) == DELAY_MODE_TIMER3) || \
                             ((MODE) == DELAY_MODE_TIMER4))
#elif ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
#define IS_DELAY_MODE(MODE) (((MODE) == DELAY_MODE_PROGRAM) || \
                             ((MODE) == DELAY_MODE_SYSTICK) || \
                             ((MODE) == DELAY_MODE_TIMER1) || \
                             ((MODE) == DELAY_MODE_TIMER2) || \
                             ((MODE) == DELAY_MODE_TIMER3) || \
                             ((MODE) == DELAY_MODE_DWT))
#endif

/** @} */ /* End of group UTILS_DELAY_Exported_Types */

/** @} */ /* End of group UTILS_Exported_Types */

/** @defgroup UTILS_Exported_Defines UTILS Exported Defines
  * @{
  */

/** @defgroup UTILS_DELAY_Exported_Defines UTILS DELAY
  * @{
  */

/** @defgroup UTILS_DELAY_Program_Mode_Defines UTILS DELAY Program Mode Defines
  * @{
  */

#if defined (USE_K1986VE9xI)
    #define DELAY_PROGRAM_LOOP_CYCLES   (6UL)
#elif defined (USE_MDR32FG16S1QI)
    #define DELAY_PROGRAM_LOOP_CYCLES   (4UL)
#elif defined (USE_K1986VE1xI)
    #define DELAY_PROGRAM_LOOP_CYCLES   (8UL)
#endif

#define DELAY_PROGRAM_GET_CONST_US(CLK)      ((((CLK) + DELAY_PROGRAM_LOOP_CYCLES - 1) / DELAY_PROGRAM_LOOP_CYCLES) / (1000000UL / 1024UL))
#define DELAY_PROGRAM_GET_CONST_MS(CLK)      (((((CLK) + DELAY_PROGRAM_LOOP_CYCLES - 1) / DELAY_PROGRAM_LOOP_CYCLES) + 999UL) / 1000UL)

#define DELAY_PROGRAM_GET_US_LOOPS(N, CONST) (((N) * (CONST)) / 1024UL)
#define DELAY_PROGRAM_GET_MS_LOOPS(N, CONST) ((N) * (CONST))

/** @} */ /* End of group UTILS_DELAY_Program_Mode_Defines */

/** @defgroup UTILS_DELAY_Hardware_Counter_Mode_Defines UTILS DELAY Hardware Counter Mode Defines
  * @{
  */

#define DELAY_COUNTER_GET_CONST_US(CLK)   (((CLK) + 999999UL) / 1000000UL)
#define DELAY_COUNTER_GET_CONST_MS(CLK)   (((CLK) + 999UL) / 1000UL)

#define DELAY_COUNTER_GET_TICKS(N, CONST) (((N) * (CONST)))

/** @} */ /* End of group UTILS_DELAY_Hardware_Counter_Mode_Defines */

/** @} */ /* End of group UTILS_DELAY_Exported_Defines */

/** @} */ /* End of group UTILS_Exported_Defines */

/** @addtogroup UTILS_Exported_Functions UTILS Exported Functions
  * @{
  */

/** @addtogroup UTILS_DELAY_Exported_Functions UTILS DELAY
  * @{
  */

void DELAY_Init(DELAY_Mode_TypeDef Mode);
void DELAY_WaitUs(uint32_t Us);
void DELAY_WaitMs(uint32_t Ms);

__RAMFUNC void DELAY_PROGRAM_WaitLoopsAsm(uint32_t Loops);

/** @} */ /* End of group UTILS_DELAY_Exported_Functions */

/** @addtogroup UTILS_STDIO_Exported_Functions UTILS STDIO
  * @{
  */

#if defined (_USE_DEBUG_UART_)
ErrorStatus STDIO_Init(void);
#endif

/** @} */ /* End of group UTILS_STDIO_Exported_Functions */

/** @} */ /* End of group UTILS_Exported_Functions */

/** @} */ /* End of group UTILS */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_UTILS_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_utils.h */


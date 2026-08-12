/**
 * *****************************************************************************
 *  @file       bmcu_wdt.c
 *  @author     Baikal electronics SDK team
 *  @brief      Watchdog Timer (WDT) module driver
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

/* Includes ------------------------------------------------------------------*/
#include "bmcu_wdt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define IS_WDT_STATE(__VALUE__)       ((__VALUE__) == WDT_DISABLE || \
                                       (__VALUE__) == WDT_ENABLE)

#define IS_WDT_MODE(__VALUE__)        ((__VALUE__) == WDT_MODE_RESET || \
                                       (__VALUE__) == WDT_MODE_INTERRUPT)

#define IS_WDT_RST_LEN(__VALUE__)     ((__VALUE__) == WDT_RST_LEN_2   || \
                                       (__VALUE__) == WDT_RST_LEN_4   || \
                                       (__VALUE__) == WDT_RST_LEN_8   || \
                                       (__VALUE__) == WDT_RST_LEN_16  || \
                                       (__VALUE__) == WDT_RST_LEN_32  || \
                                       (__VALUE__) == WDT_RST_LEN_64  || \
                                       (__VALUE__) == WDT_RST_LEN_128 || \
                                       (__VALUE__) == WDT_RST_LEN_256)

#define IS_WDT_TIME_PERIOD(__VALUE__) ((__VALUE__) == WDT_TIME_PERIOD_0  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_1  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_2  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_3  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_4  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_5  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_6  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_7  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_8  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_9  || \
                                       (__VALUE__) == WDT_TIME_PERIOD_10 || \
                                       (__VALUE__) == WDT_TIME_PERIOD_11 || \
                                       (__VALUE__) == WDT_TIME_PERIOD_12 || \
                                       (__VALUE__) == WDT_TIME_PERIOD_13 || \
                                       (__VALUE__) == WDT_TIME_PERIOD_14 || \
                                       (__VALUE__) == WDT_TIME_PERIOD_15)

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Deinitialize a WDT instance
 * @note    When WDT_CR_WDT_EN bit in WDT_CR register has been enabled,
 *          it can be cleared only by a system reset
 * @retval  An ErrorStatus enumeration value:
 *          - SUCCESS: The WDT instance have been deinitialized
 *          - ERROR: The WDT instance have not been deinitialized
 * @see     WDT_Init()
 */
ErrorStatus
WDT_DeInit (WDT_TypeDef *WDTx) {
    ErrorStatus status = SUCCESS;

    assert(IS_WDT_ALL_INSTANCE(WDTx));

    /* Reload WDT counter */
    WDT_ReloadCounter(WDTx);

#if defined (BMCU_U)
    if ((WDTx == WDT0) || (WDTx == WDT1))
    {
        WRITE_REG(CRU->WDTCLRKEY, WDT_KEY_RESET);
        SET_BIT(CRU->SYSCR0, CRU_SYSCR0_WDTCLR);
        CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_WDTCLR);        
    }
    else
    {
        status = ERROR;
    }
#endif

#if defined (BE_U1000)
    if (WDTx == WDT0)
    {
        WRITE_REG(CRU->WDTCLRKEY, WDT_KEY_RESET);
        CLEAR_BIT(CRU->PCLK0EN, CRU_PCLK0EN_WDT0RSTN);
        SET_BIT(CRU->PCLK0EN, CRU_PCLK0EN_WDT0RSTN);
    }
    else if (WDTx == WDT1)
    {
        WRITE_REG(CRU->WDTCLRKEY, WDT_KEY_RESET);
        CLEAR_BIT(CRU->PCLK1EN, CRU_PCLK1EN_WDT1RSTN);
        SET_BIT(CRU->PCLK1EN, CRU_PCLK1EN_WDT1RSTN);
    }
    else
    {
        status = ERROR;
    }
#endif

    return status;
}

/**
 * @brief   Initialize a WDT instance
 * @note    WDT_InitStruct structure should be initialized prior to calling this
 *          function
 * @param   WDTx The WDT instance
 * @param   WDT_InitStruct The pointer to WDT_InitStruct structure
 * @retval  An ErrorStatus enumeration value:
 *          - SUCCESS: The WDT instance have been initialized
 *          - ERROR: The WDT instance have not been initialized
 * @see     WDT_DeInit()
 */
ErrorStatus
WDT_Init (WDT_TypeDef *WDTx, WDT_InitStruct_TypeDef *WDT_InitStruct) {
    ErrorStatus status = SUCCESS;

    assert(IS_WDT_ALL_INSTANCE(WDTx));
    assert(IS_WDT_MODE(WDT_InitStruct->Mode));
    assert(IS_WDT_RST_LEN(WDT_InitStruct->Rstlen));
    assert(IS_WDT_TIME_PERIOD(WDT_InitStruct->Timeout));

    /* Configure WDT output response */
    WDT_SetMode(WDTx, WDT_InitStruct->Mode);

    /* Configure the reset pulse length in case of system reset */
    WDT_SetRstLen(WDTx, WDT_InitStruct->Rstlen);

    /* Configure the timeout period to be set at each WDT restart */
    WDT_SetReloadCounter(WDTx, WDT_InitStruct->Timeout);

    /* Clear the WDT interrupt */
    WDT_ClearIT(WDTx);

    /* Enable WDT */
    WDT_Enable(WDTx);

    /* WDT counter refrash */
    WDT_ReloadCounter(WDTx);

    return status;
}

/**
 * @brief Set each field of WDT_InitStruct_TypeDef structure to default value
 * @param WDT_InitStruct The pointer to WDT_InitStruct_TypeDef structure
 */
void
WDT_StructInit (WDT_InitStruct_TypeDef *WDT_InitStruct) {
    /* Set WDT_InitStruct fields to default values */
    WDT_InitStruct->Mode    = WDT_MODE_RESET;
    WDT_InitStruct->Rstlen  = WDT_RST_LEN_32;
    WDT_InitStruct->Timeout = WDT_TIME_PERIOD_4;
}

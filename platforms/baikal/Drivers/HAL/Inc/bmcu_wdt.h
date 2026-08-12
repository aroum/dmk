/**
 * *****************************************************************************
 *  @file       bmcu_wdt.h
 *  @author     Baikal electronics SDK team
 *  @brief      Watchdog Timer (WDT) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *  @details
 *  Programmable Watchdog Timer (WDT) is purposed to protect the user
 *  application from incorrect behavior in case of software or hardware error.
 *  The preconfigured WDT counter counts down from user defined timeoutvalue to
 *  0 to indicate the occurrence of a timeout.
 *  If a timeout occurs (i.e. the WDT counter reaches zero) the WDT can perform
 *  one of the following operations:
 *      - generate a system reset
 *      - first generate an interrupt and even if it is cleared (or not) by the
 *        service routine by the time a second timeout occurs then generate a
 *        system reset
 *  If WDT module is enabled, user application should periodicaly re-set WDT
 *  counter value (refresh).
 *
 *  Features of WDT:
 *      - Programmable timout range
 *      - Programmable reset pulse length
 *      - Prevention of accidental restart of WDT counter
 *      - Prevention of accidental disabling of WDT
 *
 *  File content:
 *      - WDT related type definitions and enumerations
 *      - Macros for WDT control
 *      - WDT module export functions prototypes
 *
 *  Example: configure WDT0 timeout as 0x1FFFFFF clock cycles and enable WDT0

 *          ... application initialization code ...
 *          /// WDT configuration struct variable
 *          WDT_InitStruct_TypeDef WDT_Init;
 *          /// Initialize configuration struct with default values
 *          WDT_StructInit(&WDT_Init);
 *          /// Configure WDT accordint to WDT_Init structure values
 *          WDT_Init(WDT0, &WDT_Init);
 *          /// Set WDT counter reload value
 *          WDT_SetReloadCounter(WDT0, WDT_TIME_PERIOD_13);
 *          /// Enable WDT0. WDT counter starts to count down from 0x1FFFFFF
 *          WDT_Enable(WDT0);
 *
 *          ... application code ...
 *          /// WDT counter should be restarted periodically
 *          WDT_ReloadCounter(WDT0);
 *
 *  WDT module state after system reset:
 *      - State: disabled (WDT_CR_WDT_EN_DISABLED)
 *      - Response mode: generate system reset (WDT_MODE_RESET)
 *      - Reset pulse length: 32 clock cycles (WDT_RST_LEN_32)
 *      - Timeout period: 0xFFFF clock cycles (WDT_TIME_PERIOD_4)
 *
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMCU_WDT_H
#define __BMCU_WDT_H

/* Includes ------------------------------------------------------------------*/
#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#define WDT_KEY_RELOAD (0x76U)         /*!< Key value to recharge WDT counter */
#define WDT_KEY_RESET  (0xD09C1EA7UL)  /*!< Key value to reset the WDT module */

/* Exported types ------------------------------------------------------------*/

/**
 * @typedef
 * @brief   Response mode types enumeration
 * @note    Selects the output response generated to a timeout.
 * @see     WDT_SetMode(), WDT_GetMode()
 */
typedef enum {
    WDT_MODE_RESET     = WDT_CR_RMOD_RESET,
    WDT_MODE_INTERRUPT = WDT_CR_RMOD_INTERRUPT
} WDT_Mode_TypeDef;

/**
 * @typedef
 * @brief   Reset pulse length alowed values enumeration
 * @see     WDT_SetRstLen(), WDT_GetRstLen()
 */

typedef enum {
    WDT_RST_LEN_2   = WDT_CR_RPL_CYCLES_2,
    WDT_RST_LEN_4   = WDT_CR_RPL_CYCLES_4,
    WDT_RST_LEN_8   = WDT_CR_RPL_CYCLES_8,
    WDT_RST_LEN_16  = WDT_CR_RPL_CYCLES_16,
    WDT_RST_LEN_32  = WDT_CR_RPL_CYCLES_32,
    WDT_RST_LEN_64  = WDT_CR_RPL_CYCLES_64,
    WDT_RST_LEN_128 = WDT_CR_RPL_CYCLES_128,
    WDT_RST_LEN_256 = WDT_CR_RPL_CYCLES_256
} WDT_RstLen_TypeDef;

/**
 * @typedef
 * @brief   Timeout period (cycles) allowed values enumeration
 * @note    This enumeration is used to select the timeout period from which
 *          thew atchdog counter restarts. A change of the timeout period takes
 *          effect only after the next counter restart (kick).
 * @see     WDT_SetReloadCounter(), WDT_GetReloadCounter()
 */
typedef enum {
    WDT_TIME_PERIOD_0  = WDT_TORR_TOP_USER_0,   /*!< 0xFF      */
    WDT_TIME_PERIOD_1  = WDT_TORR_TOP_USER_1,   /*!< 0x1FF     */
    WDT_TIME_PERIOD_2  = WDT_TORR_TOP_USER_2,   /*!< 0x3FF     */
    WDT_TIME_PERIOD_3  = WDT_TORR_TOP_USER_3,   /*!< 0x7FF     */
    WDT_TIME_PERIOD_4  = WDT_TORR_TOP_USER_4,   /*!< 0xFFFF    */
    WDT_TIME_PERIOD_5  = WDT_TORR_TOP_USER_5,   /*!< 0x1FFFF   */
    WDT_TIME_PERIOD_6  = WDT_TORR_TOP_USER_6,   /*!< 0x3FFFF   */
    WDT_TIME_PERIOD_7  = WDT_TORR_TOP_USER_7,   /*!< 0x7FFFF   */
    WDT_TIME_PERIOD_8  = WDT_TORR_TOP_USER_8,   /*!< 0xFFFFF   */
    WDT_TIME_PERIOD_9  = WDT_TORR_TOP_USER_9,   /*!< 0x1FFFFF  */
    WDT_TIME_PERIOD_10 = WDT_TORR_TOP_USER_10,  /*!< 0x3FFFFF  */
    WDT_TIME_PERIOD_11 = WDT_TORR_TOP_USER_11,  /*!< 0x7FFFFF  */
    WDT_TIME_PERIOD_12 = WDT_TORR_TOP_USER_12,  /*!< 0xFFFFFF  */
    WDT_TIME_PERIOD_13 = WDT_TORR_TOP_USER_13,  /*!< 0x1FFFFFF */
    WDT_TIME_PERIOD_14 = WDT_TORR_TOP_USER_14,  /*!< 0x3FFFFFF */
    WDT_TIME_PERIOD_15 = WDT_TORR_TOP_USER_15   /*!< 0x7FFFFFF */
} WDT_TimeoutPeriod_TypeDef;

/** @brief   WDT Init Structure definition */
typedef struct {
    WDT_Mode_TypeDef          Mode;           /*!< Response mode */
    WDT_RstLen_TypeDef        Rstlen;         /*!< Reset pulse length */
    WDT_TimeoutPeriod_TypeDef Timeout;        /*!< Timeout period */
} WDT_InitStruct_TypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief   Enable WDT
 * @param   WDTx The WDT instance
 * @note    When disabled, the WDT counter does not decrement.
 *          Thus, no interrupts or system resets are generated.
 *          To prevent a software bug from disabling the DW_apb_wdt, once this
 *          bit has been enabled, it can be cleared only by a system reset.
 *          WDT is disable after reset
 * @retval  None
 */
__STATIC_INLINE void
WDT_Enable (WDT_TypeDef *WDTx) {
    SET_BIT(WDTx->CR, WDT_CR_WDT_EN);
}

/**
 * @brief   Check if WDT is enabled
 * @param   WDTx The WDT instance
 * @retval  1 if WDT is enabled, 0 otherwise
 */
__STATIC_INLINE uint8_t
WDT_IsEnabled (WDT_TypeDef *WDTx) {
    return (READ_BIT(WDTx->CR, WDT_CR_WDT_EN) != 0x0UL) ? 1U : 0U;
}

/**
 * @brief   Reload WDT counter
 * @note    As a safety feature to prevent accidental restarts, the
 *          value WDT_KEY_RELOAD must be written.
 *          A restart also clears the WDT interrupt
 * @param   WDTx The WDT instance
 * @retval  None
 */
__STATIC_INLINE void
WDT_ReloadCounter (WDT_TypeDef *WDTx) {
    WRITE_REG(WDTx->CRR, WDT_CRR & (WDT_KEY_RELOAD << WDT_CRR_Pos));
}

/**
 * @brief   Selects the output response generated to a WDT timeout.
 * @note    When WDT counter reaches zero, either generate a system reset or
 *          first generate an interrupt and even if it is cleared by the time
 *          a second timeout occurs then generate a system reset
 *          Value after reset: WDT_MODE_RESET
 * @param   WDTx The WDT instance
 * @param   mode Response mode selection
 * @retval  None
 * @see     WDT_GetMode()
 */
__STATIC_INLINE void
WDT_SetMode (WDT_TypeDef *WDTx, WDT_Mode_TypeDef Mode) {
    MODIFY_REG(WDTx->CR, WDT_CR_RMOD, Mode);
}

/**
 * @brief   Get the output response generated to a WDT timeout.
 * @param   WDTx The WDT instance
 * @retval  Response mode selection
 * @see     WDT_SetMode()
 */
__STATIC_INLINE WDT_Mode_TypeDef
WDT_GetMode (WDT_TypeDef *WDTx) {
    return (READ_BIT(WDTx->CR, WDT_CR_RMOD) != 0x0UL) ? WDT_MODE_RESET : WDT_MODE_INTERRUPT;
}

/**
 * @brief   Configure the reset pulse length in clock cycles
 * @note    Due to counter clock and the peripheral clock asynchronous work,
 *          the total reset pulse length also includes the reset synchronization
 *          delay and the time taken for peripheral clock to be made available.
 *          Value after reset: WDT_RST_LEN_32
 * @param   WDTx The WDT instance
 * @param   rstlen  Reset pulse length value
 * @retval  None
 * @see     WDT_GetRstLen()
 */
__STATIC_INLINE void
WDT_SetRstLen (WDT_TypeDef *WDTx, WDT_RstLen_TypeDef Rstlen) {
    MODIFY_REG(WDTx->CR, WDT_CR_RPL, Rstlen);
}

/**
 * @brief   Get the reset pulse length value
 * @param   WDTx The WDT instance
 * @retval  Reset pulse length value
 * @see     WDT_SetRstLen()
 */
__STATIC_INLINE WDT_RstLen_TypeDef
WDT_GetRstLen (WDT_TypeDef *WDTx) {
    return (WDT_RstLen_TypeDef)READ_BIT(WDTx->CR, WDT_CR_RPL);
}

/**
 * @brief   Select the timeout period from which the watchdog counter restarts
 * @note    A change of the timeout period takes effect only after the next
 *          counter restart (kick).
 *          Value after reset: WDT_TIME_PERIOD_4
 * @param   WDTx The WDT instance
 * @retval  None
 * @see     WDT_GetReloadCounter()
 */
__STATIC_INLINE void
WDT_SetReloadCounter (WDT_TypeDef *WDTx, WDT_TimeoutPeriod_TypeDef Timeout) {
    MODIFY_REG(WDTx->TORR, WDT_TORR_TOP, Timeout);
}

/**
 * @brief   Get the current value of timeout perion from which the watchdog
 *          counter restarts
 * @param   WDTx The WDT instance
 * @retval  Timeout perion value
 * @see     WDT_SetReloadCounter()
 */
__STATIC_INLINE WDT_TimeoutPeriod_TypeDef
WDT_GetReloadCounter (WDT_TypeDef *WDTx) {
    return (WDT_TimeoutPeriod_TypeDef)READ_BIT(WDTx->TORR, WDT_TORR_TOP);
}

/**
 * @brief   Get the current value of the WDT internal counter
 * @param   WDTx The WDT instance
 * @retval  Current value of the WDT internal counter
 */
__STATIC_INLINE uint32_t
WDT_GetCounter (WDT_TypeDef *WDTx) {
    return READ_REG(WDTx->CCVR);
}

/**
 * @brief   Get the WDT interrupt status
 * @param   WDTx The WDT instance
 * @retval  1 if WDT interrupt is active regardless of polarity,
 *          0 if interrupt is inactive
 * @see     WDT_ClearIT()
 */
__STATIC_INLINE uint32_t
WDT_IsActiveIT (WDT_TypeDef *WDTx) {
    return (READ_BIT(WDTx->STAT, WDT_STAT) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief   Clear the WDT interrupt
 * @note    This can be used to clear the interrupt without restarting
 *          the watchdog counter
 * @param   WDTx The WDT instance
 * @retval  None
 * @see     WDT_IsActiveIT()
 */
__STATIC_INLINE void
WDT_ClearIT (WDT_TypeDef *WDTx) {
    (void)READ_BIT(WDTx->EOI, WDT_EOI);
}

ErrorStatus WDT_DeInit(WDT_TypeDef *WDTx);
ErrorStatus WDT_Init(WDT_TypeDef *WDTx, WDT_InitStruct_TypeDef *WDT_InitStruct);
void WDT_StructInit(WDT_InitStruct_TypeDef *WDT_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_WDT_H */

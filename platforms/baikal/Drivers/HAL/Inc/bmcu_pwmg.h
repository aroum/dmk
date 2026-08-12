/**
 * *****************************************************************************
 *  @file       bmcu_pwmg.h
 *  @author     Baikal electronics SDK team
 *  @brief      PWMG module driver header file
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

#ifndef __BMCU_PWMG_H
#define __BMCU_PWMG_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Counter one pulse mode */
typedef enum {
    PWMG_ONEPULSEMODE_REPETITIVE = PWMG_CR1_OPM_REPETITIVE, /*!< Counter is not stopped at update event */
    PWMG_ONEPULSEMODE_SINGLE     = PWMG_CR1_OPM_SINGLE      /*!< Counter stops counting at the next update event */
} PWMG_OnePulseMode_TypeDef;

/** @brief Counter clock division */
typedef enum {
    PWMG_CLOCKDIVISION_DIV1 = PWMG_CR1_CKD_DIV1,    /*!< Clock is not divided */
    PWMG_CLOCKDIVISION_DIV2 = PWMG_CR1_CKD_DIV2,    /*!< Clock is divided by 2 */
    PWMG_CLOCKDIVISION_DIV4 = PWMG_CR1_CKD_DIV4     /*!< Clock is divided by 4 */
} PWMG_ClockDivision_TypeDef;

/** @brief Output Compare mode */
typedef enum {
    PWMG_OC_MODE_FROZEN          = PWMG_CCMR1_OC_OC0M_FROZEN,           /*!< The comparison between the output compare register CCR0 and the counter CNT has no effect on the output channel level */
    PWMG_OC_MODE_ACTIVE          = PWMG_CCMR1_OC_OC0M_ACTIVE,           /*!< OC0REF is forced high on compare match */
    PWMG_OC_MODE_INACTIVE        = PWMG_CCMR1_OC_OC0M_INACTIVE,         /*!< OC0REF is forced low on compare match */
    PWMG_OC_MODE_TOGGLE          = PWMG_CCMR1_OC_OC0M_TOGGLE,           /*!< OC0REF toggles on compare match */
    PWMG_OC_MODE_FORCED_INACTIVE = PWMG_CCMR1_OC_OC0M_FORCED_INACTIVE,  /*!< OC0REF is forced low */
    PWMG_OC_MODE_FORCED_ACTIVE   = PWMG_CCMR1_OC_OC0M_FORCED_ACTIVE,    /*!< OC0REF is forced high */
    PWMG_OC_MODE_PWM1            = PWMG_CCMR1_OC_OC0M_PWM1,             /*!< In upcounting, channel 0 is active as long as CNT <= CCR0, else inactive. In downcounting, channel 0 is inactive as long as CNT >= CCR0, else active. */
    PWMG_OC_MODE_PWM2            = PWMG_CCMR1_OC_OC0M_PWM2              /*!< In upcounting, channel 0 is inactive as long as CNT <= CCR0, else active. In downcounting, channel 0 is active as long as CNT >= CCR0, else inactive. */
} PWMG_OC_Mode_TypeDef;

/** @brief Output Compare state */
typedef enum {
    PWMG_OC_STATE_DISABLE = PWMG_CCER_CC0E_DISABLE,     /*!< OC is not active */
    PWMG_OC_STATE_ENABLE  = PWMG_CCER_CC0E_ENABLE       /*!< OC signal is output on the corresponding output pin */
} PWMG_OC_State_TypeDef;

/** @brief Output Compare polarity */
typedef enum {
    PWMG_OC_POLARITY_HIGH = PWMG_CCER_CC0P_HIGH,    /*!< Output: active high */
    PWMG_OC_POLARITY_LOW  = PWMG_CCER_CC0P_LOW      /*!< Output: active low */
} PWMG_OC_Polarity_TypeDef;

/** @brief Input Capture prescaler */
typedef enum {
    PWMG_IC_PRESCALER_DIV1 = PWMG_CCMR1_IC_IC0PSC_DIV1,     /*!< Prescaler is not used */
    PWMG_IC_PRESCALER_DIV2 = PWMG_CCMR1_IC_IC0PSC_DIV2,     /*!< Prescaler is 2 */
    PWMG_IC_PRESCALER_DIV4 = PWMG_CCMR1_IC_IC0PSC_DIV4,     /*!< Prescaler is 4 */
    PWMG_IC_PRESCALER_DIV8 = PWMG_CCMR1_IC_IC0PSC_DIV8      /*!< Prescaler is 8 */
} PWMG_IC_Prescaler_TypeDef;

/** @brief Input Capture filter */
typedef enum {
    PWMG_IC_FILTER_FDIV1     = PWMG_CCMR1_IC_IC0F_FDIV1,        /*!< No filter, sampling is done at fDTS */
    PWMG_IC_FILTER_FDIV1_N2  = PWMG_CCMR1_IC_IC0F_FDIV1_N2,     /*!< fSAMPLING = fCK_INT, N = 2 */
    PWMG_IC_FILTER_FDIV1_N4  = PWMG_CCMR1_IC_IC0F_FDIV1_N4,     /*!< fSAMPLING = fCK_INT, N = 4 */
    PWMG_IC_FILTER_FDIV1_N8  = PWMG_CCMR1_IC_IC0F_FDIV1_N8,     /*!< fSAMPLING = fCK_INT, N = 8 */
    PWMG_IC_FILTER_FDIV2_N6  = PWMG_CCMR1_IC_IC0F_FDIV2_N6,     /*!< fSAMPLING = fDTS/2, N = 6 */
    PWMG_IC_FILTER_FDIV2_N8  = PWMG_CCMR1_IC_IC0F_FDIV2_N8,     /*!< fSAMPLING = fDTS/2, N = 8 */
    PWMG_IC_FILTER_FDIV4_N6  = PWMG_CCMR1_IC_IC0F_FDIV4_N6,     /*!< fSAMPLING = fDTS/4, N = 6 */
    PWMG_IC_FILTER_FDIV4_N8  = PWMG_CCMR1_IC_IC0F_FDIV4_N8,     /*!< fSAMPLING = fDTS/4, N = 8 */
    PWMG_IC_FILTER_FDIV8_N6  = PWMG_CCMR1_IC_IC0F_FDIV8_N6,     /*!< fSAMPLING = fDTS/8, N = 6 */
    PWMG_IC_FILTER_FDIV8_N8  = PWMG_CCMR1_IC_IC0F_FDIV8_N8,     /*!< fSAMPLING = fDTS/8, N = 8 */
    PWMG_IC_FILTER_FDIV16_N5 = PWMG_CCMR1_IC_IC0F_FDIV16_N5,    /*!< fSAMPLING = fDTS/16, N = 5 */
    PWMG_IC_FILTER_FDIV16_N6 = PWMG_CCMR1_IC_IC0F_FDIV16_N6,    /*!< fSAMPLING = fDTS/16, N = 6 */
    PWMG_IC_FILTER_FDIV16_N8 = PWMG_CCMR1_IC_IC0F_FDIV16_N8,    /*!< fSAMPLING = fDTS/16, N = 8 */
    PWMG_IC_FILTER_FDIV32_N5 = PWMG_CCMR1_IC_IC0F_FDIV32_N5,    /*!< fSAMPLING = fDTS/32, N = 5 */
    PWMG_IC_FILTER_FDIV32_N6 = PWMG_CCMR1_IC_IC0F_FDIV32_N6,    /*!< fSAMPLING = fDTS/32, N = 6 */
    PWMG_IC_FILTER_FDIV32_N8 = PWMG_CCMR1_IC_IC0F_FDIV32_N8     /*!< fSAMPLING = fDTS/32, N = 8 */
} PWMG_IC_Filter_TypeDef;

/** @brief Input Capture polarity */
typedef enum {
    PWMG_IC_POLARITY_RISING  = PWMG_CCER_CC0P_NONINVERTED,  /*!< Input: not inverted/rising edge */
    PWMG_IC_POLARITY_FALLING = PWMG_CCER_CC0P_INVERTED      /*!< Input: inverted/falling edge */
} PWMG_IC_Polarity_TypeDef;

/** @brief Flags */
typedef enum {
    PWMG_FLAG_UPDATE = 0,   /*!< Update interrupt flag */
    PWMG_FLAG_CC0    = 1,   /*!< Capture/Compare 0 interrupt flag */
    PWMG_FLAG_CC0OVR = 9    /*!< Capture/Compare 0 overcapture flag */
} PWMG_Flag_TypeDef;

/** @brief Interrupts */
typedef enum {
    PWMG_IT_UPDATE = 0,     /*!< Update interrupt */
    PWMG_IT_CC0    = 1      /*!< Capture/Compare 0 interrupt */
} PWMG_IT_TypeDef;

/** @brief Events */
typedef enum {
    PWMG_EVENT_UPDATE = 0,  /*!< Update event */
    PWMG_EVENT_CC0    = 1   /*!< Capture/Compare 0 event */
} PWMG_Event_TypeDef;

/** @brief PWMG Init Structure definition */
typedef struct
{
    uint16_t Prescaler;                         /*!< Prescaler value */
    uint16_t Autoreload;                        /*!< Auto-reload value */
    PWMG_ClockDivision_TypeDef ClockDivision;   /*!< Clock division (tDTS) */
} PWMG_InitStruct_TypeDef;

/** @brief PWMG Output Compare Init Structure definition */
typedef struct
{
    PWMG_OC_Mode_TypeDef OCMode;            /*!< Output mode */
    PWMG_OC_Polarity_TypeDef OCPolarity;    /*!< Output polarity */
    PWMG_OC_State_TypeDef OCState;          /*!< Output state */
    uint16_t CompareValue;                  /*!< Capture/compare value */
} PWMG_OC_InitStruct_TypeDef;

/** @brief PWMG Input Capture Init Structure definition */
typedef struct
{
    PWMG_IC_Prescaler_TypeDef ICPrescaler;  /*!< Input Capture prescaler */
    PWMG_IC_Filter_TypeDef ICFilter;        /*!< Input Capture filter */
    PWMG_IC_Polarity_TypeDef ICPolarity;    /*!< Input Capture polarity */
} PWMG_IC_InitStruct_TypeDef;

/**
 * @brief Calculates the prescaler value to achieve the required counter clock frequency.
 * @param __APBCLK__ The timer clock frequency (in Hz).
 * @param __CNTCLK__ The counter clock frequency (in Hz).
 * @returns The prescaler value in the range of 0x0 to 0xFFFF.
 */
#define __PWMG_CALC_PSC(__APBCLK__, __CNTCLK__)         (((__APBCLK__) >= (__CNTCLK__)) ? \
                                                         (uint16_t)((((__APBCLK__) + (__CNTCLK__) / 2U) / (__CNTCLK__)) - 1U) : 0U)

/**
 * @brief Calculates the auto-reload value to achieve the required output signal frequency.
 * @param __APBCLK__ The timer clock frequency (in Hz).
 * @param __PSC__ The prescaler register value.
 * @param __FREQ__ The output signal frequency (in Hz).
 * @returns The auto-reload value in the range of 0x0 to 0xFFFF.
 */
#define __PWMG_CALC_ARR(__APBCLK__, __PSC__, __FREQ__)  ((((__APBCLK__)/((__PSC__) + 1U)) >= (__FREQ__)) ? \
                                                         (((__APBCLK__)/((__FREQ__) * ((__PSC__) + 1U))) - 1U) : 0U)

/**
 * @brief Writes a value in PWMG register.
 * @param INSTANCE The PWMG instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define PWMG_WriteReg(INSTANCE, REG, VALUE)     WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in PWMG register.
 * @param INSTANCE The PWMG instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define PWMG_ReadReg(INSTANCE, REG)             READ_REG(INSTANCE->REG)

/**
 * @brief Enables timer counter.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_EnableCounter(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->CR1, PWMG_CR1_CEN);
}

/**
 * @brief Disables timer counter.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_DisableCounter(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->CR1, PWMG_CR1_CEN);
}

/**
 * @brief Checks if the timer counter is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if the timer counter is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledCounter(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->CR1, PWMG_CR1_CEN) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables update event generation.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_EnableUpdateEvent(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->CR1, PWMG_CR1_UDIS);
}

/**
 * @brief Disables update event generation.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_DisableUpdateEvent(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->CR1, PWMG_CR1_UDIS);
}

/**
 * @brief Checks if update event generation is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if update event generation is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledUpdateEvent(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->CR1, PWMG_CR1_UDIS) == 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets one pulse mode.
 * @param PWMGx The PWMG instance.
 * @param OnePulseMode The one pulse mode. Can be one of PWMG_OnePulseMode_TypeDef values.
 */
__STATIC_INLINE void PWMG_SetOnePulseMode(PWMG_TypeDef *PWMGx, PWMG_OnePulseMode_TypeDef OnePulseMode)
{
    MODIFY_REG(PWMGx->CR1, PWMG_CR1_OPM, (uint32_t)OnePulseMode);
}

/**
 * @brief Returns the actual one pulse mode.
 * @param PWMGx The PWMG instance.
 * @returns The one pulse mode. Can be one of PWMG_OnePulseMode_TypeDef values.
 */
__STATIC_INLINE PWMG_OnePulseMode_TypeDef PWMG_GetOnePulseMode(PWMG_TypeDef *PWMGx)
{
    return (PWMG_OnePulseMode_TypeDef)READ_BIT(PWMGx->CR1, PWMG_CR1_OPM);
}

/**
 * @brief Enables auto-reload (ARR) preload.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_EnableARRPreload(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->CR1, PWMG_CR1_ARPE);
}

/**
 * @brief Disables auto-reload (ARR) preload.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_DisableARRPreload(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->CR1, PWMG_CR1_ARPE);
}

/**
 * @brief Checks if auto-reload (ARR) preload is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if auto-reload (ARR) preload is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledARRPreload(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->CR1, PWMG_CR1_ARPE) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets the division ratio between the timer clock and the sampling clock
 *        used by the digital filters.
 * @param PWMGx The PWMG instance.
 * @param ClockDivision The clock division ratio. Can be one of PWMG_ClockDivision_TypeDef values.
 */
__STATIC_INLINE void PWMG_SetClockDivision(PWMG_TypeDef *PWMGx, PWMG_ClockDivision_TypeDef ClockDivision)
{
    MODIFY_REG(PWMGx->CR1, PWMG_CR1_CKD, (uint32_t)ClockDivision);
}

/**
 * @brief Returns the division ratio between the timer clock and the sampling clock
 *        used by the digital filters.
 * @param PWMGx The PWMG instance.
 * @returns The clock division ratio. Can be one of PWMG_ClockDivision_TypeDef values.
 */
__STATIC_INLINE PWMG_ClockDivision_TypeDef PWMG_GetClockDivision(PWMG_TypeDef *PWMGx)
{
    return (PWMG_ClockDivision_TypeDef)READ_BIT(PWMGx->CR1, PWMG_CR1_CKD);
}

/**
 * @brief Sets the counter value.
 * @param PWMGx The PWMG instance.
 * @param Counter The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void PWMG_SetCounter(PWMG_TypeDef *PWMGx, uint16_t Counter)
{
    WRITE_REG(PWMGx->CNT, Counter & PWMG_CNT);
}

/**
 * @brief Returns the counter value.
 * @param PWMGx The PWMG instance.
 * @returns The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t PWMG_GetCounter(PWMG_TypeDef *PWMGx)
{
    return (uint16_t)(READ_REG(PWMGx->CNT) & PWMG_CNT);
}

/**
 * @brief Sets the prescaler value.
 * @note The counter clock frequency fCK_CNT is equal to fCK_PSC / (PSC[15:0] + 1).
 * @note The prescaler can be changed on the fly as this control register is buffered.
 *       The new prescaler ratio is taken into account at the next update event.
 * @note The macro __PWMG_CALC_PSC can be used to calculate the Prescaler parameter.
 * @param PWMGx The PWMG instance.
 * @param Prescaler The prescaler value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void PWMG_SetPrescaler(PWMG_TypeDef *PWMGx, uint16_t Prescaler)
{
    WRITE_REG(PWMGx->PSC, Prescaler & PWMG_PSC);
}

/**
 * @brief Returns the prescaler value.
 * @param PWMGx The PWMG instance.
 * @returns The prescaler value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t PWMG_GetPrescaler(PWMG_TypeDef *PWMGx)
{
    return (uint16_t)(READ_REG(PWMGx->PSC) & PWMG_PSC);
}

/**
 * @brief Sets the auto-reload value.
 * @note The counter is stopped while the auto-reload value is null.
 * @note The macro __PWMG_CALC_ARR can be used to calculate the AutoReload parameter.
 * @param PWMGx The PWMG instance.
 * @param AutoReload The auto-reload value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void PWMG_SetAutoReload(PWMG_TypeDef *PWMGx, uint16_t AutoReload)
{
    WRITE_REG(PWMGx->ARR, AutoReload & PWMG_ARR);
}

/**
 * @brief Returns the auto-reload value.
 * @param PWMGx The PWMG instance.
 * @returns The auto-reload value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t PWMG_GetAutoReload(PWMG_TypeDef *PWMGx)
{
    return (uint16_t)(READ_REG(PWMGx->ARR) & PWMG_ARR);
}

/**
 * @brief Enables capture/compare channel.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_CC_EnableChannel(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->CCER, PWMG_CCER_CC0E);
}

/**
 * @brief Disables capture/compare channel.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_CC_DisableChannel(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->CCER, PWMG_CCER_CC0E);
}

/**
 * @brief Checks if capture/compare channel is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if the channel is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_CC_IsEnabledChannel(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->CCER, PWMG_CCER_CC0E) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures the output channel.
 * @note The function configures the channel as output and sets its parameters.
 * @param PWMGx The PWMG instance.
 * @param PWMG_OC_InitStruct The pointer to PWMG_OC_InitStruct_TypeDef structure.
 *                           The next parameters will be configured:
 *                           - OCMode
 *                           - OCPolarity
 */
__STATIC_INLINE void PWMG_OC_Config(PWMG_TypeDef *PWMGx, PWMG_OC_InitStruct_TypeDef *PWMG_OC_InitStruct)
{
    MODIFY_REG(PWMGx->CCMR1,
               (PWMG_CCMR1_OC_OC0M | PWMG_CCMR1_CC0S),
               ((uint32_t)PWMG_OC_InitStruct->OCMode | PWMG_CCMR1_CC0S_OC));

    MODIFY_REG(PWMGx->CCER, PWMG_CCER_CC0P, (uint32_t)PWMG_OC_InitStruct->OCPolarity);
}

/**
 * @brief Defines the behavior of the output reference signal OC0REF
 *        from which OC0 is derived.
 * @param PWMGx The PWMG instance.
 * @param Mode The output compare mode. Can be one of PWMG_OC_Mode_TypeDef values.
 */
__STATIC_INLINE void PWMG_OC_SetMode(PWMG_TypeDef *PWMGx, PWMG_OC_Mode_TypeDef Mode)
{
    MODIFY_REG(PWMGx->CCMR1, (PWMG_CCMR1_OC_OC0M | PWMG_CCMR1_CC0S), (uint32_t)Mode | PWMG_CCMR1_CC0S_OC);
}

/**
 * @brief Returns the output compare mode of the output channel.
 * @param PWMGx The PWMG instance.
 * @returns The output compare mode. Can be one of PWMG_OC_Mode_TypeDef values.
 */
__STATIC_INLINE PWMG_OC_Mode_TypeDef PWMG_OC_GetMode(PWMG_TypeDef *PWMGx)
{
    return (PWMG_OC_Mode_TypeDef)READ_BIT(PWMGx->CCMR1, PWMG_CCMR1_OC_OC0M);
}

/**
 * @brief Sets the output polarity.
 * @param PWMGx The PWMG instance.
 * @param Polarity The output polarity. Can be one of PWMG_OC_Polarity_TypeDef values.
 */
__STATIC_INLINE void PWMG_OC_SetPolarity(PWMG_TypeDef *PWMGx, PWMG_OC_Polarity_TypeDef Polarity)
{
    MODIFY_REG(PWMGx->CCER, PWMG_CCER_CC0P, (uint32_t)Polarity);
}

/**
 * @brief Returns the output polarity.
 * @param PWMGx The PWMG instance.
 * @returns The output polarity. Can be one of PWMG_OC_Polarity_TypeDef values.
 */
__STATIC_INLINE PWMG_OC_Polarity_TypeDef PWMG_OC_GetPolarity(PWMG_TypeDef *PWMGx)
{
    return (PWMG_OC_Polarity_TypeDef)READ_BIT(PWMGx->CCER, PWMG_CCER_CC0P);
}

/**
 * @brief Enables compare register (CCR0) preload for the output channel.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_OC_EnablePreload(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->CCMR1, PWMG_CCMR1_OC_OC0PE);
}

/**
 * @brief Disables compare register (CCR0) preload for the output channel.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_OC_DisablePreload(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->CCMR1, PWMG_CCMR1_OC_OC0PE);
}

/**
 * @brief Checks if compare register (CCR0) preload is enabled for the output channel.
 * @param PWMGx The PWMG instance.
 * @retval 1 if compare register (CCR0) preload is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_OC_IsEnabledPreload(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->CCMR1, PWMG_CCMR1_OC_OC0PE) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets the compare value for the output channel.
 * @param PWMGx The PWMG instance.
 * @param CompareValue The compare value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void PWMG_OC_SetCompare(PWMG_TypeDef *PWMGx, uint16_t CompareValue)
{
    WRITE_REG(PWMGx->CCR0, CompareValue & PWMG_CCR0);
}

/**
 * @brief Returns the compare value for the output channel.
 * @param PWMGx The PWMG instance.
 * @returns The compare value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t PWMG_OC_GetCompare(PWMG_TypeDef *PWMGx)
{
    return (uint16_t)(READ_REG(PWMGx->CCR0) & PWMG_CCR0);
}

/**
 * @brief Configures the input channel.
 * @note The function configures the channel as input and sets its parameters.
 * @param PWMGx The PWMG instance.
 * @param PWMG_IC_InitStruct The pointer to the @ref PWMG_IC_InitStruct_TypeDef structure.
 *                            The next parameters will be configured:
 *                            - ICPrescaler
 *                            - ICFilter
 *                            - ICPolarity
 */
__STATIC_INLINE void PWMG_IC_Config(PWMG_TypeDef *PWMGx, PWMG_IC_InitStruct_TypeDef *PWMG_IC_InitStruct)
{
    MODIFY_REG(PWMGx->CCMR1,
               (PWMG_CCMR1_IC_IC0F | PWMG_CCMR1_IC_IC0PSC | PWMG_CCMR1_CC0S),
               ((uint32_t)PWMG_IC_InitStruct->ICFilter |
                (uint32_t)PWMG_IC_InitStruct->ICPrescaler |
               PWMG_CCMR1_CC0S_IC));

    MODIFY_REG(PWMGx->CCER, PWMG_CCER_CC0P, (uint32_t)PWMG_IC_InitStruct->ICPolarity);
}

/**
 * @brief Sets the input channel prescaler.
 * @param PWMGx The PWMG instance.
 * @param Prescaler The prescaler. Can be one of PWMG_IC_Prescaler_TypeDef values.
 */
__STATIC_INLINE void PWMG_IC_SetPrescaler(PWMG_TypeDef *PWMGx, PWMG_IC_Prescaler_TypeDef Prescaler)
{
    MODIFY_REG(PWMGx->CCMR1, PWMG_CCMR1_IC_IC0PSC, (uint32_t)Prescaler);
}

/**
 * @brief Returns the input channel prescaler.
 * @param PWMGx The PWMG instance.
 * @returns The prescaler. Can be one of PWMG_IC_Prescaler_TypeDef values.
 */
__STATIC_INLINE PWMG_IC_Prescaler_TypeDef PWMG_IC_GetPrescaler(PWMG_TypeDef *PWMGx)
{
    return (PWMG_IC_Prescaler_TypeDef)READ_BIT(PWMGx->CCMR1, PWMG_CCMR1_IC_IC0PSC);
}

/**
 * @brief Configures the input capture filter.
 * @param PWMGx The PWMG instance.
 * @param Filter The filter configuration. Can be one of PWMG_IC_Filter_TypeDef values.
 */
__STATIC_INLINE void PWMG_IC_SetFilter(PWMG_TypeDef *PWMGx, PWMG_IC_Filter_TypeDef Filter)
{
    MODIFY_REG(PWMGx->CCMR1, PWMG_CCMR1_IC_IC0F, (uint32_t)Filter);
}

/**
 * @brief Returns the input capture filter configuration.
 * @param PWMGx The PWMG instance.
 * @returns The filter configuration. Can be one of PWMG_IC_Filter_TypeDef values.
 */
__STATIC_INLINE PWMG_IC_Filter_TypeDef PWMG_IC_GetFilter(PWMG_TypeDef *PWMGx)
{
    return (PWMG_IC_Filter_TypeDef)READ_BIT(PWMGx->CCMR1, PWMG_CCMR1_IC_IC0F);
}

/**
 * @brief Sets the input polarity.
 * @param PWMGx The PWMG instance.
 * @param Polarity The input polarity. Can be one of PWMG_IC_Polarity_TypeDef values.
 */
__STATIC_INLINE void PWMG_IC_SetPolarity(PWMG_TypeDef *PWMGx, PWMG_IC_Polarity_TypeDef Polarity)
{
    MODIFY_REG(PWMGx->CCER, PWMG_CCER_CC0P, (uint32_t)Polarity);
}

/**
 * @brief Returns the input polarity.
 * @param PWMGx The PWMG instance.
 * @returns The input polarity. Can be one of PWMG_IC_Polarity_TypeDef values.
 */
__STATIC_INLINE PWMG_IC_Polarity_TypeDef PWMG_IC_GetPolarity(PWMG_TypeDef *PWMGx)
{
    return (PWMG_IC_Polarity_TypeDef)READ_BIT(PWMGx->CCER, PWMG_CCER_CC0P);
}

/**
 * @brief Returns the captured value for the input channel.
 * @param PWMGx The PWMG instance.
 * @returns The captured value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t PWMG_IC_GetCapture(PWMG_TypeDef *PWMGx)
{
    return (uint16_t)(READ_REG(PWMGx->CCR0) & PWMG_CCR0);
}

/**
 * @brief Checks if a flag is set.
 * @param PWMGx The PWMG instance.
 * @param Flag The flag. Can be one of PWMG_Flag_TypeDef values.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t PWMG_IsActiveFlag(PWMG_TypeDef *PWMGx, PWMG_Flag_TypeDef Flag)
{
    return ((READ_BIT(PWMGx->SR, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears a flag.
 * @param PWMGx The PWMG instance.
 * @param Flag The flag. Can be one of PWMG_Flag_TypeDef values.
 */
__STATIC_INLINE void PWMG_ClearFlag(PWMG_TypeDef *PWMGx, PWMG_Flag_TypeDef Flag)
{
    CLEAR_BIT(PWMGx->SR, (1UL << Flag));
}

/**
 * @brief Checks if the Update Interrupt Flag (UIF) is set.
 * @param PWMGx The PWMG instance.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t PWMG_IsActiveFlag_UPDATE(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->SR, PWMG_SR_UIF) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears the Update Interrupt Flag (UIF).
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_ClearFlag_UPDATE(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->SR, PWMG_SR_UIF);
}

/**
 * @brief Checks if the Capture/Compare 0 Interrupt Flag (CC0IF) is set.
 * @param PWMGx The PWMG instance.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t PWMG_IsActiveFlag_CC0(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->SR, PWMG_SR_CC0IF) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears the Capture/Compare 0 Interrupt Flag (CC0IF).
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_ClearFlag_CC0(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->SR, PWMG_SR_CC0IF);
}

/**
 * @brief Checks if the Capture/Compare 0 Overcapture Flag (CC0OF) is set.
 * @param PWMGx The PWMG instance.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t PWMG_IsActiveFlag_CC0OVR(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->SR, PWMG_SR_CC0OF) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears the Capture/Compare 0 Overcapture Flag (CC0OF).
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_ClearFlag_CC0OVR(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->SR, PWMG_SR_CC0OF);
}

/**
 * @brief Enables an interrupt.
 * @param PWMGx The PWMG instance.
 * @param IT The interrupt source. Can be one of PWMG_IT_TypeDef values.
 */
__STATIC_INLINE void PWMG_EnableIT(PWMG_TypeDef *PWMGx, PWMG_IT_TypeDef IT)
{
    SET_BIT(PWMGx->DIER, (1UL << IT));
}

/**
 * @brief Disables an interrupt.
 * @param PWMGx The PWMG instance.
 * @param IT The interrupt source. Can be one of PWMG_IT_TypeDef values.
 */
__STATIC_INLINE void PWMG_DisableIT(PWMG_TypeDef *PWMGx, PWMG_IT_TypeDef IT)
{
    CLEAR_BIT(PWMGx->DIER, (1UL << IT));
}

/**
 * @brief Checks if an interrupt is enabled.
 * @param PWMGx The PWMG instance.
 * @param IT The interrupt source. Can be one of PWMG_IT_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledIT(PWMG_TypeDef *PWMGx, PWMG_IT_TypeDef IT)
{
    return ((READ_BIT(PWMGx->DIER, (1UL << IT)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables the Update interrupt.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_EnableIT_UPDATE(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->DIER, PWMG_DIER_UIE);
}

/**
 * @brief Disables the Update interrupt.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_DisableIT_UPDATE(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->DIER, PWMG_DIER_UIE);
}

/**
 * @brief Checks if the Update interrupt is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledIT_UPDATE(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->DIER, PWMG_DIER_UIE) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables the Capture/Compare 0 interrupt.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_EnableIT_CC0(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->DIER, PWMG_DIER_CC0IE);
}

/**
 * @brief Disables the Capture/Compare 0 interrupt.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_DisableIT_CC0(PWMG_TypeDef *PWMGx)
{
    CLEAR_BIT(PWMGx->DIER, PWMG_DIER_CC0IE);
}

/**
 * @brief Checks if the Capture/Compare 0 interrupt is enabled.
 * @param PWMGx The PWMG instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t PWMG_IsEnabledIT_CC0(PWMG_TypeDef *PWMGx)
{
    return ((READ_BIT(PWMGx->DIER, PWMG_DIER_CC0IE) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears the pending interrupt.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_ClearIT(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->DIER, PWMG_DIER_INTR_CLEAR);
}

/**
 * @brief Generates an event.
 * @param PWMGx The PWMG instance.
 * @param Event The event. Can be one of PWMG_Event_TypeDef values.
 */
__STATIC_INLINE void PWMG_GenerateEvent(PWMG_TypeDef *PWMGx, PWMG_Event_TypeDef Event)
{
    SET_BIT(PWMGx->EGR, (1UL << Event));
}

/**
 * @brief Generates the Update event.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_GenerateEvent_UPDATE(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->EGR, PWMG_EGR_UG);
}

/**
 * @brief Generates the Capture/Compare 0 event.
 * @param PWMGx The PWMG instance.
 */
__STATIC_INLINE void PWMG_GenerateEvent_CC0(PWMG_TypeDef *PWMGx)
{
    SET_BIT(PWMGx->EGR, PWMG_EGR_CC0G);
}

ErrorStatus PWMG_DeInit(PWMG_TypeDef *PWMGx);
ErrorStatus PWMG_Init(PWMG_TypeDef *PWMGx, PWMG_InitStruct_TypeDef *PWMG_InitStruct);
void PWMG_StructInit(PWMG_InitStruct_TypeDef *PWMG_InitStruct);
ErrorStatus PWMG_OC_Init(PWMG_TypeDef *PWMGx, PWMG_OC_InitStruct_TypeDef *PWMG_OC_InitStruct);
void PWMG_OC_StructInit(PWMG_OC_InitStruct_TypeDef *PWMG_OC_InitStruct);
ErrorStatus PWMG_IC_Init(PWMG_TypeDef *PWMGx, PWMG_IC_InitStruct_TypeDef *PWMG_IC_InitStruct);
void PWMG_IC_StructInit(PWMG_IC_InitStruct_TypeDef *PWMG_IC_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_PWMG_H */

/**
 * *****************************************************************************
 *  @file       bmcu_tim.h
 *  @author     Baikal electronics SDK team
 *  @brief      Timer module driver header file
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

#ifndef __BMCU_TIM_H
#define __BMCU_TIM_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Timer channel */
typedef enum {
    TIM_CH0 = 0,    /*!< Channel 0 */
    TIM_CH1,        /*!< Channel 1 */
    TIM_CH2,        /*!< Channel 2 */
    TIM_CH3         /*!< Channel 3 */
} TIM_Channels_TypeDef;

/** @brief Timer counter mode */
typedef enum {
    TIM_COUNTERMODE_FREE_RUNNING = TIM_CH_CONTROL_MODE_FREE_RUNNING,    /*!< Free running mode. Counts down from LoadCount,
                                                                        * not stops when reaching 0 (then becomes 0xffffffff and futher),
                                                                        * generates interrupt each time when meets 0 */
    TIM_COUNTERMODE_PERIODIC = TIM_CH_CONTROL_MODE_USER_DEFINED     /*!< Periodic mode. Counts down from LoadCount,
                                                                    * when reaching 0, generates interrupt and
                                                                    * starts counting down from LoadCount again */
} TIM_CounterMode_TypeDef;

/** @brief TIM Init Structure definition */
typedef struct
{
    TIM_CounterMode_TypeDef CounterMode;            /*!< Counter mode */
    FunctionalState PWMOutputCtrl;                  /*!< PWM output enable */
    FunctionalState PWM0N100ModeCtrl;               /*!< 0% and 100% PWM mode enable */
    uint32_t LoadCount;                             /*!< Load count (LOW period) */
    uint32_t LoadCount2;                            /*!< Load count 2 (HIGH period) */
} TIM_InitStruct_TypeDef;

/**
 * @brief Writes a value in TIM register.
 * @param INSTANCE The TIM instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define TIM_WriteReg(INSTANCE, REG, VALUE)      WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in TIM register.
 * @param INSTANCE The TIM instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define TIM_ReadReg(INSTANCE, REG)              READ_REG(INSTANCE->REG)

/**
 * @brief Enables a timer channel.
 * @note When a timer transitions from disabled to enabled, the current value of
 *       its LOADCOUNT register is loaded into the timer counter on the next
 *       rising edge of timer clock.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_EnableChannel(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    SET_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_ENABLE);
}

/**
 * @brief Disables a timer channel.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_DisableChannel(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_ENABLE);
}

/**
 * @brief Checks if a timer channel is enabled.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @retval 1 if the timer channel is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t TIM_IsEnabledChannel(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return ((READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_ENABLE) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets the timer channel counter mode.
 * @note When a timer counts down to 0, it loads the current value of the
 *       LOADCOUNT register in User-defined count mode and the maximum value in
 *       Free-running mode.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @param CounterMode The counter mode. Can be one of TIM_CounterMode_TypeDef values.
 */
__STATIC_INLINE void TIM_SetCounterMode(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel, TIM_CounterMode_TypeDef CounterMode)
{
    MODIFY_REG(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE, CounterMode);
}

/**
 * @brief Returns the timer channel counter mode.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @returns The counter mode. Can be one of TIM_CounterMode_TypeDef values.
 */
__STATIC_INLINE TIM_CounterMode_TypeDef TIM_GetCounterMode(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return (TIM_CounterMode_TypeDef)READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE);
}

/**
 * @brief Returns the counter value.
 * @note 0 is always read back when the timer is not enabled; otherwise,
         the current value of the timer is read back.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @returns The counter value.
 */
__STATIC_INLINE uint32_t TIM_GetCounter(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return (uint32_t)(READ_REG(TIMx->CH[Channel].CURRENTVAL));
}

/**
 * @brief Sets timer channel load count.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @param LoadCount The load count.
 */
__STATIC_INLINE void TIM_SetLoadCount(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel, uint32_t LoadCount)
{
    WRITE_REG(TIMx->CH[Channel].LOADCOUNT1, LoadCount);
}

/**
 * @brief Returns timer channel load count.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @returns The load count.
 */
__STATIC_INLINE uint32_t TIM_GetLoadCount(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return (uint32_t)(READ_REG(TIMx->CH[Channel].LOADCOUNT1));
}

/**
 * @brief Sets timer channel load count 2.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @param LoadCount The load count 2.
 */
__STATIC_INLINE void TIM_SetLoadCount2(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel, uint32_t LoadCount)
{
    WRITE_REG(TIMx->LOADCOUNT2[Channel], LoadCount);
}

/**
 * @brief Returns timer channel load count 2.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @returns The load count 2.
 */
__STATIC_INLINE uint32_t TIM_GetLoadCount2(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return (uint32_t)(READ_REG(TIMx->LOADCOUNT2[Channel]));
}

/**
 * @brief Enables Pulse Width Modulation output.
 * @note The load count value defines the LOW period and the load count 2 value
 *       defines the HIGH period of the timer toggle output.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_EnablePWMOutput(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    SET_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_PWM);
}

/**
 * @brief Disables Pulse Width Modulation output.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_DisablePWMOutput(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_PWM);
}

/**
 * @brief Checks if Pulse Width Modulation output is enabled.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @retval 1 if PWM output is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t TIM_IsEnabledPWMOutput(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return ((READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_PWM) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables 0% and 100% PWM mode.
 * @note 0% and 100% PWM mode can be enabled only along with the timer PWM mode
 *       and the user defined count mode. If any of these modes are not enabled,
 *       that is if PWM mode or user defined mode is not enabled, then the timer
 *       operates in the normal PWM mode or free running mode.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_Enable0N100PWM(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    SET_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE_0N100PWM_EN);
}

/**
 * @brief Disables 0% and 100% PWM mode.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_Disable0N100PWM(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE_0N100PWM_EN);
}

/**
 * @brief Checks if 0% and 100% PWM mode is enabled.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @retval 1 if 0% and 100% PWM mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t TIM_IsEnabled0N100PWM(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return ((READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE_0N100PWM_EN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables a timer channel interrupt.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_EnableIT(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_INT_MASK);
}

/**
 * @brief Disables a timer channel interrupt.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_DisableIT(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    SET_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_INT_MASK);
}

/**
 * @brief Checks if a timer channel interrupt is enabled.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t TIM_IsEnabledIT(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    return ((READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_INT_MASK) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears a timer channel interrupt.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 */
__STATIC_INLINE void TIM_ClearIT(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    (void) READ_BIT(TIMx->CH[Channel].EOI, TIM_CH_EOI);
}

ErrorStatus TIM_DeInit(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel);
ErrorStatus TIM_Init(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel, TIM_InitStruct_TypeDef *TIM_InitStruct);
void TIM_StructInit(TIM_InitStruct_TypeDef *TIM_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_TIM_H */

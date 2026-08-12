/**
 * *****************************************************************************
 *  @file       bmcu_tim.c
 *  @author     Baikal electronics SDK team
 *  @brief      Timer module driver
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

#include "bmcu_tim.h"
#include "bmcu_def.h"

#define IS_TIM_CHANNEL(__VALUE__)               (((__VALUE__) == TIM_CH0) || \
                                                 ((__VALUE__) == TIM_CH1) || \
                                                 ((__VALUE__) == TIM_CH2) || \
                                                 ((__VALUE__) == TIM_CH3))

#define IS_TIM_COUNTER_MODE(__VALUE__)          (((__VALUE__) == TIM_COUNTERMODE_FREE_RUNNING) || \
                                                 ((__VALUE__) == TIM_COUNTERMODE_PERIODIC))

/**
 * @brief Deinitializes a timer channel.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The TIM channel has been deinitialized.
 *         - ERROR: The TIM channel has not been deinitialized.
 */
ErrorStatus TIM_DeInit(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel)
{
    ErrorStatus status = SUCCESS;

    /* Check parameters */
    assert(IS_TIM_ALL_INSTANCE(TIMx));
    assert(IS_TIM_CHANNEL(Channel));

    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_ENABLE);

    CLEAR_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_MODE_0N100PWM_EN | TIM_CH_CONTROL_PWM |
                                         TIM_CH_CONTROL_INT_MASK | TIM_CH_CONTROL_MODE);

    WRITE_REG(TIMx->CH[Channel].LOADCOUNT1, 0x0UL);
    WRITE_REG(TIMx->LOADCOUNT2[Channel], 0x0UL);

    return status;
}

/**
 * @brief Initializes a TIM channel.
 * @note TIM_InitStruct structure should be initialized prior to calling this function.
 * @param TIMx The TIM instance.
 * @param Channel The TIM channel. Can be one of TIM_Channels_TypeDef values.
 * @param TIM_InitStruct The pointer to TIM_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The TIM channel has been initialized.
 *         - ERROR: The TIM channel has not been initialized.
 */
ErrorStatus TIM_Init(TIM_TypeDef *TIMx, TIM_Channels_TypeDef Channel, TIM_InitStruct_TypeDef *TIM_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_TIM_ALL_INSTANCE(TIMx));
    assert(IS_TIM_CHANNEL(Channel));
    assert(IS_TIM_COUNTER_MODE(TIM_InitStruct->CounterMode));
    assert(IS_FUNCTIONAL_STATE(TIM_InitStruct->PWMOutputCtrl));
    assert(IS_FUNCTIONAL_STATE(TIM_InitStruct->PWM0N100ModeCtrl));

    if (READ_BIT(TIMx->CH[Channel].CONTROL, TIM_CH_CONTROL_ENABLE) == 0UL)
    {
        /* The selected channel is disabled */

        /* Configure timer mode */
        MODIFY_REG(TIMx->CH[Channel].CONTROL,
                   (TIM_CH_CONTROL_MODE_0N100PWM_EN | TIM_CH_CONTROL_PWM | TIM_CH_CONTROL_MODE),
                   (((uint32_t)TIM_InitStruct->PWM0N100ModeCtrl << TIM_CH_CONTROL_MODE_0N100PWM_EN_Pos) |
                   ((uint32_t)TIM_InitStruct->PWMOutputCtrl << TIM_CH_CONTROL_PWM_Pos) |
                   ((uint32_t)TIM_InitStruct->CounterMode)));

        /* Set load count */
        WRITE_REG(TIMx->CH[Channel].LOADCOUNT1, TIM_InitStruct->LoadCount);
        WRITE_REG(TIMx->LOADCOUNT2[Channel], TIM_InitStruct->LoadCount2);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of TIM_InitStruct_TypeDef structure to default value.
 * @param TIM_InitStruct The pointer to TIM_InitStruct_TypeDef structure.
 */
void TIM_StructInit(TIM_InitStruct_TypeDef *TIM_InitStruct)
{
    /* Set TIM_InitStruct fields to default values */
    TIM_InitStruct->CounterMode = TIM_COUNTERMODE_FREE_RUNNING;
    TIM_InitStruct->PWMOutputCtrl = DISABLE;
    TIM_InitStruct->PWM0N100ModeCtrl = DISABLE;
    TIM_InitStruct->LoadCount = 0xFFFFFFFFUL;
    TIM_InitStruct->LoadCount2 = 0xFFFFFFFFUL;
}

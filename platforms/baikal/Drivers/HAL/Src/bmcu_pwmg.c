/**
 * *****************************************************************************
 *  @file       bmcu_pwmg.c
 *  @author     Baikal electronics SDK team
 *  @brief      PWMG module driver
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

#include "bmcu_pwmg.h"
#include "bmcu_def.h"

#define IS_PWMG_ONEPULSEMODE(__VALUE__)         (((__VALUE__) == PWMG_ONEPULSEMODE_REPETITIVE) || \
                                                 ((__VALUE__) == PWMG_ONEPULSEMODE_SINGLE))

#define IS_PWMG_CLOCKDIVISION(__VALUE__)        (((__VALUE__) == PWMG_CLOCKDIVISION_DIV1) || \
                                                 ((__VALUE__) == PWMG_CLOCKDIVISION_DIV2) || \
                                                 ((__VALUE__) == PWMG_CLOCKDIVISION_DIV4))

#define IS_PWMG_OC_MODE(__VALUE__)              (((__VALUE__) == PWMG_OC_MODE_FROZEN) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_ACTIVE) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_INACTIVE) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_TOGGLE) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_FORCED_INACTIVE) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_FORCED_ACTIVE) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_PWM1) || \
                                                 ((__VALUE__) == PWMG_OC_MODE_PWM2))

#define IS_PWMG_OC_POLARITY(__VALUE__)          (((__VALUE__) == PWMG_OC_POLARITY_HIGH) || \
                                                 ((__VALUE__) == PWMG_OC_POLARITY_LOW))

#define IS_PWMG_OC_STATE(__VALUE__)             (((__VALUE__) == PWMG_OC_STATE_DISABLE) || \
                                                 ((__VALUE__) == PWMG_OC_STATE_ENABLE))

#define IS_PWMG_IC_PRESCALER(__VALUE__)         (((__VALUE__) == PWMG_IC_PRESCALER_DIV1) || \
                                                 ((__VALUE__) == PWMG_IC_PRESCALER_DIV2) || \
                                                 ((__VALUE__) == PWMG_IC_PRESCALER_DIV4) || \
                                                 ((__VALUE__) == PWMG_IC_PRESCALER_DIV8))

#define IS_PWMG_IC_FILTER(__VALUE__)            (((__VALUE__) == PWMG_IC_FILTER_FDIV1) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV1_N2) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV1_N4) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV1_N8) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV2_N6) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV2_N8) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV4_N6) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV4_N8) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV8_N6) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV8_N8) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV16_N5) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV16_N6) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV16_N8) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV32_N5) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV32_N6) || \
                                                 ((__VALUE__) == PWMG_IC_FILTER_FDIV32_N8))

#define IS_PWMG_IC_POLARITY(__VALUE__)          (((__VALUE__) == PWMG_IC_POLARITY_RISING) || \
                                                 ((__VALUE__) == PWMG_IC_POLARITY_FALLING))

/**
 * @brief Deinitializes a PWMG instance.
 * @param PWMGx The PWMG instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMG instance have been deinitialized.
 *         - ERROR: The PWMG instance have not been deinitialized.
 */
ErrorStatus PWMG_DeInit(PWMG_TypeDef *PWMGx)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMG_ALL_INSTANCE(PWMGx));

    /* Disable the counter if enabled */
    if (PWMG_IsEnabledCounter(PWMGx) != 0UL)
    {
        PWMG_DisableCounter(PWMGx);
    }

    /* Reset registers */
    if (PWMG_IsEnabledCounter(PWMGx) == 0UL)
    {
        CLEAR_BIT(PWMGx->CR1, (PWMG_CR1_CKD | PWMG_CR1_ARPE | PWMG_CR1_OPM | PWMG_CR1_UDIS));

        WRITE_REG(PWMGx->CCMR1, 0x0UL);
        CLEAR_BIT(PWMGx->CCER, (PWMG_CCER_CC0P | PWMG_CCER_CC0E));

        WRITE_REG(PWMGx->DIER, PWMG_DIER_INTR_CLEAR);
        WRITE_REG(PWMGx->SR, 0x0UL);

        WRITE_REG(PWMGx->CNT, 0x0UL);

        WRITE_REG(PWMGx->PSC, 0x0UL);
        WRITE_REG(PWMGx->ARR, 0x0UL);

        WRITE_REG(PWMGx->CCR0, 0x0UL);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Initializes a PWMG instance.
 * @note PWMG_InitStruct structure should be initialized prior to calling this function.
 * @param PWMGx The PWMG instance.
 * @param PWMG_InitStruct The pointer to PWMG_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMG instance have been initialized.
 *         - ERROR: The PWMG instance have not been initialized.
 */
ErrorStatus PWMG_Init(PWMG_TypeDef *PWMGx, PWMG_InitStruct_TypeDef *PWMG_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert(IS_PWMG_ALL_INSTANCE(PWMGx));
    assert(IS_PWMG_CLOCKDIVISION(PWMG_InitStruct->ClockDivision));

    if (PWMG_IsEnabledCounter(PWMGx) == 0UL)
    {
        /* The counter is disabled */

        /* Configure counter */
        MODIFY_REG(PWMGx->CR1,
                   (PWMG_CR1_CKD | PWMG_CR1_ARPE | PWMG_CR1_OPM | PWMG_CR1_UDIS),
                   (uint32_t)PWMG_InitStruct->ClockDivision);

        WRITE_REG(PWMGx->SR, 0x0UL);
        WRITE_REG(PWMGx->DIER, PWMG_DIER_INTR_CLEAR);

        WRITE_REG(PWMGx->CNT, 0x0UL);

        WRITE_REG(PWMGx->PSC, PWMG_InitStruct->Prescaler);
        WRITE_REG(PWMGx->ARR, PWMG_InitStruct->Autoreload);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of PWMG_InitStruct_TypeDef structure to default value.
 * @param PWMG_InitStruct The pointer to PWMG_InitStruct_TypeDef structure.
 */
void PWMG_StructInit(PWMG_InitStruct_TypeDef *PWMG_InitStruct)
{
    /* Set PWMG_InitStruct fields to default values */
    PWMG_InitStruct->Prescaler = 0U;
    PWMG_InitStruct->Autoreload = 0xFFFFU;
    PWMG_InitStruct->ClockDivision = PWMG_CLOCKDIVISION_DIV1;
}

/**
 * @brief Configures the PWMG output channel.
 * @note PWMG_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMGx The PWMG instance.
 * @param PWMG_OC_InitStruct The pointer to PWMG_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMG output channel have been initialized.
 *         - ERROR: The PWMG output channel have not been initialized.
 */
ErrorStatus PWMG_OC_Init(PWMG_TypeDef *PWMGx, PWMG_OC_InitStruct_TypeDef *PWMG_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert(IS_PWMG_ALL_INSTANCE(PWMGx));
    assert(IS_PWMG_OC_MODE(PWMG_OC_InitStruct->OCMode));
    assert(IS_PWMG_OC_POLARITY(PWMG_OC_InitStruct->OCPolarity));
    assert(IS_PWMG_OC_STATE(PWMG_OC_InitStruct->OCState));

    if (PWMG_CC_IsEnabledChannel(PWMGx) == 0UL)
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMGx->CCMR1,
                   (PWMG_CCMR1_OC_OC0M | PWMG_CCMR1_OC_OC0PE | PWMG_CCMR1_CC0S),
                   ((uint32_t)PWMG_OC_InitStruct->OCMode | PWMG_CCMR1_CC0S_OC));

        MODIFY_REG(PWMGx->CCER,
                   (PWMG_CCER_CC0P | PWMG_CCER_CC0E),
                   ((uint32_t)PWMG_OC_InitStruct->OCPolarity | (uint32_t)PWMG_OC_InitStruct->OCState));

        /* Set the compare value */
        WRITE_REG(PWMGx->CCR0, PWMG_OC_InitStruct->CompareValue);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of PWMG_OC_InitStruct_TypeDef structure to default value.
 * @param PWMG_OC_InitStruct The pointer to PWMG_OC_InitStruct_TypeDef structure.
 */
void PWMG_OC_StructInit(PWMG_OC_InitStruct_TypeDef *PWMG_OC_InitStruct)
{
    /* Set PWMG_OC_InitStruct fields to default values */
    PWMG_OC_InitStruct->OCMode = PWMG_OC_MODE_FROZEN;
    PWMG_OC_InitStruct->OCPolarity = PWMG_OC_POLARITY_HIGH;
    PWMG_OC_InitStruct->OCState = PWMG_OC_STATE_DISABLE;
    PWMG_OC_InitStruct->CompareValue = 0x0U;
}

/**
 * @brief Configures the PWMG input channel.
 * @note PWMG_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMGx The PWMG instance.
 * @param PWMG_IC_InitStruct The pointer to PWMG_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMG input channel have been initialized.
 *         - ERROR: The PWMG input channel have not been initialized.
 */
ErrorStatus PWMG_IC_Init(PWMG_TypeDef *PWMGx, PWMG_IC_InitStruct_TypeDef *PWMG_IC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert(IS_PWMG_ALL_INSTANCE(PWMGx));
    assert(IS_PWMG_IC_PRESCALER(PWMG_IC_InitStruct->ICPrescaler));
    assert(IS_PWMG_IC_FILTER(PWMG_IC_InitStruct->ICFilter));
    assert(IS_PWMG_IC_POLARITY(PWMG_IC_InitStruct->ICPolarity));

    if (PWMG_CC_IsEnabledChannel(PWMGx) == 0UL)
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMGx->CCMR1,
                   (PWMG_CCMR1_IC_IC0F | PWMG_CCMR1_IC_IC0PSC | PWMG_CCMR1_CC0S),
                   ((uint32_t)PWMG_IC_InitStruct->ICFilter |
                    (uint32_t)PWMG_IC_InitStruct->ICPrescaler |
                    PWMG_CCMR1_CC0S_IC));

        MODIFY_REG(PWMGx->CCER, PWMG_CCER_CC0P, (uint32_t)PWMG_IC_InitStruct->ICPolarity);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of PWMG_IC_InitStruct_TypeDef structure to default value.
 * @param PWMG_IC_InitStruct The pointer to PWMG_IC_InitStruct_TypeDef structure.
 */
void PWMG_IC_StructInit(PWMG_IC_InitStruct_TypeDef *PWMG_IC_InitStruct)
{
    /* Set PWMG_IC_InitStruct fields to default values */
    PWMG_IC_InitStruct->ICPrescaler = PWMG_IC_PRESCALER_DIV1;
    PWMG_IC_InitStruct->ICFilter = PWMG_IC_FILTER_FDIV1;
    PWMG_IC_InitStruct->ICPolarity = PWMG_IC_POLARITY_RISING;
}

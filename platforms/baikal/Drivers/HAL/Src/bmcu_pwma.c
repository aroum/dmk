/**
 * *****************************************************************************
 *  @file       bmcu_pwma.c
 *  @author     Baikal electronics SDK team
 *  @brief      PWMA timer module driver source file
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

#include "bmcu_pwma.h"
#include "bmcu_def.h"

#define IS_PWMA_ONEPULSEMODE(__VALUE__)         (((__VALUE__) == PWMA_ONEPULSEMODE_REPETITIVE) || \
                                                 ((__VALUE__) == PWMA_ONEPULSEMODE_SINGLE))

#define IS_PWMA_COUNTERMODE(__VALUE__)          (((__VALUE__) == PWMA_COUNTERMODE_UP) || \
                                                 ((__VALUE__) == PWMA_COUNTERMODE_DOWN) || \
                                                 ((__VALUE__) == PWMA_COUNTERMODE_CENTER_UP) || \
                                                 ((__VALUE__) == PWMA_COUNTERMODE_CENTER_DOWN) || \
                                                 ((__VALUE__) == PWMA_COUNTERMODE_CENTER_UP_DOWN))

#define IS_PWMA_CLOCKDIVISION(__VALUE__)        (((__VALUE__) == PWMA_CLOCKDIVISION_DIV1) || \
                                                 ((__VALUE__) == PWMA_CLOCKDIVISION_DIV2) || \
                                                 ((__VALUE__) == PWMA_CLOCKDIVISION_DIV4))

#define IS_PWMA_COUNTERDIRECTION(__VALUE__)     (((__VALUE__) == PWMA_COUNTERDIRECTION_UP) || \
                                                 ((__VALUE__) == PWMA_COUNTERDIRECTION_DOWN))

#define IS_PWMA_CC_UPDATESOURCE(__VALUE__)      (((__VALUE__) == PWMA_CC_UPDATESOURCE_COMG_ONLY) || \
                                                 ((__VALUE__) == PWMA_CC_UPDATESOURCE_COMG_AND_TRGI))

#define IS_PWMA_TRGO(__VALUE__)                 (((__VALUE__) == PWMA_TRGO_RESET) || \
                                                 ((__VALUE__) == PWMA_TRGO_ENABLE) || \
                                                 ((__VALUE__) == PWMA_TRGO_UPDATE) || \
                                                 ((__VALUE__) == PWMA_TRGO_CC0IF) || \
                                                 ((__VALUE__) == PWMA_TRGO_OC0REF) || \
                                                 ((__VALUE__) == PWMA_TRGO_OC1REF) || \
                                                 ((__VALUE__) == PWMA_TRGO_OC2REF) || \
                                                 ((__VALUE__) == PWMA_TRGO_OC3REF))

#define IS_PWMA_OC_STATE(__VALUE__)             (((__VALUE__) == PWMA_OC_STATE_DISABLE) || \
                                                 ((__VALUE__) == PWMA_OC_STATE_ENABLE))

#define IS_PWMA_OC_MODE(__VALUE__)              (((__VALUE__) == PWMA_OC_MODE_FROZEN) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_ACTIVE) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_INACTIVE) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_TOGGLE) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_FORCED_INACTIVE) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_FORCED_ACTIVE) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_PWM1) || \
                                                 ((__VALUE__) == PWMA_OC_MODE_PWM2))

#define IS_PWMA_OC_POLARITY(__VALUE__)          (((__VALUE__) == PWMA_OC_POLARITY_HIGH) || \
                                                 ((__VALUE__) == PWMA_OC_POLARITY_LOW))

#define IS_PWMA_OC_IDLESTATE(__VALUE__)         (((__VALUE__) == PWMA_OC_IDLESTATE_LOW) || \
                                                 ((__VALUE__) == PWMA_OC_IDLESTATE_HIGH))

#define IS_PWMA_IC_ACTIVEINPUT(__VALUE__)       (((__VALUE__) == PWMA_IC_ACTIVEINPUT_DIRECTTI) || \
                                                 ((__VALUE__) == PWMA_IC_ACTIVEINPUT_INDIRECTTI) || \
                                                 ((__VALUE__) == PWMA_IC_ACTIVEINPUT_TRC))

#define IS_PWMA_IC_PRESCALER(__VALUE__)         (((__VALUE__) == PWMA_IC_PRESCALER_DIV1) || \
                                                 ((__VALUE__) == PWMA_IC_PRESCALER_DIV2) || \
                                                 ((__VALUE__) == PWMA_IC_PRESCALER_DIV4) || \
                                                 ((__VALUE__) == PWMA_IC_PRESCALER_DIV8))

#define IS_PWMA_IC_FILTER(__VALUE__)            (((__VALUE__) == PWMA_IC_FILTER_FDIV1) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV1_N2) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV1_N4) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV1_N8) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV2_N6) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV2_N8) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV4_N6) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV4_N8) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV8_N6) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV8_N8) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV16_N5) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV16_N6) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV16_N8) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV32_N5) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV32_N6) || \
                                                 ((__VALUE__) == PWMA_IC_FILTER_FDIV32_N8))

#define IS_PWMA_IC_POLARITY(__VALUE__)          (((__VALUE__) == PWMA_IC_POLARITY_RISING) || \
                                                 ((__VALUE__) == PWMA_IC_POLARITY_FALLING))

#define IS_PWMA_CLOCKSOURCE(__VALUE__)          (((__VALUE__) == PWMA_CLOCKSOURCE_INTERNAL) || \
                                                 ((__VALUE__) == PWMA_CLOCKSOURCE_EXT_MODE1) || \
                                                 ((__VALUE__) == PWMA_CLOCKSOURCE_EXT_MODE2))

#define IS_PWMA_SLAVEMODE(__VALUE__)            (((__VALUE__) == PWMA_SLAVEMODE_DISABLED) || \
                                                 ((__VALUE__) == PWMA_SLAVEMODE_RESET) || \
                                                 ((__VALUE__) == PWMA_SLAVEMODE_GATED) || \
                                                 ((__VALUE__) == PWMA_SLAVEMODE_TRIGGER))

#define IS_PWMA_TS(__VALUE__)                   (((__VALUE__) == PWMA_TS_ITR0) || \
                                                 ((__VALUE__) == PWMA_TS_ITR1) || \
                                                 ((__VALUE__) == PWMA_TS_ITR2) || \
                                                 ((__VALUE__) == PWMA_TS_TI0F_ED) || \
                                                 ((__VALUE__) == PWMA_TS_TI0FP0) || \
                                                 ((__VALUE__) == PWMA_TS_TI1FP1) || \
                                                 ((__VALUE__) == PWMA_TS_ETRF))

#define IS_PWMA_ETR_POLARITY(__VALUE__)         (((__VALUE__) == PWMA_ETR_POLARITY_NONINVERTED) || \
                                                 ((__VALUE__) == PWMA_ETR_POLARITY_INVERTED))

#define IS_PWMA_ETR_PRESCALER(__VALUE__)        (((__VALUE__) == PWMA_ETR_PRESCALER_DIV1) || \
                                                 ((__VALUE__) == PWMA_ETR_PRESCALER_DIV2) || \
                                                 ((__VALUE__) == PWMA_ETR_PRESCALER_DIV4) || \
                                                 ((__VALUE__) == PWMA_ETR_PRESCALER_DIV8))

#define IS_PWMA_ETR_FILTER(__VALUE__)           (((__VALUE__) == PWMA_ETR_FILTER_FDIV1) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV1_N2) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV1_N4) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV1_N8) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV2_N6) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV2_N8) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV4_N6) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV4_N8) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV8_N6) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV8_N8) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV16_N5) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV16_N6) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV16_N8) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV32_N5) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV32_N6) || \
                                                 ((__VALUE__) == PWMA_ETR_FILTER_FDIV32_N8))

#define IS_PWMA_BRK_STATE(__VALUE__)            (((__VALUE__) == PWMA_BRK_DISABLE) || \
                                                 ((__VALUE__) == PWMA_BRK_ENABLE))

#define IS_PWMA_BRK_POLARITY(__VALUE__)         (((__VALUE__) == PWMA_BRK_POLARITY_LOW) || \
                                                 ((__VALUE__) == PWMA_BRK_POLARITY_HIGH))

#define IS_PWMA_OSSR_STATE(__VALUE__)           (((__VALUE__) == PWMA_OSSR_DISABLE) || \
                                                 ((__VALUE__) == PWMA_OSSR_ENABLE))

#define IS_PWMA_AUTOMATICOUTPUT(__VALUE__)      (((__VALUE__) == PWMA_AUTOMATICOUTPUT_DISABLE) || \
                                                 ((__VALUE__) == PWMA_AUTOMATICOUTPUT_ENABLE))

#define IS_PWMA_DMABURST_BASEADDR(__VALUE__)    (((__VALUE__) == PWMA_DMABURST_BASEADDR_CR1) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CR2) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_SMCR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_DIER) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_SR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_EGR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCMR1) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCMR2) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCER) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CNT) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_PSC) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_ARR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_RCR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCR0) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCR1) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCR2) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_CCR3) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_BDTR) || \
                                                 ((__VALUE__) == PWMA_DMABURST_BASEADDR_DCR))

#define IS_PWMA_DMABURST_LENGTH(__VALUE__)      (((__VALUE__) == PWMA_DMABURST_LENGTH_1TRANSFER) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_2TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_3TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_4TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_5TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_6TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_7TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_8TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_9TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_10TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_11TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_12TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_13TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_14TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_15TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_16TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_17TRANSFERS) || \
                                                 ((__VALUE__) == PWMA_DMABURST_LENGTH_18TRANSFERS))

static ErrorStatus OC0Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC1Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC2Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC3Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus IC0Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC1Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC2Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC3Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);

/**
 * @brief Deinitializes a PWMA instance.
 * @param PWMAx The PWMA instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA instance has been deinitialized.
 *         - ERROR: The PWMA instance has not been deinitialized.
 */
ErrorStatus PWMA_DeInit(PWMA_TypeDef *PWMAx)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));

    /* Disable the counter if enabled */
    if (PWMA_IsEnabledCounter(PWMAx) != 0UL)
    {
        PWMA_DisableCounter(PWMAx);
    }

    /* Reset registers */
    if (PWMA_IsEnabledCounter(PWMAx) == 0UL)
    {
        CLEAR_BIT(PWMAx->CR1, (PWMA_CR1_CKD | PWMA_CR1_ARPE | PWMA_CR1_CMS |
                               PWMA_CR1_DIR | PWMA_CR1_OPM | PWMA_CR1_UDIS));
        CLEAR_BIT(PWMAx->CR2, (PWMA_CR2_OIS2N | PWMA_CR2_OIS2 | PWMA_CR2_OIS1N |
                               PWMA_CR2_OIS1 | PWMA_CR2_OIS0N | PWMA_CR2_OIS0 |
                               PWMA_CR2_TI0S | PWMA_CR2_MMS | PWMA_CR2_CCUS |
                               PWMA_CR2_CCPC));

        CLEAR_BIT(PWMAx->SMCR, (PWMA_SMCR_ETP | PWMA_SMCR_ECE | PWMA_SMCR_ETPS |
                                PWMA_SMCR_ETF | PWMA_SMCR_TS | PWMA_SMCR_SMS));

        WRITE_REG(PWMAx->CCMR1, 0x0UL);
        WRITE_REG(PWMAx->CCMR2, 0x0UL);
        CLEAR_BIT(PWMAx->CCER, (PWMA_CCER_CC3P | PWMA_CCER_CC3E | PWMA_CCER_CC2NP |
                                PWMA_CCER_CC2NE | PWMA_CCER_CC2P | PWMA_CCER_CC2E |
                                PWMA_CCER_CC1NP | PWMA_CCER_CC1NE | PWMA_CCER_CC1P |
                                PWMA_CCER_CC1E | PWMA_CCER_CC0NP | PWMA_CCER_CC0NE |
                                PWMA_CCER_CC0P | PWMA_CCER_CC0E));

        CLEAR_BIT(PWMAx->BDTR, (PWMA_BDTR_MOE | PWMA_BDTR_AOE | PWMA_BDTR_BKP |
                                PWMA_BDTR_BKE | PWMA_BDTR_OSSR | PWMA_BDTR_DTG));

        CLEAR_BIT(PWMAx->DCR, (PWMA_DCR_DBL | PWMA_DCR_DBA));
        WRITE_REG(PWMAx->DMAR, 0x0UL);

        WRITE_REG(PWMAx->DIER, PWMA_DIER_INTR_CLEAR);
        WRITE_REG(PWMAx->SR, 0x0UL);

        WRITE_REG(PWMAx->CNT, 0x0UL);

        WRITE_REG(PWMAx->PSC, 0x0UL);
        WRITE_REG(PWMAx->ARR, 0x0UL);
        WRITE_REG(PWMAx->RCR, 0x0UL);

        WRITE_REG(PWMAx->CCR0, 0x0UL);
        WRITE_REG(PWMAx->CCR1, 0x0UL);
        WRITE_REG(PWMAx->CCR2, 0x0UL);
        WRITE_REG(PWMAx->CCR3, 0x0UL);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Initializes the time base unit of a PWMA instance.
 * @note PWMA_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_InitStruct The pointer to PWMA_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA instance has been initialized.
 *         - ERROR: The PWMA instance has not been initialized.
 */
ErrorStatus PWMA_Init(PWMA_TypeDef *PWMAx, PWMA_InitStruct_TypeDef *PWMA_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_COUNTERMODE(PWMA_InitStruct->CounterMode));
    assert(IS_PWMA_CLOCKDIVISION(PWMA_InitStruct->ClockDivision));

    if (PWMA_IsEnabledCounter(PWMAx) == 0UL)
    {
        /* The counter is disabled */

        /* Configure counter */
        MODIFY_REG(PWMAx->CR1,
                   (PWMA_CR1_CKD | PWMA_CR1_ARPE | PWMA_CR1_CMS |
                    PWMA_CR1_DIR | PWMA_CR1_OPM | PWMA_CR1_UDIS),
                   ((uint32_t)PWMA_InitStruct->ClockDivision |
                    (uint32_t)PWMA_InitStruct->CounterMode));

        WRITE_REG(PWMAx->DIER, PWMA_DIER_INTR_CLEAR);
        WRITE_REG(PWMAx->SR, 0x0UL);

        WRITE_REG(PWMAx->CNT, 0x0UL);

        WRITE_REG(PWMAx->PSC, (uint32_t)PWMA_InitStruct->Prescaler);
        WRITE_REG(PWMAx->ARR, (uint32_t)PWMA_InitStruct->Autoreload);
        WRITE_REG(PWMAx->RCR, (uint32_t)PWMA_InitStruct->RepetitionCounter);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of PWMA_InitStruct_TypeDef structure to default value.
 * @param PWMA_InitStruct The pointer to PWMA_InitStruct_TypeDef structure.
 */
void PWMA_StructInit(PWMA_InitStruct_TypeDef *PWMA_InitStruct)
{
    /* Set PWMA_InitStruct fields to default values */
    PWMA_InitStruct->Prescaler         = 0x0U;
    PWMA_InitStruct->CounterMode       = PWMA_COUNTERMODE_UP;
    PWMA_InitStruct->Autoreload        = 0xFFFFU;
    PWMA_InitStruct->ClockDivision     = PWMA_CLOCKDIVISION_DIV1;
    PWMA_InitStruct->RepetitionCounter = 0x0U;
}

/**
 * @brief Configures the PWMA output channel.
 * @note PWMA_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param Channel The output channel. Can be one of the PWMA_CHx values.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA output channel has been initialized.
 *         - ERROR: The PWMA output channel has not been initialized.
 */
ErrorStatus PWMA_OC_Init(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    switch (Channel)
    {
        case PWMA_CH0:
            status = OC0Config(PWMAx, PWMA_OC_InitStruct);
            break;
        case PWMA_CH1:
            status = OC1Config(PWMAx, PWMA_OC_InitStruct);
            break;
        case PWMA_CH2:
            status = OC2Config(PWMAx, PWMA_OC_InitStruct);
            break;
        case PWMA_CH3:
            status = OC3Config(PWMAx, PWMA_OC_InitStruct);
            break;
        default:
            break;
    }

    return status;
}

/**
 * @brief Sets each field of PWMA_OC_InitStruct_TypeDef structure to default value.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 */
void PWMA_OC_StructInit(PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Set PWMA_OC_InitStruct fields to default values */
    PWMA_OC_InitStruct->OCMode       = PWMA_OC_MODE_FROZEN;
    PWMA_OC_InitStruct->OCState      = PWMA_OC_STATE_DISABLE;
    PWMA_OC_InitStruct->OCNState     = PWMA_OC_STATE_DISABLE;
    PWMA_OC_InitStruct->CompareValue = 0x0U;
    PWMA_OC_InitStruct->OCPolarity   = PWMA_OC_POLARITY_HIGH;
    PWMA_OC_InitStruct->OCNPolarity  = PWMA_OC_POLARITY_HIGH;
    PWMA_OC_InitStruct->OCIdleState  = PWMA_OC_IDLESTATE_LOW;
    PWMA_OC_InitStruct->OCNIdleState = PWMA_OC_IDLESTATE_LOW;
}

/**
 * @brief Configures the PWMA input channel.
 * @note PWMA_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param Channel The input channel. Can be one of the PWMA_CHx values.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA input channel has been initialized.
 *         - ERROR: The PWMA input channel has not been initialized.
 */
ErrorStatus PWMA_IC_Init(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    ErrorStatus result = ERROR;

    switch (Channel)
    {
        case PWMA_CH0:
            result = IC0Config(PWMAx, PWMA_IC_InitStruct);
            break;
        case PWMA_CH1:
            result = IC1Config(PWMAx, PWMA_IC_InitStruct);
            break;
        case PWMA_CH2:
            result = IC2Config(PWMAx, PWMA_IC_InitStruct);
            break;
        case PWMA_CH3:
            result = IC3Config(PWMAx, PWMA_IC_InitStruct);
            break;
        default:
            break;
    }

    return result;
}

/**
 * @brief Sets each field of PWMA_IC_InitStruct_TypeDef structure to default value.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 */
void PWMA_IC_StructInit(PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Set PWMA_IC_InitStruct fields to default values */
    PWMA_IC_InitStruct->ICPolarity    = PWMA_IC_POLARITY_RISING;
    PWMA_IC_InitStruct->ICActiveInput = PWMA_IC_ACTIVEINPUT_DIRECTTI;
    PWMA_IC_InitStruct->ICPrescaler   = PWMA_IC_PRESCALER_DIV1;
    PWMA_IC_InitStruct->ICFilter      = PWMA_IC_FILTER_FDIV1;
}

/**
 * @brief Configures the External Trigger feature of a PWMA instance.
 * @note PWMA_ETR_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_ETR_InitStruct The pointer to PWMA_ETR_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The External Trigger feature has been configured.
 *         - ERROR: The External Trigger feature has not been configured.
 */
ErrorStatus PWMA_ETR_Init(PWMA_TypeDef *PWMAx, PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct)
{
    /* Check parameters */
    assert(IS_PWMA_CLOCKSOURCE(PWMA_ETR_InitStruct->ClockSource));
    assert(IS_PWMA_SLAVEMODE(PWMA_ETR_InitStruct->SlaveMode));
    assert(IS_PWMA_TS(PWMA_ETR_InitStruct->TriggerInput));
    assert(IS_PWMA_ETR_POLARITY(PWMA_ETR_InitStruct->ETRPolarity));
    assert(IS_PWMA_ETR_PRESCALER(PWMA_ETR_InitStruct->ETRPrescaler));
    assert(IS_PWMA_ETR_FILTER(PWMA_ETR_InitStruct->ETRFilter));

    MODIFY_REG(PWMAx->SMCR,
               (PWMA_SMCR_ETP | PWMA_SMCR_ECE | PWMA_SMCR_ETPS |
                PWMA_SMCR_ETF | PWMA_SMCR_TS | PWMA_SMCR_SMS),
               ((uint32_t)PWMA_ETR_InitStruct->ETRPolarity |
                (uint32_t)PWMA_ETR_InitStruct->ETRPrescaler |
                (uint32_t)PWMA_ETR_InitStruct->ETRFilter |
                (uint32_t)PWMA_ETR_InitStruct->TriggerInput |
                ((PWMA_ETR_InitStruct->ClockSource == PWMA_CLOCKSOURCE_EXT_MODE1) ?
                    ((uint32_t)PWMA_ETR_InitStruct->ClockSource) :
                    ((uint32_t)PWMA_ETR_InitStruct->ClockSource |
                     (uint32_t)PWMA_ETR_InitStruct->SlaveMode))));

    return SUCCESS;
}

/**
 * @brief Sets each field of PWMA_ETR_InitStruct_TypeDef structure to default value.
 * @param PWMA_ETR_InitStruct The pointer to PWMA_ETR_InitStruct_TypeDef structure.
 */
void PWMA_ETR_StructInit(PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct)
{
    /* Set PWMA_ETR_InitStruct fields to default values */
    PWMA_ETR_InitStruct->ClockSource  = PWMA_CLOCKSOURCE_INTERNAL;
    PWMA_ETR_InitStruct->SlaveMode    = PWMA_SLAVEMODE_DISABLED;
    PWMA_ETR_InitStruct->TriggerInput = PWMA_TS_ITR0;
    PWMA_ETR_InitStruct->ETRPolarity  = PWMA_ETR_POLARITY_NONINVERTED;
    PWMA_ETR_InitStruct->ETRPrescaler = PWMA_ETR_PRESCALER_DIV1;
    PWMA_ETR_InitStruct->ETRFilter    = PWMA_ETR_FILTER_FDIV1;
}

/**
 * @brief Configures the Break and Dead Time feature of a PWMA instance.
 * @note PWMA_BDTR_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_BDTR_InitStruct The pointer to PWMA_BDTR_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The Break and Dead Time feature has been configured.
 *         - ERROR: The Break and Dead Time feature has not been configured.
 */
ErrorStatus PWMA_BDTR_Init(PWMA_TypeDef *PWMAx, PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct)
{
    /* Check parameters */
    assert(IS_PWMA_OSSR_STATE(PWMA_BDTR_InitStruct->OSSRState));
    assert(IS_PWMA_BRK_STATE(PWMA_BDTR_InitStruct->BreakState));
    assert(IS_PWMA_BRK_POLARITY(PWMA_BDTR_InitStruct->BreakPolarity));
    assert(IS_PWMA_AUTOMATICOUTPUT(PWMA_BDTR_InitStruct->AutomaticOutput));

    MODIFY_REG(PWMAx->BDTR,
               (PWMA_BDTR_AOE | PWMA_BDTR_BKP | PWMA_BDTR_BKE |
                PWMA_BDTR_OSSR |PWMA_BDTR_DTG),
               (uint32_t)PWMA_BDTR_InitStruct->AutomaticOutput |
               (uint32_t)PWMA_BDTR_InitStruct->BreakPolarity |
               (uint32_t)PWMA_BDTR_InitStruct->BreakState |
               (uint32_t)PWMA_BDTR_InitStruct->OSSRState |
               (uint32_t)PWMA_BDTR_InitStruct->DeadTime);

    return SUCCESS;
}

/**
 * @brief Sets each field of PWMA_BDTR_InitStruct_TypeDef structure to default value.
 * @param PWMA_BDTR_InitStruct The pointer to PWMA_BDTR_InitStruct_TypeDef structure.
 */
void PWMA_BDTR_StructInit(PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct)
{
    /* Set PWMA_BDTR_InitStruct fields to default values */
    PWMA_BDTR_InitStruct->OSSRState       = PWMA_OSSR_DISABLE;
    PWMA_BDTR_InitStruct->DeadTime        = 0x0U;
    PWMA_BDTR_InitStruct->BreakState      = PWMA_BRK_DISABLE;
    PWMA_BDTR_InitStruct->BreakPolarity   = PWMA_BRK_POLARITY_LOW;
    PWMA_BDTR_InitStruct->AutomaticOutput = PWMA_AUTOMATICOUTPUT_DISABLE;
}

/**
 * @brief Configures the PWMA output channel 0.
 * @note PWMA_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA output channel has been initialized.
 *         - ERROR: The PWMA output channel has not been initialized.
 */
ErrorStatus OC0Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH0) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH0N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CR2,
                   (PWMA_CR2_OIS0N | PWMA_CR2_OIS0),
                   (((uint32_t)PWMA_OC_InitStruct->OCNIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH0N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)])));

        MODIFY_REG(PWMAx->CCMR1,
                   (PWMA_CCMR1_OC_OC0CE | PWMA_CCMR1_OC_OC0M |
                    PWMA_CCMR1_OC_OC0PE | PWMA_CCMR1_CC0S),
                   (((uint32_t)PWMA_OC_InitStruct->OCMode << SHIFT_TAB_OCxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)]) |
                    PWMA_CCMR1_CC0S_OC));

        WRITE_REG(PWMAx->CCR0, PWMA_OC_InitStruct->CompareValue);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0NP | PWMA_CCER_CC0NE | PWMA_CCER_CC0P | PWMA_CCER_CC0E),
                   (((uint32_t)PWMA_OC_InitStruct->OCNPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH0N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCNState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH0N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)])));

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA output channel 1.
 * @note PWMA_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA output channel has been initialized.
 *         - ERROR: The PWMA output channel has not been initialized.
 */
ErrorStatus OC1Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH1) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH1N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CR2,
                   (PWMA_CR2_OIS1N | PWMA_CR2_OIS1),
                   (((uint32_t)PWMA_OC_InitStruct->OCNIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH1N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)])));

        MODIFY_REG(PWMAx->CCMR1,
                   (PWMA_CCMR1_OC_OC1CE | PWMA_CCMR1_OC_OC1M |
                    PWMA_CCMR1_OC_OC1PE | PWMA_CCMR1_CC1S),
                   (((uint32_t)PWMA_OC_InitStruct->OCMode << SHIFT_TAB_OCxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)]) |
                    PWMA_CCMR1_CC1S_OC));

        WRITE_REG(PWMAx->CCR1, PWMA_OC_InitStruct->CompareValue);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC1NP | PWMA_CCER_CC1NE | PWMA_CCER_CC1P | PWMA_CCER_CC1E),
                   (((uint32_t)PWMA_OC_InitStruct->OCNPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH1N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCNState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH1N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)])));

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA output channel 2.
 * @note PWMA_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA output channel has been initialized.
 *         - ERROR: The PWMA output channel has not been initialized.
 */
ErrorStatus OC2Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH2) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH2N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CR2,
                   (PWMA_CR2_OIS2N | PWMA_CR2_OIS2),
                   (((uint32_t)PWMA_OC_InitStruct->OCNIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH2N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCIdleState << SHIFT_TAB_OISx[PWMA_GET_CHANNEL_INDEX(PWMA_CH2)])));

        MODIFY_REG(PWMAx->CCMR2,
                   (PWMA_CCMR2_OC_OC2CE | PWMA_CCMR2_OC_OC2M |
                    PWMA_CCMR2_OC_OC2PE | PWMA_CCMR2_CC2S),
                   (((uint32_t)PWMA_OC_InitStruct->OCMode << SHIFT_TAB_OCxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH2)]) |
                    PWMA_CCMR2_CC2S_OC));

        WRITE_REG(PWMAx->CCR2, PWMA_OC_InitStruct->CompareValue);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC2NP | PWMA_CCER_CC2NE | PWMA_CCER_CC2P | PWMA_CCER_CC2E),
                   (((uint32_t)PWMA_OC_InitStruct->OCNPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH2N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCNState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH2N)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH2)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH2)])));

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA output channel 3.
 * @note PWMA_OC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_OC_InitStruct The pointer to PWMA_OC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA output channel has been initialized.
 *         - ERROR: The PWMA output channel has not been initialized.
 */
ErrorStatus OC3Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    if (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH3) == 0UL)
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CCMR2,
                   (PWMA_CCMR2_OC_OC3CE | PWMA_CCMR2_OC_OC3M |
                    PWMA_CCMR2_OC_OC3PE | PWMA_CCMR2_CC3S),
                   (((uint32_t)PWMA_OC_InitStruct->OCMode << SHIFT_TAB_OCxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH3)]) |
                    PWMA_CCMR2_CC3S_OC));

        WRITE_REG(PWMAx->CCR3, PWMA_OC_InitStruct->CompareValue);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0P | PWMA_CCER_CC0E),
                   (((uint32_t)PWMA_OC_InitStruct->OCPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH3)]) |
                    ((uint32_t)PWMA_OC_InitStruct->OCState << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH3)])));

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA input channel 0.
 * @note PWMA_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA input channel has been initialized.
 *         - ERROR: The PWMA input channel has not been initialized.
 */
ErrorStatus IC0Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH0) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH0N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CCMR1,
                   (PWMA_CCMR1_IC_IC0F | PWMA_CCMR1_IC_IC0PSC | PWMA_CCMR1_CC0S),
                   ((uint32_t)PWMA_IC_InitStruct->ICFilter |
                    (uint32_t)PWMA_IC_InitStruct->ICPrescaler |
                    (uint32_t)PWMA_IC_InitStruct->ICActiveInput) << SHIFT_TAB_ICxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)]);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0NP | PWMA_CCER_CC0P),
                   (uint32_t)PWMA_IC_InitStruct->ICPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH0)]);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA input channel 1.
 * @note PWMA_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA input channel has been initialized.
 *         - ERROR: The PWMA input channel has not been initialized.
 */
ErrorStatus IC1Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH1) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH1N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CCMR1,
                   (PWMA_CCMR1_IC_IC1F | PWMA_CCMR1_IC_IC1PSC | PWMA_CCMR1_CC1S),
                   ((uint32_t)PWMA_IC_InitStruct->ICFilter |
                    (uint32_t)PWMA_IC_InitStruct->ICPrescaler |
                    (uint32_t)PWMA_IC_InitStruct->ICActiveInput) << SHIFT_TAB_ICxx[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)]);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC1NP | PWMA_CCER_CC1P),
                   (uint32_t)PWMA_IC_InitStruct->ICPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH1)]);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA input channel 2.
 * @note PWMA_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA input channel has been initialized.
 *         - ERROR: The PWMA input channel has not been initialized.
 */
ErrorStatus IC2Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    if ((PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH2) == 0UL) &&
        (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH2N) == 0UL))
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CCMR2,
                   (PWMA_CCMR2_IC_IC2F | PWMA_CCMR2_IC_IC2PSC | PWMA_CCMR2_CC2S),
                   ((uint32_t)PWMA_IC_InitStruct->ICFilter |
                    (uint32_t)PWMA_IC_InitStruct->ICPrescaler |
                    (uint32_t)PWMA_IC_InitStruct->ICActiveInput) << SHIFT_TAB_ICxx[PWMA_CH2]);

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC2NP | PWMA_CCER_CC2P),
                   (uint32_t)PWMA_IC_InitStruct->ICPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH2)]);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Configures the PWMA input channel 3.
 * @note PWMA_IC_InitStruct structure should be initialized prior to calling this function.
 * @param PWMAx The PWMA instance.
 * @param PWMA_IC_InitStruct The pointer to PWMA_IC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The PWMA input channel has been initialized.
 *         - ERROR: The PWMA input channel has not been initialized.
 */
ErrorStatus IC3Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    if (PWMA_CC_IsEnabledChannel(PWMAx, PWMA_CH3) == 0UL)
    {
        /* The capture/compare channel is disabled */

        /* Configure the capture/compare channel */
        MODIFY_REG(PWMAx->CCMR2,
                   (PWMA_CCMR2_IC_IC3F | PWMA_CCMR2_IC_IC3PSC | PWMA_CCMR2_CC3S),
                   ((uint32_t)PWMA_IC_InitStruct->ICFilter |
                    (uint32_t)PWMA_IC_InitStruct->ICPrescaler |
                    (uint32_t)PWMA_IC_InitStruct->ICActiveInput) << SHIFT_TAB_ICxx[PWMA_CH3]);

        MODIFY_REG(PWMAx->CCER,
                   PWMA_CCER_CC3P,
                   (uint32_t)PWMA_IC_InitStruct->ICPolarity << SHIFT_TAB_CCxP[PWMA_GET_CHANNEL_INDEX(PWMA_CH3)]);

        status = SUCCESS;
    }

    return status;
}

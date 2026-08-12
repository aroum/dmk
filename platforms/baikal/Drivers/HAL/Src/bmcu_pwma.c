/**
 * *****************************************************************************
 *  @file       bmcu_pwma.c
 *  @author     Baikal electronics SDK team
 *  @brief      PWMA timer (PWMA) module driver
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - PWMA Init/Deinit functions implementation
 *      - Definitions for PWMA function assertions check
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

/* Private function prototypes -----------------------------------------------*/

static ErrorStatus OC0Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC1Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC2Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus OC3Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
static ErrorStatus IC0Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC1Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC2Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
static ErrorStatus IC3Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);

/**
 * @brief  Set PWMAx registers to their reset values.
 * @param  PWMAx Timer instance
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx registers are de-initialized
 *          - ERROR: invalid PWMAx instance
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
 * @brief  Configure the PWMAx time base unit.
 * @param  PWMAx Timer Instance
 * @param  PWMA_InitStruct pointer to a @ref PWMA_InitStruct_TypeDef structure (PWMAx time base unit configuration data structure)
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx registers are de-initialized
 *          - ERROR: not applicable
 */
ErrorStatus PWMA_Init(PWMA_TypeDef *PWMAx, PWMA_InitStruct_TypeDef *PWMA_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_COUNTERMODE(PWMA_InitStruct->CounterMode));
    assert(IS_PWMA_CLOCKDIVISION(PWMA_InitStruct->ClockDivision));

    uint32_t tmpcr1 = READ_REG(PWMAx->CR1);

    /* Select the Counter Mode */
    MODIFY_REG(tmpcr1, (PWMA_CR1_DIR | PWMA_CR1_CMS), (uint32_t)PWMA_InitStruct->CounterMode);

    /* Set the clock division */
    MODIFY_REG(tmpcr1, PWMA_CR1_CKD, (uint32_t)PWMA_InitStruct->ClockDivision);

    /* Write to PWMAx CR1 */
    WRITE_REG(PWMAx->CR1, tmpcr1);

    /* Set the Autoreload value */
    PWMA_SetAutoReload(PWMAx, (uint32_t)PWMA_InitStruct->Autoreload);

    /* Set the Prescaler value */
    PWMA_SetPrescaler(PWMAx, (uint32_t)PWMA_InitStruct->Prescaler);

    /* Set the Repetition Counter value */
    PWMA_SetRepetitionCounter(PWMAx, (uint32_t)PWMA_InitStruct->RepetitionCounter);

    /* Generate an update event to reload the Prescaler
       and the repetition counter value (if applicable) immediately */
    PWMA_GenerateEvent_UPDATE(PWMAx);

    return SUCCESS;
}

/**
 * @brief  Set the fields of the time base unit configuration data structure
 *         to their default values.
 * @param  PWMA_InitStruct pointer to a @ref PWMA_InitStruct_TypeDef structure (time base unit configuration data structure)
 * @retval None
 */
void PWMA_StructInit(PWMA_InitStruct_TypeDef *PWMA_InitStruct)
{
    /* Set the default configuration */
    PWMA_InitStruct->Prescaler         = 0x0U;
    PWMA_InitStruct->CounterMode       = PWMA_COUNTERMODE_UP;
    PWMA_InitStruct->Autoreload        = 0xFFFFU;
    PWMA_InitStruct->ClockDivision     = PWMA_CLOCKDIVISION_DIV1;
    PWMA_InitStruct->RepetitionCounter = 0x0U;
}

/**
 * @brief  Configure the PWMAx output channel.
 * @param  PWMAx Timer Instance
 * @param  Channel This parameter can be one of the following values:
 *         @arg @ref PWMA_CH0
 *         @arg @ref PWMA_CH1
 *         @arg @ref PWMA_CH2
 *         @arg @ref PWMA_CH3
 * @param  PWMA_OC_InitStruct pointer to a @ref PWMA_OC_InitStruct_TypeDef structure (PWMAx output channel configuration data structure)
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx output channel is initialized
 *          - ERROR: PWMAx output channel is not initialized
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
 * @brief  Set the fields of the PWMAx output channel configuration data
 *         structure to their default values.
 * @param  PWMA_OC_InitStruct pointer to a @ref PWMA_OC_InitStruct_TypeDef structure (the output channel configuration data structure)
 * @retval None
 */
void PWMA_OC_StructInit(PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Set the default configuration */
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
 * @brief  Configure the PWMAx input channel.
 * @param  PWMAx Timer Instance
 * @param  Channel This parameter can be one of the following values:
 *         @arg @ref PWMA_CH0
 *         @arg @ref PWMA_CH1
 *         @arg @ref PWMA_CH2
 *         @arg @ref PWMA_CH3
 * @param  PWMA_IC_InitStruct pointer to a @ref PWMA_IC_InitStruct_TypeDef structure (PWMAx input channel configuration data structure)
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx input channel is initialized
 *          - ERROR: PWMAx input channel is not initialized
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
 * @brief  Set the fields of the PWMAx input channel configuration data
 *         structure to their default values.
 * @param  PWMA_IC_InitStruct pointer to a @ref PWMA_IC_InitStruct_TypeDef structure (the input channel configuration data structure)
 * @retval None
 */
void PWMA_IC_StructInit(PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Set the default configuration */
    PWMA_IC_InitStruct->ICPolarity    = PWMA_IC_POLARITY_RISING;
    PWMA_IC_InitStruct->ICActiveInput = PWMA_IC_ACTIVEINPUT_DIRECTTI;
    PWMA_IC_InitStruct->ICPrescaler   = PWMA_IC_PRESCALER_DIV1;
    PWMA_IC_InitStruct->ICFilter      = PWMA_IC_FILTER_FDIV1;
}

/**
 * @brief Configure the External Trigger feature of the timer instance.
 * @param  PWMAx Timer Instance
 * @param  PWMA_ETR_InitStruct pointer to a @ref PWMA_ETR_InitStruct_TypeDef structure (External Trigger configuration data structure)
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: External Trigger feature is initialized
 *          - ERROR: not applicable
 */
ErrorStatus PWMA_ETR_Init(PWMA_TypeDef *PWMAx, PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_CLOCKSOURCE(PWMA_ETR_InitStruct->ClockSource));
    assert(IS_PWMA_SLAVEMODE(PWMA_ETR_InitStruct->SlaveMode));
    assert(IS_PWMA_TS(PWMA_ETR_InitStruct->TriggerInput));
    assert(IS_PWMA_ETR_POLARITY(PWMA_ETR_InitStruct->ETRPolarity));
    assert(IS_PWMA_ETR_PRESCALER(PWMA_ETR_InitStruct->ETRPrescaler));
    assert(IS_PWMA_ETR_FILTER(PWMA_ETR_InitStruct->ETRFilter));

    uint32_t tmpsmcr = READ_REG(PWMAx->SMCR);

    /* Configure the external trigger signal */
    MODIFY_REG(tmpsmcr, PWMA_SMCR_ETP, (uint32_t)PWMA_ETR_InitStruct->ETRPolarity);
    MODIFY_REG(tmpsmcr, PWMA_SMCR_ETPS, (uint32_t)PWMA_ETR_InitStruct->ETRPrescaler);
    MODIFY_REG(tmpsmcr, PWMA_SMCR_ETF, (uint32_t)PWMA_ETR_InitStruct->ETRFilter);

    /* Configure the input synchronization signal */
    MODIFY_REG(tmpsmcr, PWMA_SMCR_TS, (uint32_t)PWMA_ETR_InitStruct->TriggerInput);

    /* Configure the input clock source and timer slave mode */
    if (PWMA_ETR_InitStruct->ClockSource == PWMA_CLOCKSOURCE_EXT_MODE2)
    {
        MODIFY_REG(tmpsmcr,
                   (PWMA_SMCR_ECE | PWMA_SMCR_SMS),
                   ((uint32_t)PWMA_ETR_InitStruct->ClockSource |
                    (uint32_t)PWMA_ETR_InitStruct->SlaveMode));
    }
    else
    {
        MODIFY_REG(tmpsmcr,
                   (PWMA_SMCR_ECE | PWMA_SMCR_SMS),
                   (uint32_t)PWMA_ETR_InitStruct->SlaveMode);
    }

    WRITE_REG(PWMAx->SMCR, tmpsmcr);

    return SUCCESS;
}

/**
 * @brief  Set the fields of the External Trigger configuration data structure
 *         to their default values.
 * @param  PWMA_ETR_InitStruct pointer to a @ref PWMA_ETR_InitStruct_TypeDef structure (External Trigger configuration data structure)
 * @retval None
 */
void PWMA_ETR_StructInit(PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct)
{
    /* Set the default configuration */
    PWMA_ETR_InitStruct->ClockSource  = PWMA_CLOCKSOURCE_INTERNAL;
    PWMA_ETR_InitStruct->SlaveMode    = PWMA_SLAVEMODE_DISABLED;
    PWMA_ETR_InitStruct->ETRPolarity  = PWMA_ETR_POLARITY_NONINVERTED;
    PWMA_ETR_InitStruct->ETRPrescaler = PWMA_ETR_PRESCALER_DIV1;
    PWMA_ETR_InitStruct->ETRFilter    = PWMA_ETR_FILTER_FDIV1;
}

/**
 * @brief  Configure the Break and Dead Time feature of the timer instance.
 * @note As the bits AOE, BKP, BKE, OSSR, OSSI and DTG[7:0] can be write-locked
 *  depending on the LOCK configuration, it can be necessary to configure all of
 *  them during the first write access to the PWMAx_BDTR register.
 * @note Macro @ref IS_PWMA_BREAK_INSTANCE(PWMAx) can be used to check whether or not
 *       a timer instance provides a break input.
 * @param  PWMAx Timer Instance
 * @param  PWMA_BDTR_InitStruct pointer to a @ref PWMA_BDTR_InitStruct_TypeDef structure (Break and Dead Time configuration data structure)
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: Break and Dead Time are initialized
 *          - ERROR: not applicable
 */
ErrorStatus PWMA_BDTR_Init(PWMA_TypeDef *PWMAx, PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_OSSR_STATE(PWMA_BDTR_InitStruct->OSSRState));
    assert(IS_PWMA_BRK_STATE(PWMA_BDTR_InitStruct->BreakState));
    assert(IS_PWMA_BRK_POLARITY(PWMA_BDTR_InitStruct->BreakPolarity));
    assert(IS_PWMA_AUTOMATICOUTPUT(PWMA_BDTR_InitStruct->AutomaticOutput));

    /* Set the Lock level, the Break enable Bit and the Polarity, the OSSR State,
       the OSSI State, the dead time value and the Automatic Output Enable Bit */
    uint32_t tmpbdtr = READ_REG(PWMAx->BDTR);

    /* Set the BDTR bits */
    MODIFY_REG(tmpbdtr, PWMA_BDTR_DTG, PWMA_BDTR_InitStruct->DeadTime);
    MODIFY_REG(tmpbdtr, PWMA_BDTR_OSSR, (uint32_t)PWMA_BDTR_InitStruct->OSSRState);
    MODIFY_REG(tmpbdtr, PWMA_BDTR_BKE, (uint32_t)PWMA_BDTR_InitStruct->BreakState);
    MODIFY_REG(tmpbdtr, PWMA_BDTR_BKP, (uint32_t)PWMA_BDTR_InitStruct->BreakPolarity);
    MODIFY_REG(tmpbdtr, PWMA_BDTR_AOE, (uint32_t)PWMA_BDTR_InitStruct->AutomaticOutput);

    /* Set PWMAx_BDTR */
    WRITE_REG(PWMAx->BDTR, tmpbdtr);

    return SUCCESS;
}

/**
 * @brief  Set the fields of the Break and Dead Time configuration data structure
 *         to their default values.
 * @param  PWMA_BDTR_InitStruct pointer to a @ref PWMA_BDTR_InitStruct_TypeDef structure (Break and Dead Time configuration data structure)
 * @retval None
 */
void PWMA_BDTR_StructInit(PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct)
{
    /* Set the default configuration */
    PWMA_BDTR_InitStruct->OSSRState       = PWMA_OSSR_DISABLE;
    PWMA_BDTR_InitStruct->DeadTime        = 0x0U;
    PWMA_BDTR_InitStruct->BreakState      = PWMA_BRK_DISABLE;
    PWMA_BDTR_InitStruct->BreakPolarity   = PWMA_BRK_POLARITY_LOW;
    PWMA_BDTR_InitStruct->AutomaticOutput = PWMA_AUTOMATICOUTPUT_DISABLE;
}

/**
 * @brief  Configure the PWMAx output channel 0.
 * @param  PWMAx Timer Instance
 * @param  PWMA_OC_InitStruct pointer to the the PWMAx output channel 0 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx output channel is initialized
 *          - ERROR: PWMAx output channel is not initialized
 */
ErrorStatus OC0Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    /* Disable the Channel 0: Reset the CC0E Bit */
    CLEAR_BIT(PWMAx->CCER, PWMA_CCER_CC0E);

    /* Get the PWMAx CCER register value */
    uint32_t tmpccer = READ_REG(PWMAx->CCER);

    /* Get the PWMAx CR2 register value */
    uint32_t tmpcr2 = READ_REG(PWMAx->CR2);

    /* Get the PWMAx CCMR1 register value */
    uint32_t tmpccmr1 = READ_REG(PWMAx->CCMR1);

    /* Reset Capture/Compare selection Bits */
    CLEAR_BIT(tmpccmr1, PWMA_CCMR1_CC0S);

    /* Set the Output Compare Mode */
    MODIFY_REG(tmpccmr1, PWMA_CCMR1_OC_OC0M, (uint32_t)PWMA_OC_InitStruct->OCMode);

    /* Set the Output Compare Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC0P, (uint32_t)PWMA_OC_InitStruct->OCPolarity);

    /* Set the Output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC0E, (uint32_t)PWMA_OC_InitStruct->OCState);

    /* Set the complementary output Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC0NP, (uint32_t)PWMA_OC_InitStruct->OCNPolarity << 2);

    /* Set the complementary output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC0NE, (uint32_t)PWMA_OC_InitStruct->OCNState << 2);

    /* Set the Output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS0, (uint32_t)PWMA_OC_InitStruct->OCIdleState);

    /* Set the complementary output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS0N, (uint32_t)PWMA_OC_InitStruct->OCNIdleState << 1);

    /* Write to PWMAx CR2 */
    WRITE_REG(PWMAx->CR2, tmpcr2);

    /* Write to PWMAx CCMR1 */
    WRITE_REG(PWMAx->CCMR1, tmpccmr1);

    /* Set the Capture Compare Register value */
    PWMA_OC_SetCompareCH0(PWMAx, PWMA_OC_InitStruct->CompareValue);

    /* Write to PWMAx CCER */
    WRITE_REG(PWMAx->CCER, tmpccer);

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx output channel 1.
 * @param  PWMAx Timer Instance
 * @param  PWMA_OC_InitStruct pointer to the the PWMAx output channel 1 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx output channel is initialized
 *          - ERROR: PWMAx output channel is not initialized
 */
ErrorStatus OC1Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    /* Disable the Channel 1: Reset the CC1E Bit */
    CLEAR_BIT(PWMAx->CCER, PWMA_CCER_CC1E);

    /* Get the PWMAx CCER register value */
    uint32_t tmpccer =  READ_REG(PWMAx->CCER);

    /* Get the PWMAx CR2 register value */
    uint32_t tmpcr2 = READ_REG(PWMAx->CR2);

    /* Get the PWMAx CCMR1 register value */
    uint32_t tmpccmr1 = READ_REG(PWMAx->CCMR1);

    /* Reset Capture/Compare selection Bits */
    CLEAR_BIT(tmpccmr1, PWMA_CCMR1_CC1S);

    /* Select the Output Compare Mode */
    MODIFY_REG(tmpccmr1, PWMA_CCMR1_OC_OC1M, (uint32_t)PWMA_OC_InitStruct->OCMode << 8);

    /* Set the Output Compare Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC1P, (uint32_t)PWMA_OC_InitStruct->OCPolarity << 4);

    /* Set the Output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC1E, (uint32_t)PWMA_OC_InitStruct->OCState << 4);

    /* Set the complementary output Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC1NP, (uint32_t)PWMA_OC_InitStruct->OCNPolarity << 6);

    /* Set the complementary output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC1NE, (uint32_t)PWMA_OC_InitStruct->OCNState << 6);

    /* Set the Output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS1, (uint32_t)PWMA_OC_InitStruct->OCIdleState << 2);

    /* Set the complementary output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS1N, (uint32_t)PWMA_OC_InitStruct->OCNIdleState << 3);

    /* Write to PWMAx CR2 */
    WRITE_REG(PWMAx->CR2, tmpcr2);

    /* Write to PWMAx CCMR1 */
    WRITE_REG(PWMAx->CCMR1, tmpccmr1);

    /* Set the Capture Compare Register value */
    PWMA_OC_SetCompareCH1(PWMAx, PWMA_OC_InitStruct->CompareValue);

    /* Write to PWMAx CCER */
    WRITE_REG(PWMAx->CCER, tmpccer);

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx output channel 2.
 * @param  PWMAx Timer Instance
 * @param  PWMA_OC_InitStruct pointer to the the PWMAx output channel 2 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx output channel is initialized
 *          - ERROR: PWMAx output channel is not initialized
 */
ErrorStatus OC2Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCIdleState));
    assert(IS_PWMA_OC_IDLESTATE(PWMA_OC_InitStruct->OCNIdleState));

    /* Disable the Channel 2: Reset the CC2E Bit */
    CLEAR_BIT(PWMAx->CCER, PWMA_CCER_CC2E);

    /* Get the PWMAx CCER register value */
    uint32_t tmpccer =  READ_REG(PWMAx->CCER);

    /* Get the PWMAx CR2 register value */
    uint32_t tmpcr2 = READ_REG(PWMAx->CR2);

    /* Get the PWMAx CCMR2 register value */
    uint32_t tmpccmr2 = READ_REG(PWMAx->CCMR2);

    /* Reset Capture/Compare selection Bits */
    CLEAR_BIT(tmpccmr2, PWMA_CCMR2_CC2S);

    /* Select the Output Compare Mode */
    MODIFY_REG(tmpccmr2, PWMA_CCMR2_OC_OC2M, (uint32_t)PWMA_OC_InitStruct->OCMode);

    /* Set the Output Compare Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC2P, (uint32_t)PWMA_OC_InitStruct->OCPolarity << 8);

    /* Set the Output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC2E, (uint32_t)PWMA_OC_InitStruct->OCState << 8);

    /* Set the complementary output Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC2NP, (uint32_t)PWMA_OC_InitStruct->OCNPolarity << 10);

    /* Set the complementary output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC2NE, (uint32_t)PWMA_OC_InitStruct->OCNState << 10);

    /* Set the Output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS2, (uint32_t)PWMA_OC_InitStruct->OCIdleState << 4);

    /* Set the complementary output Idle state */
    MODIFY_REG(tmpcr2, PWMA_CR2_OIS2N, (uint32_t)PWMA_OC_InitStruct->OCNIdleState << 5);

    /* Write to PWMAx CR2 */
    WRITE_REG(PWMAx->CR2, tmpcr2);

    /* Write to PWMAx CCMR2 */
    WRITE_REG(PWMAx->CCMR2, tmpccmr2);

    /* Set the Capture Compare Register value */
    PWMA_OC_SetCompareCH2(PWMAx, PWMA_OC_InitStruct->CompareValue);

    /* Write to PWMAx CCER */
    WRITE_REG(PWMAx->CCER, tmpccer);

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx output channel 3.
 * @param  PWMAx Timer Instance
 * @param  PWMA_OC_InitStruct pointer to the the PWMAx output channel 3 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx output channel is initialized
 *          - ERROR: PWMAx output channel is not initialized
 */
ErrorStatus OC3Config(PWMA_TypeDef *PWMAx, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_ALL_INSTANCE(PWMAx));
    assert(IS_PWMA_OC_MODE(PWMA_OC_InitStruct->OCMode));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCState));
    assert(IS_PWMA_OC_STATE(PWMA_OC_InitStruct->OCNState));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCPolarity));
    assert(IS_PWMA_OC_POLARITY(PWMA_OC_InitStruct->OCNPolarity));

    /* Disable the Channel 3: Reset the CC3E Bit */
    CLEAR_BIT(PWMAx->CCER, PWMA_CCER_CC3E);

    /* Get the PWMAx CCER register value */
    uint32_t tmpccer = READ_REG(PWMAx->CCER);

    /* Get the PWMAx CR2 register value */
    uint32_t tmpcr2 =  READ_REG(PWMAx->CR2);

    /* Get the PWMAx CCMR2 register value */
    uint32_t tmpccmr2 = READ_REG(PWMAx->CCMR2);

    /* Reset Capture/Compare selection Bits */
    CLEAR_BIT(tmpccmr2, PWMA_CCMR2_CC3S);

    /* Select the Output Compare Mode */
    MODIFY_REG(tmpccmr2, PWMA_CCMR2_OC_OC3M, (uint32_t)PWMA_OC_InitStruct->OCMode << 8);

    /* Set the Output Compare Polarity */
    MODIFY_REG(tmpccer, PWMA_CCER_CC3P, (uint32_t)PWMA_OC_InitStruct->OCPolarity << 12);

    /* Set the Output State */
    MODIFY_REG(tmpccer, PWMA_CCER_CC3E, (uint32_t)PWMA_OC_InitStruct->OCState << 12);

    /* Write to PWMAx CR2 */
    WRITE_REG(PWMAx->CR2, tmpcr2);

    /* Write to PWMAx CCMR2 */
    WRITE_REG(PWMAx->CCMR2, tmpccmr2);

    /* Set the Capture Compare Register value */
    PWMA_OC_SetCompareCH3(PWMAx, PWMA_OC_InitStruct->CompareValue);

    /* Write to PWMAx CCER */
    WRITE_REG(PWMAx->CCER, tmpccer);

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx input channel 0.
 * @param  PWMAx Timer Instance
 * @param  PWMA_IC_InitStruct pointer to the the PWMAx input channel 0 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx input channel is initialized
 *          - ERROR: PWMAx input channel is not initialized
 */
ErrorStatus IC0Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    /* Disable the Channel 0: Reset the CC0E Bit */
    PWMAx->CCER &= ~PWMA_CCER_CC0E;

    /* Select the Input and set the filter and the prescaler value */
    MODIFY_REG(PWMAx->CCMR1,
               (PWMA_CCMR1_CC0S | PWMA_CCMR1_IC_IC0F | PWMA_CCMR1_IC_IC0PSC),
               (PWMA_IC_InitStruct->ICActiveInput | PWMA_IC_InitStruct->ICFilter | PWMA_IC_InitStruct->ICPrescaler) >> 16U);

    /* Select the Polarity and set the CC0E Bit */
    MODIFY_REG(PWMAx->CCER,
               (PWMA_CCER_CC0P | PWMA_CCER_CC0NP),
               (PWMA_IC_InitStruct->ICPolarity | PWMA_CCER_CC0E));

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx input channel 1.
 * @param  PWMAx Timer Instance
 * @param  PWMA_IC_InitStruct pointer to the the PWMAx input channel 1 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx input channel is initialized
 *          - ERROR: PWMAx input channel is not initialized
 */
ErrorStatus IC1Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    /* Disable the Channel 1: Reset the CC1E Bit */
    PWMAx->CCER &= ~PWMA_CCER_CC1E;

    /* Select the Input and set the filter and the prescaler value */
    MODIFY_REG(PWMAx->CCMR1,
               (PWMA_CCMR1_CC1S | PWMA_CCMR1_IC_IC1F | PWMA_CCMR1_IC_IC1PSC),
               ((uint32_t)PWMA_IC_InitStruct->ICActiveInput | 
                (uint32_t)PWMA_IC_InitStruct->ICFilter | 
                (uint32_t)PWMA_IC_InitStruct->ICPrescaler) >> 8);

    /* Select the Polarity and set the CC1E Bit */
    MODIFY_REG(PWMAx->CCER,
               (PWMA_CCER_CC1P | PWMA_CCER_CC1NP),
               (((uint32_t)PWMA_IC_InitStruct->ICPolarity << 4) | PWMA_CCER_CC1E));

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx input channel 2.
 * @param  PWMAx Timer Instance
 * @param  PWMA_IC_InitStruct pointer to the the PWMAx input channel 2 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx input channel is initialized
 *          - ERROR: PWMAx input channel is not initialized
 */
ErrorStatus IC2Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    /* Disable the Channel 2: Reset the CC2E Bit */
    PWMAx->CCER &= ~PWMA_CCER_CC2E;

    /* Select the Input and set the filter and the prescaler value */
    MODIFY_REG(PWMAx->CCMR2,
               (PWMA_CCMR2_CC2S | PWMA_CCMR2_IC_IC2F | PWMA_CCMR2_IC_IC2PSC),
               ((uint32_t)PWMA_IC_InitStruct->ICActiveInput |
                (uint32_t)PWMA_IC_InitStruct->ICFilter |
                (uint32_t)PWMA_IC_InitStruct->ICPrescaler) >> 16);

    /* Select the Polarity and set the CC2E Bit */
    MODIFY_REG(PWMAx->CCER,
               (PWMA_CCER_CC2P | PWMA_CCER_CC2NP),
               (((uint32_t)PWMA_IC_InitStruct->ICPolarity << 8) | PWMA_CCER_CC2E));

    return SUCCESS;
}

/**
 * @brief  Configure the PWMAx input channel 3.
 * @param  PWMAx Timer Instance
 * @param  PWMA_IC_InitStruct pointer to the the PWMAx input channel 3 configuration data structure
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: PWMAx input channel is initialized
 *          - ERROR: PWMAx input channel is not initialized
 */
ErrorStatus IC3Config(PWMA_TypeDef *PWMAx, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    /* Check the parameters */
    assert(IS_PWMA_IC_POLARITY(PWMA_IC_InitStruct->ICPolarity));
    assert(IS_PWMA_IC_ACTIVEINPUT(PWMA_IC_InitStruct->ICActiveInput));
    assert(IS_PWMA_IC_PRESCALER(PWMA_IC_InitStruct->ICPrescaler));
    assert(IS_PWMA_IC_FILTER(PWMA_IC_InitStruct->ICFilter));

    /* Disable the Channel 3: Reset the CC3E Bit */
    PWMAx->CCER &= ~PWMA_CCER_CC3E;

    /* Select the Input and set the filter and the prescaler value */
    MODIFY_REG(PWMAx->CCMR2,
               (PWMA_CCMR2_CC3S | PWMA_CCMR2_IC_IC3F | PWMA_CCMR2_IC_IC3PSC),
               ((uint32_t)PWMA_IC_InitStruct->ICActiveInput |
                (uint32_t)PWMA_IC_InitStruct->ICFilter |
                (uint32_t)PWMA_IC_InitStruct->ICPrescaler) >> 8);

    /* Select the Polarity and set the CC3E Bit */
    MODIFY_REG(PWMAx->CCER,
               PWMA_CCER_CC3P,
               (((uint32_t)PWMA_IC_InitStruct->ICPolarity << 12) | PWMA_CCER_CC3E));

    return SUCCESS;
}

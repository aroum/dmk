/**
 * *****************************************************************************
 *  @file       bmcu_adc.c
 *  @author     Baikal electronics SDK team
 *  @brief      Analog-to-Digital Converter (ADC) module driver
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

#include "bmcu_adc.h"
#include "bmcu_def.h"
#include "bmcu_cru.h"

#define IS_ADC_CHANNEL(__VALUE__)               (((__VALUE__) == ADC_CH0) || \
                                                 ((__VALUE__) == ADC_CH1) || \
                                                 ((__VALUE__) == ADC_CH2) || \
                                                 ((__VALUE__) == ADC_CH3) || \
                                                 ((__VALUE__) == ADC_CH4) || \
                                                 ((__VALUE__) == ADC_CH5) || \
                                                 ((__VALUE__) == ADC_CH6) || \
                                                 ((__VALUE__) == ADC_CH7))

#define IS_ADC_CONVERSION_MODE(__VALUE__)       (((__VALUE__) == ADC_CONV_SINGLE) || \
                                                 ((__VALUE__) == ADC_CONV_CONTINUOUS))

#define IS_ADC_SEQ_SCAN_MODE(__VALUE__)         (((__VALUE__) == ADC_SEQ_SCAN_DISABLE) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_ENABLE))

#define IS_ADC_SEQ_SCAN_LENGTH(__VALUE__)       (((__VALUE__) == ADC_SEQ_SCAN_LENGTH_1RANK) ||  \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_2RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_3RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_4RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_5RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_6RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_7RANKS) || \
                                                 ((__VALUE__) == ADC_SEQ_SCAN_LENGTH_8RANKS))

#define IS_ADC_SEQ_SCAN_DISCONT_MODE(__VALUE__) (((__VALUE__) == ADC_SEQ_DISCONT_DISABLE) ||    \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_1RANK) ||      \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_2RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_3RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_4RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_5RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_6RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_7RANKS) ||     \
                                                 ((__VALUE__) == ADC_SEQ_DISCONT_8RANKS))

#define IS_ADC_TRIG_SOURCE(__VALUE__)           (((__VALUE__) == ADC_TRIG_SOFTWARE) ||  \
                                                 ((__VALUE__) == ADC_TRIG_EXT_0) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_1) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_2) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_3) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_4) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_5) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_6) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_7) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_8) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_9) ||     \
                                                 ((__VALUE__) == ADC_TRIG_EXT_10) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_11) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_12) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_13) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_14) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_15) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_16) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_17) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_18) ||    \
                                                 ((__VALUE__) == ADC_TRIG_EXT_19))

/**
 * @brief Deinitializes an ADC instance.
 * @param ADCx The ADC instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The ADC instance have been deinitialized.
 *         - ERROR: The ADC instance have not been deinitialized.
 */
ErrorStatus ADC_DeInit(ADC_TypeDef *ADCx)
{
    ErrorStatus status = SUCCESS;
  
    /* Check parameters */
    assert(IS_ADC_ALL_INSTANCE(ADCx));

#if defined (BMCU_U)
    /* Disable the ADC instance if enabled */
    if (ADC_IsEnabled(ADCx) != 0UL)
    {
        /* Disable external conversion trigger */
        ADC_StopConversionExtTrig(ADCx);
        
        /* Disable the ADC instance */
        ADC_Disable(ADCx);
    }
  
    /* Reset registers */
    if (ADC_IsEnabled(ADCx) == 0UL)
    {
        CLEAR_BIT(ADCx->CR1,
                  (ADC_CR1_CKD | ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL |
                   ADC_CR1_DISCNUM | ADC_CR1_DMAEN | ADC_CR1_IEEOC |
                   ADC_CR1_DISCEN | ADC_CR1_SCANEN | ADC_CR1_CONT));

        SET_BIT(ADCx->ASTR, (ADC_ASTR_STRT | ADC_ASTR_EOC));

        MODIFY_REG(ADCx->ASER,
                   (ADC_ASER_TEST_EN | ADC_ASER_ADJ_TD_GA | ADC_ASER_ADJ_TD_OS |
                    ADC_ASER_SELDO_HS_LU | ADC_ASER_TS_MODE | ADC_ASER_SELVI_HD_LS),
                   (((uint32_t)ADC_TS_GAIN_DEFAULT << ADC_ASER_ADJ_TD_GA_Pos) | 
                    ((uint32_t)ADC_TS_OFFSET_DEFAULT << ADC_ASER_ADJ_TD_OS_Pos) |
                    ADC_ASER_TS_MODE_AVG256));
        
        CLEAR_BIT(ADCx->ASMPR,
                  (ADC_ASMPR_SMP7 | ADC_ASMPR_SMP6 | ADC_ASMPR_SMP5 |
                   ADC_ASMPR_SMP4 | ADC_ASMPR_SMP3 | ADC_ASMPR_SMP2 |
                   ADC_ASMPR_SMP1 | ADC_ASMPR_SMP0));
        
        CLEAR_BIT(ADCx->ASQR,
                  (ADC_ASQR_L | ADC_ASQR_SQ7 | ADC_ASQR_SQ6 |
                   ADC_ASQR_SQ5 | ADC_ASQR_SQ4 | ADC_ASQR_SQ3 |
                   ADC_ASQR_SQ2 | ADC_ASQR_SQ1 | ADC_ASQR_SQ0));
    }
    else
    {
        status = ERROR;
    }
#endif

#if defined (BE_U1000)
    if (ADCx == ADC0)
    {
        CRU_APB2_ResetPeriph(CRU_APB2_PERIPH_ADC0);
    }
    else if (ADCx == ADC1)
    {
        CRU_APB2_ResetPeriph(CRU_APB2_PERIPH_ADC1);
    }
    else if (ADCx == ADC2)
    {
        CRU_APB2_ResetPeriph(CRU_APB2_PERIPH_ADC2);
    }
    else
    {
        status = ERROR;
    }
#endif
  
    return status;
}

/**
 * @brief Initializes an ADC instance.
 * @note ADC_InitStruct structure should be initialized prior to calling this function.
 * @param ADCx The ADC instance.
 * @param ADC_InitStruct The pointer to ADC_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The ADC instance have been initialized.
 *         - ERROR: The ADC instance have not been initialized.
 */
ErrorStatus ADC_Init(ADC_TypeDef *ADCx, ADC_InitStruct_TypeDef *ADC_InitStruct)
{
    ErrorStatus status = ERROR;
  
    /* Check parameters */
    assert(IS_ADC_ALL_INSTANCE(ADCx));
    assert(IS_ADC_CONVERSION_MODE(ADC_InitStruct->ConversionMode));
    assert(IS_ADC_SEQ_SCAN_MODE(ADC_InitStruct->SequencerScanMode));
    assert(IS_ADC_SEQ_SCAN_LENGTH(ADC_InitStruct->SequencerLength));
    assert(IS_ADC_SEQ_SCAN_DISCONT_MODE(ADC_InitStruct->SequencerDiscontMode));
    assert(IS_ADC_TRIG_SOURCE(ADC_InitStruct->TriggerSource));
    assert(IS_FUNCTIONAL_STATE(ADC_InitStruct->DMATransfer));
  
    if (ADC_IsEnabled(ADCx) == 0UL)
    {
        /* ADC instance is disabled */

        MODIFY_REG(ADCx->CR1,
                   (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL | ADC_CR1_DMAEN |
                    ADC_CR1_DISCEN | ADC_CR1_SCANEN | ADC_CR1_CONT),
                   ((uint32_t)ADC_InitStruct->TriggerSource |
                    ((uint32_t)ADC_InitStruct->DMATransfer << ADC_CR1_DMAEN_Pos) |
                    (uint32_t)ADC_InitStruct->SequencerScanMode |
                    (uint32_t)ADC_InitStruct->ConversionMode));
        
        if (ADC_InitStruct->SequencerScanMode != ADC_CR1_SCANEN_DISABLE)
        {
            MODIFY_REG(ADCx->ASQR,
                       ADC_ASQR_L,
                       (uint32_t)ADC_InitStruct->SequencerLength);

            MODIFY_REG(ADCx->CR1,
                       (ADC_CR1_DISCNUM | ADC_CR1_DISCEN),
                       (uint32_t)ADC_InitStruct->SequencerDiscontMode);
        }

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of ADC_InitStruct_TypeDef structure to default value.
 * @param ADC_InitStruct The pointer to ADC_InitStruct_TypeDef structure.
 */
void ADC_StructInit(ADC_InitStruct_TypeDef *ADC_InitStruct)
{
    /* Set ADC_InitStruct fields to default values */
    ADC_InitStruct->ConversionMode = ADC_CONV_SINGLE;
    ADC_InitStruct->SequencerScanMode = ADC_SEQ_SCAN_DISABLE;
    ADC_InitStruct->SequencerLength = ADC_SEQ_SCAN_LENGTH_1RANK;
    ADC_InitStruct->SequencerDiscontMode = ADC_SEQ_DISCONT_DISABLE;
    ADC_InitStruct->TriggerSource = ADC_TRIG_SOFTWARE;
    ADC_InitStruct->DMATransfer = DISABLE;
}

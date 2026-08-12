/**
 * *****************************************************************************
 *  @file       bmcu_adc.h
 *  @author     Baikal electronics SDK team
 *  @brief      Analog-to-Digital Converter (ADC) module driver header file
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

#ifndef __BMCU_ADC_H
#define __BMCU_ADC_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_TS_GAIN_DEFAULT                 0x3U    /*!< Temperature sensor default gain */
#define ADC_TS_OFFSET_DEFAULT               0x4U    /*!< Temperature sensor default offset */

/** @brief ADC channel */
typedef enum {
    ADC_CH0 = 0,    /*!< Channel 0 */
    ADC_CH1,        /*!< Channel 1 */
    ADC_CH2,        /*!< Channel 2 */
    ADC_CH3,        /*!< Channel 3 */
    ADC_CH4,        /*!< Channel 4 */
    ADC_CH5,        /*!< Channel 5 */
    ADC_CH6,        /*!< Channel 6 */
    ADC_CH7         /*!< Channel 7 */
} ADC_Channels_TypeDef;

/** @brief ADC conversion mode */
typedef enum {
    ADC_CONV_SINGLE     = ADC_CR1_CONT_DISABLE, /*!< Single conversion */
    ADC_CONV_CONTINUOUS = ADC_CR1_CONT_ENABLE   /*!< Continuous conversion */
} ADC_ConvMode_TypeDef;

/** @brief ADC sequencer scan mode */
typedef enum {
    ADC_SEQ_SCAN_DISABLE = ADC_CR1_SCANEN_DISABLE,  /*!< Scan mode is disabled */
    ADC_SEQ_SCAN_ENABLE  = ADC_CR1_SCANEN_ENABLE    /*!< Scan mode is enabled */
} ADC_SeqScanMode_TypeDef;

/** @brief ADC scan sequence rank */
typedef enum {
    ADC_RANK0 = 0,  /*!< Rank 0 */
    ADC_RANK1,      /*!< Rank 1 */
    ADC_RANK2,      /*!< Rank 2 */
    ADC_RANK3,      /*!< Rank 3 */
    ADC_RANK4,      /*!< Rank 4 */
    ADC_RANK5,      /*!< Rank 5 */
    ADC_RANK6,      /*!< Rank 6 */
    ADC_RANK7       /*!< Rank 7 */
} ADC_Rank_TypeDef;

/** @brief ADC sequencer scan length */
typedef enum {
    ADC_SEQ_SCAN_LENGTH_1RANK  = ADC_ASQR_L_1RANK,      /*!< 1 rank */
    ADC_SEQ_SCAN_LENGTH_2RANKS = ADC_ASQR_L_2RANKS,     /*!< 2 ranks */
    ADC_SEQ_SCAN_LENGTH_3RANKS = ADC_ASQR_L_3RANKS,     /*!< 3 ranks */
    ADC_SEQ_SCAN_LENGTH_4RANKS = ADC_ASQR_L_4RANKS,     /*!< 4 ranks */
    ADC_SEQ_SCAN_LENGTH_5RANKS = ADC_ASQR_L_5RANKS,     /*!< 5 ranks */
    ADC_SEQ_SCAN_LENGTH_6RANKS = ADC_ASQR_L_6RANKS,     /*!< 6 ranks */
    ADC_SEQ_SCAN_LENGTH_7RANKS = ADC_ASQR_L_7RANKS,     /*!< 7 ranks */
    ADC_SEQ_SCAN_LENGTH_8RANKS = ADC_ASQR_L_8RANKS      /*!< 8 ranks */
} ADC_SeqScanLength_TypeDef;

/** @brief ADC sequencer discontinuous mode */
typedef enum {
    ADC_SEQ_DISCONT_DISABLE = ADC_CR1_DISCEN_DISABLE,                               /*!< Discontinuous mode is disabled */
    ADC_SEQ_DISCONT_1RANK   = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_1RANK),      /*!< 1 rank */
    ADC_SEQ_DISCONT_2RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_2RANKS),     /*!< 2 ranks */
    ADC_SEQ_DISCONT_3RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_3RANKS),     /*!< 3 ranks */
    ADC_SEQ_DISCONT_4RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_4RANKS),     /*!< 4 ranks */
    ADC_SEQ_DISCONT_5RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_5RANKS),     /*!< 5 ranks */
    ADC_SEQ_DISCONT_6RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_6RANKS),     /*!< 6 ranks */
    ADC_SEQ_DISCONT_7RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_7RANKS),     /*!< 7 ranks */
    ADC_SEQ_DISCONT_8RANKS  = (ADC_CR1_DISCEN_ENABLE | ADC_CR1_DISCNUM_8RANKS)      /*!< 8 ranks */
} ADC_SeqDiscontMode_TypeDef;

/** @brief ADC conversion trigger source */
typedef enum {
    ADC_TRIG_SOFTWARE = ADC_CR1_EXTSEL_0,                       /*!< Software trigger */
    ADC_TRIG_EXT_0    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_0),   /*!< External event 0 trigger */
    ADC_TRIG_EXT_1    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_1),   /*!< External event 1 trigger */
    ADC_TRIG_EXT_2    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_2),   /*!< External event 2 trigger */
    ADC_TRIG_EXT_3    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_3),   /*!< External event 3 trigger */
    ADC_TRIG_EXT_4    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_4),   /*!< External event 4 trigger */
    ADC_TRIG_EXT_5    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_5),   /*!< External event 5 trigger */
    ADC_TRIG_EXT_6    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_6),   /*!< External event 6 trigger */
    ADC_TRIG_EXT_7    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_7),   /*!< External event 7 trigger */
    ADC_TRIG_EXT_8    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_8),   /*!< External event 8 trigger */
    ADC_TRIG_EXT_9    = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_9),   /*!< External event 9 trigger */
    ADC_TRIG_EXT_10   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_10),  /*!< External event 10 trigger */
    ADC_TRIG_EXT_11   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_11),  /*!< External event 11 trigger */
    ADC_TRIG_EXT_12   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_12),  /*!< External event 12 trigger */
    ADC_TRIG_EXT_13   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_13),  /*!< External event 13 trigger */
    ADC_TRIG_EXT_14   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_14),  /*!< External event 14 trigger */
    ADC_TRIG_EXT_15   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_15),  /*!< External event 15 trigger */
    ADC_TRIG_EXT_16   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_16),  /*!< External event 16 trigger */
    ADC_TRIG_EXT_17   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_17),  /*!< External event 17 trigger */
    ADC_TRIG_EXT_18   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_18),  /*!< External event 18 trigger */
    ADC_TRIG_EXT_19   = (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL_19)   /*!< External event 19 trigger */
} ADC_TriggerSource_TypeDef;

/** @brief ADC sampling time */
typedef enum {
#if defined (BMCU_U)
    ADC_SAMPLINGTIME_3CYCLES   = ADC_ASMPR_SMP0_3CYCLES,    /*!< 3 cycles */
    ADC_SAMPLINGTIME_25CYCLES  = ADC_ASMPR_SMP0_25CYCLES,   /*!< 25 cycles */
    ADC_SAMPLINGTIME_50CYCLES  = ADC_ASMPR_SMP0_50CYCLES,   /*!< 50 cycles */
    ADC_SAMPLINGTIME_100CYCLES = ADC_ASMPR_SMP0_100CYCLES,  /*!< 100 cycles */
    ADC_SAMPLINGTIME_150CYCLES = ADC_ASMPR_SMP0_150CYCLES,  /*!< 150 cycles */
    ADC_SAMPLINGTIME_180CYCLES = ADC_ASMPR_SMP0_180CYCLES,  /*!< 180 cycles */
    ADC_SAMPLINGTIME_250CYCLES = ADC_ASMPR_SMP0_250CYCLES,  /*!< 250 cycles */
    ADC_SAMPLINGTIME_300CYCLES = ADC_ASMPR_SMP0_300CYCLES   /*!< 300 cycles */
#endif

#if defined (BE_U1000)
    ADC_SAMPLINGTIME_0CYCLES   = ADC_ASMPR_SMP0_0CYCLES,    /*!< 0 cycles */
    ADC_SAMPLINGTIME_25CYCLES  = ADC_ASMPR_SMP0_25CYCLES,   /*!< 25 cycles */
    ADC_SAMPLINGTIME_50CYCLES  = ADC_ASMPR_SMP0_50CYCLES,   /*!< 50 cycles */
    ADC_SAMPLINGTIME_75CYCLES  = ADC_ASMPR_SMP0_75CYCLES,   /*!< 75 cycles */
    ADC_SAMPLINGTIME_125CYCLES = ADC_ASMPR_SMP0_125CYCLES,  /*!< 125 cycles */
    ADC_SAMPLINGTIME_175CYCLES = ADC_ASMPR_SMP0_175CYCLES,  /*!< 175 cycles */
    ADC_SAMPLINGTIME_225CYCLES = ADC_ASMPR_SMP0_225CYCLES,  /*!< 225 cycles */
    ADC_SAMPLINGTIME_275CYCLES = ADC_ASMPR_SMP0_275CYCLES   /*!< 275 cycles */
#endif
} ADC_SamplingTime_TypeDef;

/** @brief ADC clock division */
typedef enum {
#if defined (BMCU_U)
    ADC_CLOCKDIVISION_DIV2 = ADC_CR1_CKD_DIV2,      /*!< Clock divided by 2 */
    ADC_CLOCKDIVISION_DIV4 = ADC_CR1_CKD_DIV4,      /*!< Clock divided by 4 */
    ADC_CLOCKDIVISION_DIV8 = ADC_CR1_CKD_DIV8       /*!< Clock divided by 8 */
#endif

#if defined (BE_U1000)
    ADC_CLOCKDIVISION_DIV1  = ADC_CR1_CKD_DIV1,     /*!< Clock is not divided */
    ADC_CLOCKDIVISION_DIV2  = ADC_CR1_CKD_DIV2,     /*!< Clock is divided by 2 */
    ADC_CLOCKDIVISION_DIV3  = ADC_CR1_CKD_DIV3,     /*!< Clock is divided by 3 */
    ADC_CLOCKDIVISION_DIV4  = ADC_CR1_CKD_DIV4,     /*!< Clock is divided by 4 */
    ADC_CLOCKDIVISION_DIV5  = ADC_CR1_CKD_DIV5,     /*!< Clock is divided by 5 */
    ADC_CLOCKDIVISION_DIV6  = ADC_CR1_CKD_DIV6,     /*!< Clock is divided by 6 */
    ADC_CLOCKDIVISION_DIV7  = ADC_CR1_CKD_DIV7,     /*!< Clock is divided by 7 */
    ADC_CLOCKDIVISION_DIV8  = ADC_CR1_CKD_DIV8,     /*!< Clock is divided by 8 */
    ADC_CLOCKDIVISION_DIV9  = ADC_CR1_CKD_DIV9,     /*!< Clock is divided by 9 */
    ADC_CLOCKDIVISION_DIV10 = ADC_CR1_CKD_DIV10,    /*!< Clock is divided by 10 */
    ADC_CLOCKDIVISION_DIV11 = ADC_CR1_CKD_DIV11,    /*!< Clock is divided by 11 */
    ADC_CLOCKDIVISION_DIV12 = ADC_CR1_CKD_DIV12,    /*!< Clock is divided by 12 */
    ADC_CLOCKDIVISION_DIV13 = ADC_CR1_CKD_DIV13,    /*!< Clock is divided by 13 */
    ADC_CLOCKDIVISION_DIV14 = ADC_CR1_CKD_DIV14,    /*!< Clock is divided by 14 */
    ADC_CLOCKDIVISION_DIV15 = ADC_CR1_CKD_DIV15,    /*!< Clock is divided by 15 */
    ADC_CLOCKDIVISION_DIV16 = ADC_CR1_CKD_DIV16     /*!< Clock is divided by 16 */
#endif
} ADC_ClockDivision_TypeDef;

/** @brief ADC input mode */
typedef enum {
    ADC_INPUT_SINGLE_END   = ADC_ASER_SELVI_HD_LS_SINGLE,   /*!< Single end */
    ADC_INPUT_DIFFERENTIAL = ADC_ASER_SELVI_HD_LS_DIFF      /*!< Differential */
} ADC_InputMode_TypeDef;

/** @brief ADC output mode */
typedef enum {
    ADC_OUTPUT_DATA_UNSIGNED = ADC_ASER_SELDO_HS_LU_UNSIGNED,   /*!< Unsigned data */
    ADC_OUTPUT_DATA_SIGNED   = ADC_ASER_SELDO_HS_LU_SIGNED      /*!< Signed data */
} ADC_OutputMode_TypeDef;

/** @brief ADC temperature sensor filter mode */
typedef enum {
    ADC_TS_MODE_BYPASS = ADC_ASER_TS_MODE_BYPASS,   /*!< Bypass internal lowpass filter */
    ADC_TS_MODE_AVG64  = ADC_ASER_TS_MODE_AVG64,    /*!< 64 times average */
    ADC_TS_MODE_AVG128 = ADC_ASER_TS_MODE_AVG128,   /*!< 128 times average */
    ADC_TS_MODE_AVG256 = ADC_ASER_TS_MODE_AVG256    /*!< 256 times average */
} ADC_TS_FilterMode_TypeDef;

/** @brief Flags */
typedef enum {
    ADC_FLAG_EOC = 0,           /*!< End of conversion */
    ADC_FLAG_STRT  = 1          /*!< Start of conversion */
} ADC_Flag_TypeDef;

/** @brief ADC Init Structure definition */
typedef struct
{
    ADC_ConvMode_TypeDef ConversionMode;                /*!< Conversion mode */
    ADC_SeqScanMode_TypeDef SequencerScanMode;          /*!< Scan mode */
    ADC_SeqScanLength_TypeDef SequencerLength;          /*!< Scan sequence length */
    ADC_SeqDiscontMode_TypeDef SequencerDiscontMode;    /*!< Discontinuous mode */
    ADC_TriggerSource_TypeDef TriggerSource;            /*!< Trigger source */
    FunctionalState DMATransfer;                        /*!< DMA transfer control */
} ADC_InitStruct_TypeDef;

/* Constants for the measured temperature calculation */
#define ADC_TEMPSENS_DINITIAL   (542.7)
#define ADC_TEMPSENS_CE         (0.4854)
#define ADC_TEMPSENS_DREF       (1496U)

/**
 * @brief Writes a value in ADC register.
 * @param INSTANCE The ADC instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define ADC_WriteReg(INSTANCE, REG, VALUE)      WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in ADC register.
 * @param INSTANCE The ADC instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define ADC_ReadReg(INSTANCE, REG)              READ_REG(INSTANCE->REG)

/**
 * @brief Enables the selected ADC instance.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_Enable(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_ADON);
}

/**
 * @brief Disables the selected ADC instance.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_Disable(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->CR1, ADC_CR1_ADON);
}

/**
 * @brief Checks if the selected ADC instance is enabled.
 * @param ADCx The ADC instance.
 * @retval 1 if ADC is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t ADC_IsEnabled(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->CR1, ADC_CR1_ADON) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets ADC conversion mode.
 * @param ADCx The ADC instance.
 * @param ConvMode The conversion mode. Can be one of ADC_ConvMode_TypeDef values.
 */
__STATIC_INLINE void ADC_SetConversionMode(ADC_TypeDef *ADCx, ADC_ConvMode_TypeDef ConvMode)
{
    MODIFY_REG(ADCx->CR1, ADC_CR1_CONT, (uint32_t)ConvMode);
}

/**
 * @brief Returns ADC conversion mode.
 * @param ADCx The ADC instance.
 * @returns The conversion mode. Can be one of ADC_ConvMode_TypeDef values.
 */
__STATIC_INLINE ADC_ConvMode_TypeDef ADC_GetConversionMode(ADC_TypeDef *ADCx)
{
    return (ADC_ConvMode_TypeDef)READ_BIT(ADCx->CR1, ADC_CR1_CONT);
}

/**
 * @brief Sets ADC sequencer scan mode.
 * @param ADCx The ADC instance.
 * @param ScanMode The scan mode. Can be one of ADC_SeqScanMode_TypeDef values.
 */
__STATIC_INLINE void ADC_SetSequencerScanMode(ADC_TypeDef *ADCx, ADC_SeqScanMode_TypeDef ScanMode)
{
    MODIFY_REG(ADCx->CR1, ADC_CR1_SCANEN, (uint32_t)ScanMode);
}

/**
 * @brief Returns ADC sequencer scan mode.
 * @param ADCx The ADC instance.
 * @returns The scan mode. Can be one of ADC_SeqScanMode_TypeDef values.
 */
__STATIC_INLINE ADC_SeqScanMode_TypeDef ADC_GetSequencerScanMode(ADC_TypeDef *ADCx)
{
    return (ADC_SeqScanMode_TypeDef)READ_BIT(ADCx->CR1, ADC_CR1_SCANEN);
}

/**
 * @brief Sets ADC sequencer length.
 * @param ADCx The ADC instance.
 * @param SequencerNbRanks The sequencer length.
 */
__STATIC_INLINE void ADC_SetSequencerLength(ADC_TypeDef *ADCx, ADC_SeqScanLength_TypeDef SequencerNbRanks)
{
    MODIFY_REG(ADCx->ASQR, ADC_ASQR_L, (uint32_t)SequencerNbRanks);
}

/**
 * @brief Returns ADC sequencer length.
 * @param ADCx The ADC instance.
 * @returns The sequencer length.
 */
__STATIC_INLINE ADC_SeqScanLength_TypeDef ADC_GetSequencerLength(ADC_TypeDef *ADCx)
{
    return (ADC_SeqScanLength_TypeDef)READ_BIT(ADCx->ASQR, ADC_ASQR_L);
}

/**
 * @brief Sets ADC channel on the selected scan sequence rank.
 * @param ADCx The ADC instance.
 * @param Rank The scan sequence rank. Can be one of ADC_Rank_TypeDef values.
 * @param Channel The ADC channel. Can be one of ADC_Channels_TypeDef values.
 */
__STATIC_INLINE void ADC_SetSequencerRanks(ADC_TypeDef *ADCx, ADC_Rank_TypeDef Rank, ADC_Channels_TypeDef Channel)
{
    uint32_t Pos = (uint32_t)Rank * 3U;
    
    MODIFY_REG(ADCx->ASQR,
               ADC_ASQR_SQ0 << Pos,
               ((uint32_t)Channel & ADC_ASQR_SQ0) << Pos);
}

/**
 * @brief Returns ADC channel on the selected scan sequence rank.
 * @param ADCx The ADC instance.
 * @param Rank The scan sequence rank. Can be one of ADC_Rank_TypeDef values.
 * @returns The ADC channel. Can be one of ADC_Channels_TypeDef values.
 */
__STATIC_INLINE ADC_Channels_TypeDef ADC_GetSequencerRanks(ADC_TypeDef *ADCx, ADC_Rank_TypeDef Rank)
{
    uint32_t Pos = (uint32_t)Rank * 3U;

    return (ADC_Channels_TypeDef)(READ_BIT(ADCx->ASQR, ADC_ASQR_SQ0 << Pos) >> Pos);
}

/**
 * @brief Sets ADC sequencer discontinuous mode.
 * @note The sequence is subdivided and scan conversions interrupted every
 *       selected number of ranks.
 * @param ADCx The ADC instance.
 * @param SeqDiscont The discontinuous mode. Can be one of ADC_SeqDiscontMode_TypeDef values.
 */
__STATIC_INLINE void ADC_SetSequencerDiscontMode(ADC_TypeDef *ADCx, ADC_SeqDiscontMode_TypeDef SeqDiscont)
{
    MODIFY_REG(ADCx->CR1, ADC_CR1_DISCEN | ADC_CR1_DISCNUM, (uint32_t)SeqDiscont);
}

/**
 * @brief Returns ADC sequencer discontinuous mode.
 * @note The sequence is subdivided and scan conversions interrupted every
 *       selected number of ranks.
 * @param ADCx The ADC instance.
 * @returns The discontinuous mode. Can be one of ADC_SeqDiscontMode_TypeDef values.
 */
__STATIC_INLINE ADC_SeqDiscontMode_TypeDef ADC_GetSequencerDiscontMode(ADC_TypeDef *ADCx)
{
    __IO uint32_t tmpreg = READ_BIT(ADCx->CR1, ADC_CR1_DISCEN | ADC_CR1_DISCNUM);

    return (ADC_SeqDiscontMode_TypeDef)(((tmpreg & ADC_CR1_DISCEN) != 0UL) ? tmpreg : ADC_CR1_DISCEN_DISABLE);
}

/**
 * @brief Sets ADC conversion trigger source.
 * @param ADCx The ADC instance.
 * @param TriggerSource The trigger source. Can be one of ADC_TriggerSource_TypeDef values.
 */
__STATIC_INLINE void ADC_SetTriggerSource(ADC_TypeDef *ADCx, ADC_TriggerSource_TypeDef TriggerSource)
{
    MODIFY_REG(ADCx->CR1,
               (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL),
               (uint32_t)TriggerSource);
}

/**
 * @brief Returns ADC conversion trigger source.
 * @param ADCx The ADC instance.
 * @returns The trigger source. Can be one of ADC_TriggerSource_TypeDef values.
 */
__STATIC_INLINE ADC_TriggerSource_TypeDef ADC_GetTriggerSource(ADC_TypeDef *ADCx)
{
    return (ADC_TriggerSource_TypeDef)READ_BIT(ADCx->CR1, (ADC_CR1_EXTTRIG | ADC_CR1_EXTSEL));
}

/**
 * @brief Checks if ADC trigger source is internal (SW start) or external.
 * @param ADCx The ADC instance.
 * @retval 0 if trigger source is external trigger,
 *         1 if trigger source is SW start.
 */
__STATIC_INLINE uint32_t ADC_IsTriggerSourceSWStart(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->CR1, ADC_CR1_EXTTRIG) != 0UL) ? 0UL : 1UL);
}

/**
 * @brief Sets sampling time of the selected ADC channel.
 * @param ADCx The ADC instance.
 * @param Channel The ADC channel. Can be one of ADC_Channels_TypeDef values.
 * @param SamplingTime The sampling time. Can be one of ADC_SamplingTime_TypeDef values.
 */
__STATIC_INLINE void ADC_SetChannelSamplingTime(ADC_TypeDef *ADCx, ADC_Channels_TypeDef Channel, ADC_SamplingTime_TypeDef SamplingTime)
{
    uint32_t Pos = (uint32_t)Channel * 3U;
    
    MODIFY_REG(ADCx->ASMPR,
               ADC_ASMPR_SMP0 << Pos,
               ((uint32_t)SamplingTime & ADC_ASMPR_SMP0) << Pos);
}

/**
 * @brief Returns sampling time of the selected ADC channel.
 * @param ADCx The ADC instance.
 * @param Channel The ADC channel. Can be one of ADC_Channels_TypeDef values.
 * @returns The sampling time. Can be one of ADC_SamplingTime_TypeDef values.
 */
__STATIC_INLINE ADC_SamplingTime_TypeDef ADC_GetChannelSamplingTime(ADC_TypeDef *ADCx, ADC_Channels_TypeDef Channel)
{
    uint32_t Pos = (uint32_t)Channel * 3U;

    return (ADC_SamplingTime_TypeDef)(READ_BIT(ADCx->ASMPR, ADC_ASMPR_SMP0 << Pos) >> Pos);
}

/**
 * @brief Sets the ADC clock division ratio.
 * @param ADCx The ADC instance.
 * @param ClockDivision The division ratio. Can be one of ADC_ClockDivision_TypeDef values.
 */
__STATIC_INLINE void ADC_SetClockDivision(ADC_TypeDef *ADCx, ADC_ClockDivision_TypeDef ClockDivision)
{
    MODIFY_REG(ADCx->CR1, ADC_CR1_CKD, (uint32_t)ClockDivision);
}

/**
 * @brief Returns the ADC clock division ratio.
 * @param ADCx The ADC instance.
 * @returns The division ratio. Can be one of ADC_ClockDivision_TypeDef values.
 */
__STATIC_INLINE ADC_ClockDivision_TypeDef ADC_GetClockDivision(ADC_TypeDef *ADCx)
{
    return (ADC_ClockDivision_TypeDef)READ_BIT(ADCx->CR1, ADC_CR1_CKD);
}

/**
 * @brief Enables ADC DMA transfer.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_DMA_Enable(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_DMAEN);
}

/**
 * @brief Disables ADC DMA transfer.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_DMA_Disable(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->CR1, ADC_CR1_DMAEN);
}

/**
 * @brief Checks if ADC DMA transfer is enabled.
 * @param ADCx The ADC instance.
 * @retval 1 if ADC DMA transfer is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t ADC_DMA_IsEnabled(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->CR1, ADC_CR1_DMAEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets ADC input mode.
 * @param ADCx The ADC instance.
 * @param InputMode The input mode. Can be one of ADC_InputMode_TypeDef values.
 */
__STATIC_INLINE void ADC_SetInputMode(ADC_TypeDef *ADCx, ADC_InputMode_TypeDef InputMode)
{
    MODIFY_REG(ADCx->ASER, ADC_ASER_SELVI_HD_LS, ((uint32_t)InputMode & ADC_ASER_SELVI_HD_LS));
}

/**
 * @brief Returns ADC input mode.
 * @param ADCx The ADC instance.
 * @returns The input mode. Can be one of ADC_InputMode_TypeDef values.
 */
__STATIC_INLINE ADC_InputMode_TypeDef ADC_GetInputMode(ADC_TypeDef *ADCx)
{
    return (ADC_InputMode_TypeDef)READ_BIT(ADCx->ASER, ADC_ASER_SELVI_HD_LS);
}

/**
 * @brief Sets ADC output mode.
 * @param ADCx The ADC instance.
 * @param OutputMode The output mode. Can be one of ADC_OutputMode_TypeDef values.
 */
__STATIC_INLINE void ADC_SetOutputMode(ADC_TypeDef *ADCx, ADC_OutputMode_TypeDef OutputMode)
{
    MODIFY_REG(ADCx->ASER, ADC_ASER_SELDO_HS_LU, ((uint32_t)OutputMode & ADC_ASER_SELDO_HS_LU));
}

/**
 * @brief Returns ADC output mode.
 * @param ADCx The ADC instance.
 * @returns The output mode. Can be one of ADC_OutputMode_TypeDef values.
 */
__STATIC_INLINE ADC_OutputMode_TypeDef ADC_GetOutputMode(ADC_TypeDef *ADCx)
{
    return (ADC_OutputMode_TypeDef)READ_BIT(ADCx->ASER, ADC_ASER_SELDO_HS_LU);
}

/**
 * @brief Enable temperature measurement mode.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_TS_Enable(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_TEMPSENS);
}

/**
 * @brief Disable temperature measurement mode.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_TS_Disable(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->CR1, ADC_CR1_TEMPSENS);
}

/**
 * @brief Sets the temperature sensor gain.
 *        The default value is 0x8.
 * @param ADCx The ADC instance.
 * @param Gain The gain value.
 */
__STATIC_INLINE void ADC_TS_SetGain(ADC_TypeDef *ADCx, uint32_t Gain)
{
    MODIFY_REG(ADCx->ASER, ADC_ASER_ADJ_TD_GA, ((Gain << ADC_ASER_ADJ_TD_GA_Pos) & ADC_ASER_ADJ_TD_GA));
}

/**
 * @brief Returns the temperature sensor gain.
 * @param ADCx The ADC instance.
 * @returns The gain value.
 */
__STATIC_INLINE uint32_t ADC_TS_GetGain(ADC_TypeDef *ADCx)
{
    return (uint32_t)(READ_BIT(ADCx->ASER, ADC_ASER_ADJ_TD_GA) >> ADC_ASER_ADJ_TD_GA_Pos);
}

/**
 * @brief Sets the temperature sensor offset.
 *        The default value is 0x8.
 * @param ADCx The ADC instance.
 * @param Offset The offset value.
 */
__STATIC_INLINE void ADC_TS_SetOffset(ADC_TypeDef *ADCx, uint32_t Offset)
{
    MODIFY_REG(ADCx->ASER, ADC_ASER_ADJ_TD_OS, ((Offset << ADC_ASER_ADJ_TD_OS_Pos) & ADC_ASER_ADJ_TD_OS));
}

/**
 * @brief Returns the temperature sensor offset.
 * @param ADCx The ADC instance.
 * @returns The offset value.
 */
__STATIC_INLINE uint32_t ADC_TS_GetOffset(ADC_TypeDef *ADCx)
{
    return (uint32_t)(READ_BIT(ADCx->ASER, ADC_ASER_ADJ_TD_OS) >> ADC_ASER_ADJ_TD_OS_Pos);
}

/**
 * @brief Sets the temperature sensor filter mode.
 * @param ADCx The ADC instance.
 * @param FilterMode The filter mode. Can be one of ADC_TS_FilterMode_TypeDef values.
 */
__STATIC_INLINE void ADC_TS_SetFilterMode(ADC_TypeDef *ADCx, ADC_TS_FilterMode_TypeDef FilterMode)
{
    MODIFY_REG(ADCx->ASER, ADC_ASER_TS_MODE, ((uint32_t)FilterMode & ADC_ASER_TS_MODE));
}

/**
 * @brief Returns the temperature sensor filter mode.
 * @param ADCx The ADC instance.
 * @returns The filter mode. Can be one of ADC_TS_FilterMode_TypeDef values.
 */
__STATIC_INLINE ADC_TS_FilterMode_TypeDef ADC_TS_GetFilterMode(ADC_TypeDef *ADCx)
{
    return (ADC_TS_FilterMode_TypeDef)READ_BIT(ADCx->ASER, ADC_ASER_TS_MODE);
}

/**
 * @brief Starts ADC calibration.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_StartCalibration(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->ASER, ADC_ASER_CAL_SEL);
}

/**
 * @brief Starts ADC calibration.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_StopCalibration(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->ASER, ADC_ASER_CAL_SEL);
}

/**
 * @brief Returns ADC calibration state.
 * @param ADCx The ADC instance.
 * @returns 1 if calibration is in progress, otherwise 0.
 */
__STATIC_INLINE uint32_t ADC_IsCalibrationOnGoing(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->ASER, ADC_ASER_CAL_SEL) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Starts ADC conversion.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_StartConversionSWStart(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_SWSTART);
}

/**
 * @brief Starts ADC conversion from external trigger.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_StartConversionExtTrig(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_EXTTRIG);
}

/**
 * @brief Stops ADC conversion from external trigger.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_StopConversionExtTrig(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->CR1, ADC_CR1_EXTTRIG);
}

/**
 * @brief Returns ADC 12-bit conversion data.
 * @param ADCx The ADC instance.
 * @returns The ADC convertion data in the range of 0x000 to 0xFFF.
 */
__STATIC_INLINE uint16_t ADC_ReadConversionData12(ADC_TypeDef *ADCx)
{
    return (uint16_t)(READ_BIT(ADCx->ADR, ADC_ADR));
}

/**
 * @brief Checks if a flag is set.
 * @param ADCx The ADC instance.
 * @param Flag The flag. Can be one of ADC_Flag_TypeDef values.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t ADC_IsActiveFlag(ADC_TypeDef *ADCx, ADC_Flag_TypeDef Flag)
{
    return ((READ_BIT(ADCx->ASTR, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if the EOC flag is set.
 * @param ADCx The ADC instance.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t ADC_IsActiveFlag_EOC(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->ASTR, ADC_ASTR_EOC) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if the STRT flag is set.
 * @param ADCx The ADC instance.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t ADC_IsActiveFlag_STRT(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->ASTR, ADC_ASTR_STRT) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears a flag.
 * @note Writing to ASTR register clears all flags.
 * @param ADCx The ADC instance.
 * @param Flag The flag. Can be one of ADC_Flag_TypeDef values.
 */
__STATIC_INLINE void ADC_ClearFlag(ADC_TypeDef *ADCx, ADC_Flag_TypeDef Flag)
{
    SET_BIT(ADCx->ASTR, (1UL << Flag));
}

/**
 * @brief Clears the EOC flag.
 * @note Writing to ASTR register clears all flags.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_ClearFlag_EOC(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->ASTR, ADC_ASTR_EOC);
}

/**
 * @brief Clears the STRT flag.
 * @note Writing to ASTR register clears all flags.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_ClearFlag_STRT(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->ASTR, ADC_ASTR_STRT);
}

/**
 * @brief Clears all flags.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_ClearFlag_All(ADC_TypeDef *ADCx)
{
    WRITE_REG(ADCx->ASTR, 0xFFFFFFFFUL);
}

/**
 * @brief Enables End Of Conversion interrupt.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_EnableIT_EOC(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_IEEOC);
}

/**
 * @brief Disables End Of Conversion interrupt.
 * @param ADCx The ADC instance.
 */
__STATIC_INLINE void ADC_DisableIT_EOC(ADC_TypeDef *ADCx)
{
    CLEAR_BIT(ADCx->CR1, ADC_CR1_IEEOC);
}

/**
 * @brief Checks if End Of Conversion interrupt is enabled.
 * @param ADCx The ADC instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t ADC_IsEnabledIT_EOC(ADC_TypeDef *ADCx)
{
    return ((READ_BIT(ADCx->CR1, ADC_CR1_IEEOC) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears End Of Conversion interrupt.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void ADC_ClearIT_EOC(ADC_TypeDef *ADCx)
{
    SET_BIT(ADCx->CR1, ADC_CR1_CLRINTRPT);
}

ErrorStatus ADC_DeInit(ADC_TypeDef *ADCx);
ErrorStatus ADC_Init(ADC_TypeDef *ADCx, ADC_InitStruct_TypeDef *ADC_InitStruct);
void ADC_StructInit(ADC_InitStruct_TypeDef *ADC_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_ADC_H */

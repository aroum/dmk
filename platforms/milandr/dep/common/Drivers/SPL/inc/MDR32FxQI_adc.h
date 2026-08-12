/**
  ******************************************************************************
  * @file    MDR32FxQI_adc.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the functions prototypes for the ADC
  *          firmware library.
  ******************************************************************************
  * <br><br>
  *
  * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
  * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDR32FxQI_ADC_H
#define __MDR32FxQI_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup ADC ADC
  * @{
  */

/** @defgroup ADC_Exported_Constants ADC Exported Constants
  * @{
  */

/** @defgroup ADC_Start_Delay ADC Start Delay
  * @{
  */

#define IS_ADC_START_DELAY_VALUE(VALUE) (((VALUE) & ~0xF) == 0)

/** @} */ /* End of group ADC_Start_Delay */

/** @defgroup ADC_Int_VRef_Trimming ADC Internal Voltage Reference Trimming
  * @{
  */
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    #define IS_ADC_VREF_TRIMMING_VALUE(VALUE) (((VALUE) & ~0x0F) == 0)
#elif defined (USE_K1986VE1xI)
    #define IS_ADC_VREF_TRIMMING_VALUE(VALUE) (((VALUE) & ~0x1F) == 0)
#endif

/** @} */ /* End of group ADC_Int_VRef_Trimming */

/** @defgroup ADCx_Channels ADC Channels
  * @{
  */
#define ADC_CH_ADC0_MSK         (((uint32_t)0x1) << ADC_CH_ADC0       ) /*!< Selects the ADC channel 0.  */
#define ADC_CH_ADC1_MSK         (((uint32_t)0x1) << ADC_CH_ADC1       ) /*!< Selects the ADC channel 1.  */
#define ADC_CH_ADC2_MSK         (((uint32_t)0x1) << ADC_CH_ADC2       ) /*!< Selects the ADC channel 2.  */
#define ADC_CH_ADC3_MSK         (((uint32_t)0x1) << ADC_CH_ADC3       ) /*!< Selects the ADC channel 3.  */
#define ADC_CH_ADC4_MSK         (((uint32_t)0x1) << ADC_CH_ADC4       ) /*!< Selects the ADC channel 4.  */
#define ADC_CH_ADC5_MSK         (((uint32_t)0x1) << ADC_CH_ADC5       ) /*!< Selects the ADC channel 5.  */
#define ADC_CH_ADC6_MSK         (((uint32_t)0x1) << ADC_CH_ADC6       ) /*!< Selects the ADC channel 6.  */
#define ADC_CH_ADC7_MSK         (((uint32_t)0x1) << ADC_CH_ADC7       ) /*!< Selects the ADC channel 7.  */
#define ADC_CH_ADC8_MSK         (((uint32_t)0x1) << ADC_CH_ADC8       ) /*!< Selects the ADC channel 8.  */
#define ADC_CH_ADC9_MSK         (((uint32_t)0x1) << ADC_CH_ADC9       ) /*!< Selects the ADC channel 9.  */
#define ADC_CH_ADC10_MSK        (((uint32_t)0x1) << ADC_CH_ADC10      ) /*!< Selects the ADC channel 10. */
#define ADC_CH_ADC11_MSK        (((uint32_t)0x1) << ADC_CH_ADC11      ) /*!< Selects the ADC channel 11. */
#define ADC_CH_ADC12_MSK        (((uint32_t)0x1) << ADC_CH_ADC12      ) /*!< Selects the ADC channel 12. */
#define ADC_CH_ADC13_MSK        (((uint32_t)0x1) << ADC_CH_ADC13      ) /*!< Selects the ADC channel 13. */
#define ADC_CH_ADC14_MSK        (((uint32_t)0x1) << ADC_CH_ADC14      ) /*!< Selects the ADC channel 14. */
#define ADC_CH_ADC15_MSK        (((uint32_t)0x1) << ADC_CH_ADC15      ) /*!< Selects the ADC channel 15. */
#define ADC_CH_INT_VREF_MSK     (((uint32_t)0x1) << ADC_CH_INT_VREF   ) /*!< Selects the ADC channel 30 (Internal VRef). */
#define ADC_CH_TEMP_SENSOR_MSK  (((uint32_t)0x1) << ADC_CH_TEMP_SENSOR) /*!< Selects the ADC channel 31 (Temperature Sensor). */

#define ADC2_ALL_CH_MSK         (ADC_CH_ADC0_MSK  | \
                                 ADC_CH_ADC1_MSK  | \
                                 ADC_CH_ADC2_MSK  | \
                                 ADC_CH_ADC3_MSK  | \
                                 ADC_CH_ADC4_MSK  | \
                                 ADC_CH_ADC5_MSK  | \
                                 ADC_CH_ADC6_MSK  | \
                                 ADC_CH_ADC7_MSK  | \
                                 ADC_CH_ADC8_MSK  | \
                                 ADC_CH_ADC9_MSK  | \
                                 ADC_CH_ADC10_MSK | \
                                 ADC_CH_ADC11_MSK | \
                                 ADC_CH_ADC12_MSK | \
                                 ADC_CH_ADC13_MSK | \
                                 ADC_CH_ADC14_MSK | \
                                 ADC_CH_ADC15_MSK)

#define IS_ADC2_CH_MASK(MASK)   (((MASK) & ~ADC2_ALL_CH_MSK) == 0)

#define ADC1_ALL_CH_MSK         (ADC2_ALL_CH_MSK     | \
                                 ADC_CH_INT_VREF_MSK | \
                                 ADC_CH_TEMP_SENSOR_MSK)

#define IS_ADC1_CH_MASK(MASK)   ((((MASK) & ~ADC2_ALL_CH_MSK) == 0) || \
                                 ((MASK) == ADC_CH_INT_VREF_MSK)    || \
                                 ((MASK) == ADC_CH_TEMP_SENSOR_MSK))

/** @} */ /* End of group ADCx_Channels */

/** @defgroup ADCx_Delay_Go ADCx start conversion delay
  * @{
  */

#define IS_ADC_DELAY_GO_VALUE(VALUE) (((VALUE) & ~0x7) == 0)

/** @} */ /* End of group ADCx_Delay_Go */

/** @} */ /* End of group ADC_Exported_Constants */


/** @defgroup ADC_Exported_Types ADC Exported Types
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief ADC Synchronous Mode
  */
typedef enum
{
    ADC_SyncMode_Independent = (((uint32_t)0x0) << ADC1_CFG_SYNC_CONVER_Pos), /*!< The independent ADC1, ADC2 operation mode. */
    ADC_SyncMode_Synchronous = (((uint32_t)0x1) << ADC1_CFG_SYNC_CONVER_Pos)  /*!< The synchronous ADC1, ADC2 operation mode. */
} ADC_SyncMode;

#define IS_ADC_SYNC_MODE(MODE) (((MODE) == ADC_SyncMode_Independent) || \
                                ((MODE) == ADC_SyncMode_Synchronous))

#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief ADC Temperature Sensor configuration
  */
typedef enum
{
    ADC_TEMP_SENSOR_Disable = (((uint32_t)0x0) << ADC1_CFG_TS_EN_Pos), /*!< Disables Temperature Sensor. */
    ADC_TEMP_SENSOR_Enable  = (((uint32_t)0x1) << ADC1_CFG_TS_EN_Pos)  /*!< Enables Temperature Sensor. */
} ADC_Temp_Sensor;

#define IS_ADC_TEMP_SENSOR_CONFIG(CONFIG) (((CONFIG) == ADC_TEMP_SENSOR_Disable) || \
                                           ((CONFIG) == ADC_TEMP_SENSOR_Enable ))

/**
  * @brief ADC Temperature Sensor Amplifier configuration
  */
typedef enum
{
    ADC_TEMP_SENSOR_AMPLIFIER_Disable = (((uint32_t)0x0) << ADC1_CFG_TS_BUF_EN_Pos), /*!< Disables Temperature Sensor Amplifier. */
    ADC_TEMP_SENSOR_AMPLIFIER_Enable  = (((uint32_t)0x1) << ADC1_CFG_TS_BUF_EN_Pos)  /*!< Enables Temperature Sensor Amplifier. */
} ADC_Temp_Sensor_Amplifier;

#define IS_ADC_TEMP_SENSOR_AMP_CONFIG(CONFIG) (((CONFIG) == ADC_TEMP_SENSOR_AMPLIFIER_Disable) || \
                                               ((CONFIG) == ADC_TEMP_SENSOR_AMPLIFIER_Enable ))

/**
  * @brief ADC Temperature Sensor Conversion configuration
  */
typedef enum
{
    ADC_TEMP_SENSOR_CONVERSION_Disable = (((uint32_t)0x0) << ADC1_CFG_SEL_TS_Pos), /*!< Disables Temperature Sensor Conversion. */
    ADC_TEMP_SENSOR_CONVERSION_Enable  = (((uint32_t)0x1) << ADC1_CFG_SEL_TS_Pos)  /*!< Enables Temperature Sensor Conversion. */
} ADC_Temp_Sensor_Conversion;

#define IS_ADC_TEMP_SENSOR_CONVERSION_CONFIG(CONFIG) (((CONFIG) == ADC_TEMP_SENSOR_CONVERSION_Disable) || \
                                                      ((CONFIG) == ADC_TEMP_SENSOR_CONVERSION_Enable ))

/**
  * @brief ADC Internal Voltage Reference Conversion configuration
  */
typedef enum
{
    ADC_VREF_CONVERSION_Disable = (((uint32_t)0x0) << ADC1_CFG_SEL_VREF_Pos), /*!< Disables Internal Voltage Reference Conversion. */
    ADC_VREF_CONVERSION_Enable  = (((uint32_t)0x1) << ADC1_CFG_SEL_VREF_Pos)  /*!< Enables Internal Voltage Reference Conversion. */
} ADC_Int_VRef_Conversion;

#define IS_ADC_VREF_CONVERSION_CONFIG(CONFIG) (((CONFIG) == ADC_VREF_CONVERSION_Disable) || \
                                               ((CONFIG) == ADC_VREF_CONVERSION_Enable ))

/**
  * @brief ADCx Clock Source configuration
  */
typedef enum
{
    ADC_CLOCK_SOURCE_CPU = (((uint32_t)0x0) << ADC1_CFG_REG_CLKS_Pos), /*!< Selects CPU_CLK as ADC clock. */
    ADC_CLOCK_SOURCE_ADC = (((uint32_t)0x1) << ADC1_CFG_REG_CLKS_Pos)  /*!< Selects ADC_CLK as ADC clock. */
} ADCx_Clock_Source;

#define IS_ADC_CLOCK_SOURCE_CONFIG(CONFIG) (((CONFIG) == ADC_CLOCK_SOURCE_ADC) || \
                                            ((CONFIG) == ADC_CLOCK_SOURCE_CPU))

/**
  * @brief ADC Sampling Mode configuration
  */
typedef enum
{
    ADC_SAMPLING_MODE_SINGLE_CONV = (((uint32_t)0x0) << ADC1_CFG_REG_SAMPLE_Pos), /*!< Selects ADC single mode operation. */
    ADC_SAMPLING_MODE_CYCLIC_CONV = (((uint32_t)0x1) << ADC1_CFG_REG_SAMPLE_Pos)  /*!< Selects ADC cyclic mode operation. */
} ADCx_Sampling_Mode;

#define IS_ADC_SAMPLING_MODE_CONFIG(CONFIG) (((CONFIG) == ADC_SAMPLING_MODE_SINGLE_CONV) || \
                                             ((CONFIG) == ADC_SAMPLING_MODE_CYCLIC_CONV))

/**
  * @brief ADC Channel Swithing configuration
  */
typedef enum
{
    ADC_CH_SWITCHING_Disable = (((uint32_t)0x0) << ADC1_CFG_REG_CHCH_Pos), /*!< Disables Channel Swithing. */
    ADC_CH_SWITCHING_Enable  = (((uint32_t)0x1) << ADC1_CFG_REG_CHCH_Pos)  /*!< Enables Channel Swithing. */
} ADCx_Channel_Switching;

#define IS_ADC_CH_SWITCHING_CONFIG(CONFIG) (((CONFIG) == ADC_CH_SWITCHING_Disable) || \
                                            ((CONFIG) == ADC_CH_SWITCHING_Enable ))

/**
  * @brief ADC Channel Number
  */
typedef enum
{
    ADC_CH_ADC0        = ((uint32_t)0x00), /*!< Specifies the ADC channel 0.  */
    ADC_CH_ADC1        = ((uint32_t)0x01), /*!< Specifies the ADC channel 1.  */
    ADC_CH_ADC2        = ((uint32_t)0x02), /*!< Specifies the ADC channel 2.  */
    ADC_CH_ADC3        = ((uint32_t)0x03), /*!< Specifies the ADC channel 3.  */
    ADC_CH_ADC4        = ((uint32_t)0x04), /*!< Specifies the ADC channel 4.  */
    ADC_CH_ADC5        = ((uint32_t)0x05), /*!< Specifies the ADC channel 5.  */
    ADC_CH_ADC6        = ((uint32_t)0x06), /*!< Specifies the ADC channel 6.  */
    ADC_CH_ADC7        = ((uint32_t)0x07), /*!< Specifies the ADC channel 7.  */
    ADC_CH_ADC8        = ((uint32_t)0x08), /*!< Specifies the ADC channel 8.  */
    ADC_CH_ADC9        = ((uint32_t)0x09), /*!< Specifies the ADC channel 9.  */
    ADC_CH_ADC10       = ((uint32_t)0x0A), /*!< Specifies the ADC channel 10. */
    ADC_CH_ADC11       = ((uint32_t)0x0B), /*!< Specifies the ADC channel 11. */
    ADC_CH_ADC12       = ((uint32_t)0x0C), /*!< Specifies the ADC channel 12. */
    ADC_CH_ADC13       = ((uint32_t)0x0D), /*!< Specifies the ADC channel 13. */
    ADC_CH_ADC14       = ((uint32_t)0x0E), /*!< Specifies the ADC channel 14. */
    ADC_CH_ADC15       = ((uint32_t)0x0F), /*!< Specifies the ADC channel 15. */
    ADC_CH_INT_VREF    = ((uint32_t)0x1E), /*!< Specifies the ADC channel 30 (Internal VRef). */
    ADC_CH_TEMP_SENSOR = ((uint32_t)0x1F)  /*!< Specifies the ADC channel 31 (Temperature Sensor). */
} ADCx_Channel_Number;

#define IS_ADC2_CH_NUM(NUM) (((NUM) == ADC_CH_ADC0 ) || \
                             ((NUM) == ADC_CH_ADC1 ) || \
                             ((NUM) == ADC_CH_ADC2 ) || \
                             ((NUM) == ADC_CH_ADC3 ) || \
                             ((NUM) == ADC_CH_ADC4 ) || \
                             ((NUM) == ADC_CH_ADC5 ) || \
                             ((NUM) == ADC_CH_ADC6 ) || \
                             ((NUM) == ADC_CH_ADC7 ) || \
                             ((NUM) == ADC_CH_ADC8 ) || \
                             ((NUM) == ADC_CH_ADC9 ) || \
                             ((NUM) == ADC_CH_ADC10) || \
                             ((NUM) == ADC_CH_ADC11) || \
                             ((NUM) == ADC_CH_ADC12) || \
                             ((NUM) == ADC_CH_ADC13) || \
                             ((NUM) == ADC_CH_ADC14) || \
                             ((NUM) == ADC_CH_ADC15))

#define IS_ADC1_CH_NUM(NUM)  ((IS_ADC2_CH_NUM(NUM))        || \
                             ((NUM) == ADC_CH_INT_VREF   ) || \
                             ((NUM) == ADC_CH_TEMP_SENSOR))

/**
  * @brief ADC Level Control configuration
  */
typedef enum
{
    ADC_LEVEL_CONTROL_Disable = (((uint32_t)0x0) << ADC1_CFG_REG_RNGC_Pos), /*!< Disables Level Control. */
    ADC_LEVEL_CONTROL_Enable  = (((uint32_t)0x1) << ADC1_CFG_REG_RNGC_Pos)  /*!< Enables Level Control. */
} ADCx_Level_Control;

#define IS_ADC_LEVEL_CONTROL_CONFIG(CONFIG) (((CONFIG) == ADC_LEVEL_CONTROL_Disable) || \
                                             ((CONFIG) == ADC_LEVEL_CONTROL_Enable ))

#define ADC_VALUE_MAX                       (0x0FFF)
#define IS_ADC_VALUE(VALUE)     (((VALUE) & ~ADC_VALUE_MAX) == 0)

/**
  * @brief ADC Voltage Reference Source configuration
  */
typedef enum
{
    ADC_VREF_SOURCE_INTERNAL = (((uint32_t)0x0) << ADC1_CFG_M_REF_Pos), /*!< Selects Internal Voltage Reference. */
    ADC_VREF_SOURCE_EXTERNAL = (((uint32_t)0x1) << ADC1_CFG_M_REF_Pos)  /*!< Selects External Voltage Reference. */
} ADCx_VRef_Source;

#define IS_ADC_VREF_SOURCE_CONFIG(CONFIG) (((CONFIG) == ADC_VREF_SOURCE_INTERNAL) || \
                                           ((CONFIG) == ADC_VREF_SOURCE_EXTERNAL))

/**
  * @brief ADC Internal Voltage Reference Source configuration
  */
typedef enum
{
    ADC_INT_VREF_SOURCE_INEXACT = ((uint32_t)0x0), /*!< Selects inexact Internal Voltage Reference. */
    ADC_INT_VREF_SOURCE_EXACT   = ((uint32_t)0x1)  /*!< Selects exact Internal Voltage Reference (from Temperature Sensor). */
} ADCx_Int_VRef_Source;

#define IS_ADC_INT_VREF_SOURCE_CONFIG(CONFIG) (((CONFIG) == ADC_INT_VREF_SOURCE_INEXACT) || \
                                               ((CONFIG) == ADC_INT_VREF_SOURCE_EXACT  ))

/**
  * @brief ADC Clock Prescaler
  */
typedef enum
{
    ADC_CLK_div_None = (((uint32_t)0x0) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 1. */
    ADC_CLK_div_2    = (((uint32_t)0x1) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 2. */
    ADC_CLK_div_4    = (((uint32_t)0x2) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 4. */
    ADC_CLK_div_8    = (((uint32_t)0x3) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 8. */
    ADC_CLK_div_16   = (((uint32_t)0x4) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 16. */
    ADC_CLK_div_32   = (((uint32_t)0x5) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 32. */
    ADC_CLK_div_64   = (((uint32_t)0x6) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 64. */
    ADC_CLK_div_128  = (((uint32_t)0x7) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 128. */
    ADC_CLK_div_256  = (((uint32_t)0x8) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 256. */
    ADC_CLK_div_512  = (((uint32_t)0x9) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 512. */
    ADC_CLK_div_1024 = (((uint32_t)0xA) << ADC1_CFG_REG_DIVCLK_Pos), /*!< The input ADC clock devides by 1024.*/
    ADC_CLK_div_2048 = (((uint32_t)0xB) << ADC1_CFG_REG_DIVCLK_Pos)  /*!< The input ADC clock devides by 2048. */
} ADCx_Prescaler;

#define IS_ADC_CLK_div_VALUE(VALUE) (((VALUE) == ADC_CLK_div_None) || \
                                     ((VALUE) == ADC_CLK_div_2   ) || \
                                     ((VALUE) == ADC_CLK_div_4   ) || \
                                     ((VALUE) == ADC_CLK_div_8   ) || \
                                     ((VALUE) == ADC_CLK_div_16  ) || \
                                     ((VALUE) == ADC_CLK_div_32  ) || \
                                     ((VALUE) == ADC_CLK_div_64  ) || \
                                     ((VALUE) == ADC_CLK_div_128 ) || \
                                     ((VALUE) == ADC_CLK_div_256 ) || \
                                     ((VALUE) == ADC_CLK_div_512 ) || \
                                     ((VALUE) == ADC_CLK_div_1024) || \
                                     ((VALUE) == ADC_CLK_div_2048))

/**
  * @brief ADCx Flags
  */
typedef enum
{
    ADCx_FLAG_OVERWRITE         = ADC_STATUS_FLG_REG_OVERWRITE,
    ADCx_FLAG_OUT_OF_RANGE      = ADC_STATUS_FLG_REG_AWOIFEN,
    ADCx_FLAG_END_OF_CONVERSION = ADC_STATUS_FLG_REG_EOCIF
} ADCx_Flags;

#define IS_ADCx_STATUS_FLAG(FLAG) (((FLAG) == ADCx_FLAG_OVERWRITE)    || \
                                   ((FLAG) == ADCx_FLAG_OUT_OF_RANGE) || \
                                   ((FLAG) == ADCx_FLAG_END_OF_CONVERSION))

#define IS_ADCx_STATUS_FLAGS(FLAGS) (((FLAGS) & ~(ADCx_FLAG_OVERWRITE | \
                                     ADCx_FLAG_OUT_OF_RANGE | \
                                     ADCx_FLAG_END_OF_CONVERSION)) == 0x0)

/**
  * @brief ADC Flags
  */
typedef enum
{
    ADC1_FLAG_OVERWRITE         = (ADCx_FLAG_OVERWRITE         <<  0),
    ADC1_FLAG_OUT_OF_RANGE      = (ADCx_FLAG_OUT_OF_RANGE      <<  0),
    ADC1_FLAG_END_OF_CONVERSION = (ADCx_FLAG_END_OF_CONVERSION <<  0),
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    ADC2_FLAG_OVERWRITE         = (ADCx_FLAG_OVERWRITE         << 16),
    ADC2_FLAG_OUT_OF_RANGE      = (ADCx_FLAG_OUT_OF_RANGE      << 16),
    ADC2_FLAG_END_OF_CONVERSION = (ADCx_FLAG_END_OF_CONVERSION << 16)
#endif
} ADC_Flags;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    #define IS_ADC_STATUS_FLAG(FLAG) (((FLAG) == ADC1_FLAG_OVERWRITE        ) || \
                                      ((FLAG) == ADC1_FLAG_OUT_OF_RANGE     ) || \
                                      ((FLAG) == ADC1_FLAG_END_OF_CONVERSION) || \
                                      ((FLAG) == ADC2_FLAG_OVERWRITE        ) || \
                                      ((FLAG) == ADC2_FLAG_OUT_OF_RANGE     ) || \
                                      ((FLAG) == ADC2_FLAG_END_OF_CONVERSION))    
    
    #define IS_ADC_STATUS_FLAGS(FLAGS)  (((FLAGS) & ~(ADC1_FLAG_OVERWRITE | \
                                         ADC1_FLAG_OUT_OF_RANGE      | \
                                         ADC1_FLAG_END_OF_CONVERSION | \
                                         ADC2_FLAG_OVERWRITE         | \
                                         ADC2_FLAG_OUT_OF_RANGE      | \
                                         ADC2_FLAG_END_OF_CONVERSION)) == 0x0)
#endif /* #if defined  (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    #define IS_ADC_STATUS_FLAG(FLAG) (((FLAG) == ADC1_FLAG_OVERWRITE        ) || \
                                      ((FLAG) == ADC1_FLAG_OUT_OF_RANGE     ) || \
                                      ((FLAG) == ADC1_FLAG_END_OF_CONVERSION))
    
    #define IS_ADC_STATUS_FLAGS(FLAGS)  (((FLAGS) & ~(ADC1_FLAG_OVERWRITE | \
                                         ADC1_FLAG_OUT_OF_RANGE           | \
                                         ADC1_FLAG_END_OF_CONVERSION)) == 0x0)
#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief ADCx Interrupt definition
  */
typedef enum
{
    ADCx_IT_OUT_OF_RANGE      = ADC_STATUS_AWOIF_IE,
    ADCx_IT_END_OF_CONVERSION = ADC_STATUS_ECOIF_IE
} ADCx_IT_Def;

#define IS_ADCx_CONFIG_IT(IT)           (((IT) == ADCx_IT_OUT_OF_RANGE) || \
                                         ((IT) == ADCx_IT_END_OF_CONVERSION))

#define IS_ADCx_CONFIG_ITS(ITS)         (((ITS) & ~(ADCx_IT_OUT_OF_RANGE| \
                                         ADCx_IT_END_OF_CONVERSION)) == 0x0)

/**
  * @brief ADC Interrupt definition
  */
typedef enum
{
    ADC1_IT_OUT_OF_RANGE      = (ADCx_IT_OUT_OF_RANGE      <<  0),
    ADC1_IT_END_OF_CONVERSION = (ADCx_IT_END_OF_CONVERSION <<  0),
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    ADC2_IT_OUT_OF_RANGE      = (ADCx_IT_OUT_OF_RANGE      << 16),
    ADC2_IT_END_OF_CONVERSION = (ADCx_IT_END_OF_CONVERSION << 16)
#endif
} ADC_IT_Def;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    #define IS_ADC_CONFIG_IT(IT)            (((IT) == ADC1_IT_OUT_OF_RANGE     ) || \
                                             ((IT) == ADC1_IT_END_OF_CONVERSION) || \
                                             ((IT) == ADC2_IT_OUT_OF_RANGE     ) || \
                                             ((IT) == ADC2_IT_END_OF_CONVERSION))
    #define IS_ADC_CONFIG_ITS(ITS)          (((ITS) & ~(ADC1_IT_OUT_OF_RANGE | \
                                             ADC1_IT_END_OF_CONVERSION       | \
                                             ADC2_IT_OUT_OF_RANGE            | \
                                             ADC2_IT_END_OF_CONVERSION)) == 0x0)
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    #define IS_ADC_CONFIG_IT(IT)            (((IT) == ADC1_IT_OUT_OF_RANGE     ) || \
                                             ((IT) == ADC1_IT_END_OF_CONVERSION))
    #define IS_ADC_CONFIG_ITS(ITS)          (((ITS) & ~(ADC1_IT_OUT_OF_RANGE | \
                                               ADC1_IT_END_OF_CONVERSION)) == 0x0)
#endif /* #if defined (USE_K1986VE1xI) */


#if defined (USE_K1986VE1xI)

/**
  * @brief ADC Int_VRef Amplifier state
  */
typedef enum
{
    ADC_INT_VREF_AMPLIFIER_Enable  = ((uint32_t)(1 << ADC1_TRIM_SEL_VREF_BUF_Pos)),
    ADC_INT_VREF_AMPLIFIER_Disable = ((uint32_t)(0 << ADC1_TRIM_SEL_VREF_BUF_Pos))
} ADC_Int_VRef_Ampl_State;

#define IS_ADC_INT_VREF_AMPLIFIER(AMPLIFIER)    (((AMPLIFIER) == ADC_INT_VREF_AMPLIFIER_Enable) ||\
                                                 ((AMPLIFIER) == ADC_INT_VREF_AMPLIFIER_Disable))

#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief ADC Init structure definition
  */
typedef struct
{
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    ADC_SyncMode               ADC_SynchronousMode;      /*!< Enables or disables the ADC1, ADC2 synchronous mode operation.
                                                              This parameter can be a value of @ref ADC_SyncMode */
    uint32_t                   ADC_StartDelay;           /*!< Specifies the ADC1 and ADC2 conversion start delay.
                                                              This parameter can be a number between 0 and 15. */
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */
    ADC_Temp_Sensor            ADC_TempSensor;           /*!< Enables or disables the temperature sensor and internal voltage reference.
                                                              This parameter can be a value of @ref ADC_Temp_Sensor */
    ADC_Temp_Sensor_Amplifier  ADC_TempSensorAmplifier;  /*!< Enables or disables the temperature sensor and internal voltage reference amplifier.
                                                              This parameter can be a value of @ref ADC_Temp_Sensor_Amplifier */
    ADC_Temp_Sensor_Conversion ADC_TempSensorConversion; /*!< Enables or disables the temperature sensor conversion.
                                                              This parameter can be a value of @ref ADC_Temp_Sensor_Conversion */
    ADC_Int_VRef_Conversion    ADC_IntVRefConversion;    /*!< Enables or disables the internal voltage reference conversion.
                                                              This parameter can be a value of @ref ADC_Int_VRef_Conversion */
    uint32_t                   ADC_IntVRefTrimming;      /*!< Configures the internal voltage reference trimming. @ref ADC_Int_VRef_Trimming
                                                              This parameter can be a number between 0 and 7 for MDR32F9Q2I, K1986VE9xI, and MDR32FG16S1QI;
                                                                                and a number between 0 and 31 for MDR32F1QI, K1986VE1xI. */
#if defined (USE_K1986VE1xI)
    ADC_Int_VRef_Ampl_State    ADC_IntVRefAmplifier;     /*!< Enable or disable the internal voltage reference amplifier.
                                                              This parameter can be a value of @ref ADC_Int_VRef_Ampl_State */
#endif /* #if defined (USE_K1986VE1xI) */
} ADC_InitTypeDef;

/**
  * @brief  ADC1, ADC2 Init structure definition
  */
typedef struct
{
    ADCx_Clock_Source      ADC_ClockSource;      /*!< Specifies the ADCx clock source.
                                                      This parameter can be a value of @ref ADCx_Clock_Source */

    ADCx_Sampling_Mode     ADC_SamplingMode;     /*!< Specifies the ADCx sampling mode.
                                                      This parameter can be a value of @ref ADCx_Sampling_Mode */

    ADCx_Channel_Switching ADC_ChannelSwitching; /*!< Enables or disables the ADCx channel switching.
                                                      This parameter can be a value of @ref ADCx_Channel_Switching */

    ADCx_Channel_Number    ADC_ChannelNumber;    /*!< Specifies the ADCx channel number.
                                                      This parameter can be a value of @ref ADCx_Channel_Number */

    uint32_t               ADC_Channels;         /*!< Specifies the ADCx channels mask.
                                                      This parameter can be a value of @ref ADCx_Channels */

    ADCx_Level_Control     ADC_LevelControl;     /*!< Enables or disables the ADCx level control.
                                                      This parameter can be a value of @ref ADCx_Level_Control */

    uint16_t               ADC_LowLevel;         /*!< Specifies the ADCx value low level.
                                                      This parameter can be a number between 0x0000 and 0x0FFF. */

    uint16_t               ADC_HighLevel;        /*!< Specifies the ADCx value high level.
                                                      This parameter can be a number between 0x0000 and 0x0FFF. */

    ADCx_VRef_Source       ADC_VRefSource;       /*!< Specifies the ADCx voltage reference source (internal or external).
                                                      This parameter can be a value of @ref ADCx_VRef_Source */

    ADCx_Int_VRef_Source   ADC_IntVRefSource;    /*!< Specifies the ADCx internal voltage reference source (inexact or exact).
                                                      This parameter can be a value of @ref ADCx_Int_VRef_Source */

    ADCx_Prescaler         ADC_Prescaler;        /*!< Specifies the ADCx Prescaler configuration.
                                                      This parameter can be a value of @ref ADCx_Prescaler */

    uint32_t               ADC_DelayGo;          /*!< Specifies the ADCx start conversion delay at sequential conversion mode.
                                                      This parameter can be a number between 0 and 7. */
} ADCx_InitTypeDef;

/** @} */ /* End of group ADC_Exported_Types */


/** @defgroup ADC_Exported_Functions ADC Exported Functions
  * @{
  */

void ADC_DeInit(void);

void ADC_Init(const ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);

void ADC_SetTrim(uint8_t Trim);

void ADC1_Init(const ADCx_InitTypeDef* ADCx_InitStruct);
void ADCx_StructInit(ADCx_InitTypeDef* ADCx_InitStruct);

void ADC1_Cmd(FunctionalState NewState);

void ADC1_SetChannel(ADCx_Channel_Number Channel);
void ADC1_SetChannels(uint32_t ChannelMask);

void ADC1_OperationModeConfig(ADCx_Sampling_Mode SamplingMode, ADCx_Channel_Switching SwitchingMode);
void ADC1_SamplingModeConfig(ADCx_Sampling_Mode SamplingMode);
void ADC1_ChannelSwithingConfig(ADCx_Channel_Switching SwitchingMode);

void ADC1_LevelsConfig(uint32_t LowLevel, uint32_t HighLevel, ADCx_Level_Control NewState);
void ADC1_SetLowLevel(uint32_t LowLevel);
void ADC1_SetHighLevel(uint32_t HighLevel);

void ADC1_Start(void);

uint32_t ADC1_GetResult(void);

uint32_t ADC_GetStatus(void);
uint32_t ADC1_GetStatus(void);

FlagStatus ADC_GetFlagStatus(uint32_t Flags);
FlagStatus ADC1_GetFlagStatus(uint32_t Flags);

void ADC1_ClearOverwriteFlag(void);
void ADC1_ClearOutOfRangeFlag(void);

void ADC_ITConfig(uint32_t ADC_IT, FunctionalState NewState);
void ADC1_ITConfig(uint32_t ADCx_IT, FunctionalState NewState);

ITStatus ADC_GetITStatus(uint32_t ADC_IT);
ITStatus ADC1_GetITStatus(uint32_t ADCx_IT);


#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void ADC2_Init(const ADCx_InitTypeDef* ADCx_InitStruct);
    void ADC2_Cmd(FunctionalState NewState);
    void ADC2_SetChannel(ADCx_Channel_Number Channel);
    void ADC2_SetChannels(uint32_t ChannelMask);
    void ADC2_OperationModeConfig(ADCx_Sampling_Mode SamplingMode, ADCx_Channel_Switching SwitchingMode);
    void ADC2_OperationModeConfig(ADCx_Sampling_Mode SamplingMode, ADCx_Channel_Switching SwitchingMode);
    void ADC2_SamplingModeConfig(ADCx_Sampling_Mode SamplingMode);
    void ADC2_ChannelSwithingConfig(ADCx_Channel_Switching SwitchingMode);
    void ADC2_LevelsConfig(uint32_t LowLevel, uint32_t HighLevel, ADCx_Level_Control NewState);
    void ADC2_SetLowLevel(uint32_t LowLevel);
    void ADC2_SetHighLevel(uint32_t HighLevel);
    void ADC2_Start(void);
    uint32_t ADC2_GetResult(void);
    uint32_t ADC2_GetStatus(void);
    FlagStatus ADC2_GetFlagStatus(uint32_t Flags);
    void ADC2_ClearOverwriteFlag(void);
    void ADC2_ClearOutOfRangeFlag(void);
    void ADC2_ITConfig(uint32_t ADCx_IT, FunctionalState NewState);
    ITStatus ADC2_GetITStatus(uint32_t ADCx_IT);
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group ADC_Exported_Functions */

/** @} */ /* End of group ADC */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_ADC_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_adc.h */


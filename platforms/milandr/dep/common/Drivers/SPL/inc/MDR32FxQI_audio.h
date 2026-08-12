/**
  ******************************************************************************
  * @file    MDR32FxQI_audio.h
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    29/06/2023
  * @brief   This file contains all the functions prototypes for the AUDIO_IP
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
#ifndef MDR32FxQI_AUDIO_H_
#define MDR32FxQI_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @addtogroup AUDIO_IP
  * @warning This module can be used only for MDR32FG16S1QI microcontroller's.
  * @{
  */

/** @defgroup AUDIO_IP_Exported_Types AUDIO_IP Exported Types
  * @{
  */

/**
  * @brief AUDIO_IP AUC_C1
  */
typedef enum
{
    AUDIO_IP_AUC_C1_HSI   = 0x00,
    AUDIO_IP_AUC_C1_HSI_2 = 0x01,
    AUDIO_IP_AUC_C1_HSE   = 0x02,
    AUDIO_IP_AUC_C1_HSE_2 = 0x03
} AUDIO_IP_AUC_C1;

#define IS_AUDIO_IP_AUC_C1(AUC_C1)  (((AUC_C1) == AUDIO_IP_AUC_C1_HSI)   || \
                                     ((AUC_C1) == AUDIO_IP_AUC_C1_HSI_2) || \
                                     ((AUC_C1) == AUDIO_IP_AUC_C1_HSE)   || \
                                     ((AUC_C1) == AUDIO_IP_AUC_C1_HSE_2))

/**
  * @brief AUDIO_IP AUC_C2
  */
typedef enum
{
    AUDIO_IP_AUC_C2_AUC_C1      = 0x00,
    AUDIO_IP_AUC_C2_AUC_PLLCPU0 = 0x01,
    AUDIO_IP_AUC_C2_AUC_PLLUSB0 = 0x02
} AUDIO_IP_AUC_C2;

#define IS_AUDIO_IP_AUC_C2(AUC_C2)  (((AUC_C2) == AUDIO_IP_C2_AUC_C1)      || \
                                     ((AUC_C2) == AUDIO_IP_C2_AUC_PLLCPU0) || \
                                     ((AUC_C2) == AUDIO_IP_C2_AUC_PLLUSB0))

/**
  * @brief AUDIO_IP AUDIO_C3
  */
typedef enum
{
    AUDIO_IP_AUDIO_CLK_DIV1   = ((uint32_t)0x00),
    AUDIO_IP_AUDIO_CLK_DIV2   = ((uint32_t)0x08),
    AUDIO_IP_AUDIO_CLK_DIV4   = ((uint32_t)0x09),
    AUDIO_IP_AUDIO_CLK_DIV8   = ((uint32_t)0x0A),
    AUDIO_IP_AUDIO_CLK_DIV16  = ((uint32_t)0x0B),
    AUDIO_IP_AUDIO_CLK_DIV32  = ((uint32_t)0x0C),
    AUDIO_IP_AUDIO_CLK_DIV64  = ((uint32_t)0x0D),
    AUDIO_IP_AUDIO_CLK_DIV128 = ((uint32_t)0x0E),
    AUDIO_IP_AUDIO_CLK_DIV256 = ((uint32_t)0x0F)
} AUDIO_IP_AUDIO_C3;

#define IS_AUDIO_IP_AUDIO_CLK_DIV(CLK_DIV)  (((CLK_DIV) >= AUDIO_IP_AUDIO_CLK_DIV1) && \
                                             ((CLK_DIV) <= AUDIO_IP_AUDIO_CLK_DIV256))

/**
  * @brief AUDIO_IP Filter Mode
  */
typedef enum
{
    AUDIO_IP_FILTER_MODE_FIR_FILTER = ((uint32_t)0x0),
    AUDIO_IP_FILTER_MODE_IIR_FILTER = ((uint32_t)0x4)
} AUDIO_IP_Filter_Mode;

#define IS_AUDIO_IP_FILTER_MODE(FILTER_MODE)  (((FILTER_MODE) == AUDIO_IP_FILTER_MODE_FIR_FILTER) || \
                                               ((FILTER_MODE) == AUDIO_IP_FILTER_MODE_IIR_FILTER))

/**
  * @brief AUDIO_IP ADC gain
  */
typedef enum
{
    AUDIO_IP_ADGain_minus42dB = ((uint32_t)0x00 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus41dB = ((uint32_t)0x01 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus40dB = ((uint32_t)0x02 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus39dB = ((uint32_t)0x03 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus38dB = ((uint32_t)0x04 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus37dB = ((uint32_t)0x05 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus36dB = ((uint32_t)0x06 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus35dB = ((uint32_t)0x07 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus34dB = ((uint32_t)0x08 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus33dB = ((uint32_t)0x09 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus32dB = ((uint32_t)0x0A << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus31dB = ((uint32_t)0x0B << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus30dB = ((uint32_t)0x0C << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus29dB = ((uint32_t)0x0D << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus28dB = ((uint32_t)0x0E << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus27dB = ((uint32_t)0x0F << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus26dB = ((uint32_t)0x10 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus25dB = ((uint32_t)0x11 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus24dB = ((uint32_t)0x12 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus23dB = ((uint32_t)0x13 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus22dB = ((uint32_t)0x14 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus21dB = ((uint32_t)0x15 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus20dB = ((uint32_t)0x16 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus19dB = ((uint32_t)0x17 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus18dB = ((uint32_t)0x18 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus17dB = ((uint32_t)0x19 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus16dB = ((uint32_t)0x1A << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus15dB = ((uint32_t)0x1B << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus14dB = ((uint32_t)0x1C << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus13dB = ((uint32_t)0x1D << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus12dB = ((uint32_t)0x1E << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus11dB = ((uint32_t)0x1F << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus10dB = ((uint32_t)0x20 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus9dB  = ((uint32_t)0x21 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus8dB  = ((uint32_t)0x22 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus7dB  = ((uint32_t)0x23 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus6dB  = ((uint32_t)0x24 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus5dB  = ((uint32_t)0x25 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus4dB  = ((uint32_t)0x26 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus3dB  = ((uint32_t)0x27 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus2dB  = ((uint32_t)0x28 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_minus1dB  = ((uint32_t)0x29 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_0dB       = ((uint32_t)0x2A << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus1dB   = ((uint32_t)0x2B << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus2dB   = ((uint32_t)0x2C << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus3dB   = ((uint32_t)0x2D << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus4dB   = ((uint32_t)0x2E << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus5dB   = ((uint32_t)0x2F << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus6dB   = ((uint32_t)0x30 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus7dB   = ((uint32_t)0x31 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus8dB   = ((uint32_t)0x32 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus9dB   = ((uint32_t)0x33 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus10dB  = ((uint32_t)0x34 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus11dB  = ((uint32_t)0x35 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus12dB  = ((uint32_t)0x36 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus13dB  = ((uint32_t)0x37 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus14dB  = ((uint32_t)0x38 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus15dB  = ((uint32_t)0x39 << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus16dB  = ((uint32_t)0x3A << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus17dB  = ((uint32_t)0x3B << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus18dB  = ((uint32_t)0x3C << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus19dB  = ((uint32_t)0x3D << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_plus20dB  = ((uint32_t)0x3E << AUDIO_IP_ADCCTL_ADGAIN_Pos),
    AUDIO_IP_ADGain_MUTE      = ((uint32_t)0x3F << AUDIO_IP_ADCCTL_ADGAIN_Pos)
} AUDIO_IP_ADGain;

#define IS_AUDIO_IP_AD_GAIN(AD_GAIN)    \
            (((AD_GAIN) == AUDIO_IP_ADGain_minus42dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus41dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus40dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus39dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus38dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus37dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus36dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus35dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus34dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus33dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus32dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus31dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus30dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus29dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus28dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus27dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus26dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus25dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus24dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus23dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus22dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus21dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus20dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus19dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus18dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus17dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus16dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus15dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus14dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus13dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus12dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus11dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus10dB) || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus9dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus8dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus7dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus6dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus5dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus4dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus3dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus2dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_minus1dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_0dB)       || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus1dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus2dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus3dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus4dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus5dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus6dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus7dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus8dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus9dB)   || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus10dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus11dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus12dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus13dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus14dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus15dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus16dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus17dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus18dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus19dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_plus20dB)  || \
             ((AD_GAIN) == AUDIO_IP_ADGain_MUTE))

/**
  * @brief AUDIO_IP ADC Input Amplifier Level
  */
typedef enum
{
    AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_0_DB  = ((uint32_t)0x00),
    AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_12_DB = ((uint32_t)0x01),
    AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_6_DB  = ((uint32_t)0x02),
    AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_24_DB = ((uint32_t)0x03)
} AUDIO_IP_ADC_Input_Ampl;

#define IS_AUDIO_IP_INPUT_AMPLIFIER_LEVEL(AMPLIFIER_LEVEL) \
            (((AMPLIFIER_LEVEL) == AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_0_DB)  || \
             ((AMPLIFIER_LEVEL) == AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_6_DB)  || \
             ((AMPLIFIER_LEVEL) == AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_12_DB) || \
             ((AMPLIFIER_LEVEL) == AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_24_DB))

/**
  * @brief AUDIO_IP ADC Analog Input Source
  */
typedef enum
{
    AUDIO_IP_ANALOG_INPUT_SOURCE_INP1      = ((uint32_t)0x00),
    AUDIO_IP_ANALOG_INPUT_SOURCE_MICIN_INT = ((uint32_t)0x01),
    AUDIO_IP_ANALOG_INPUT_SOURCE_MICIN_EXT = ((uint32_t)0x02),
    AUDIO_IP_ANALOG_INPUT_SOURCE_INP2      = ((uint32_t)0x03)
} AUDIO_IP_ADC_Input_Src;

#define IS_AUDIO_IP_ANALOG_INPUT_SOURCE(INPUT_SOURCE) \
                (((INPUT_SOURCE) == AUDIO_IP_ANALOG_INPUT_SOURCE_INP1)      || \
                 ((INPUT_SOURCE) == AUDIO_IP_ANALOG_INPUT_SOURCE_MICIN_INT) || \
                 ((INPUT_SOURCE) == AUDIO_IP_ANALOG_INPUT_SOURCE_MICIN_EXT) || \
                 ((INPUT_SOURCE) == AUDIO_IP_ANALOG_INPUT_SOURCE_INP2))

/**
  * @brief AUDIO_IP DAC_SIDE_TONE_LOOP_BACK
  */
typedef enum
{
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_MUTE = ((uint32_t)0x7),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_21dB = ((uint32_t)0x6),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_18dB = ((uint32_t)0x5),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_15dB = ((uint32_t)0x4),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_12dB = ((uint32_t)0x3),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_9dB  = ((uint32_t)0x2),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_6dB  = ((uint32_t)0x1),
    AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_3dB  = ((uint32_t)0x0)
} AUDIO_IP_DAC_Side_Tone;

#define IS_AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK(SIDE_TONE_LOOP_BACK) \
                ((SIDE_TONE_LOOP_BACK) <= AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_MUTE)

/**
  * @brief AUDIO_IP DAC gain
  */
typedef enum
{
    AUDIO_IP_DAGain_minus42dB = ((uint32_t)0x00 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus41dB = ((uint32_t)0x01 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus40dB = ((uint32_t)0x02 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus39dB = ((uint32_t)0x03 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus38dB = ((uint32_t)0x04 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus37dB = ((uint32_t)0x05 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus36dB = ((uint32_t)0x06 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus35dB = ((uint32_t)0x07 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus34dB = ((uint32_t)0x08 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus33dB = ((uint32_t)0x09 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus32dB = ((uint32_t)0x0A << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus31dB = ((uint32_t)0x0B << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus30dB = ((uint32_t)0x0C << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus29dB = ((uint32_t)0x0D << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus28dB = ((uint32_t)0x0E << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus27dB = ((uint32_t)0x0F << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus26dB = ((uint32_t)0x10 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus25dB = ((uint32_t)0x11 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus24dB = ((uint32_t)0x12 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus23dB = ((uint32_t)0x13 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus22dB = ((uint32_t)0x14 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus21dB = ((uint32_t)0x15 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus20dB = ((uint32_t)0x16 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus19dB = ((uint32_t)0x17 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus18dB = ((uint32_t)0x18 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus17dB = ((uint32_t)0x19 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus16dB = ((uint32_t)0x1A << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus15dB = ((uint32_t)0x1B << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus14dB = ((uint32_t)0x1C << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus13dB = ((uint32_t)0x1D << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus12dB = ((uint32_t)0x1E << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus11dB = ((uint32_t)0x1F << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus10dB = ((uint32_t)0x20 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus9dB  = ((uint32_t)0x21 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus8dB  = ((uint32_t)0x22 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus7dB  = ((uint32_t)0x23 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus6dB  = ((uint32_t)0x24 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus5dB  = ((uint32_t)0x25 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus4dB  = ((uint32_t)0x26 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus3dB  = ((uint32_t)0x27 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus2dB  = ((uint32_t)0x28 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_minus1dB  = ((uint32_t)0x29 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_0dB       = ((uint32_t)0x2A << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus1dB   = ((uint32_t)0x2B << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus2dB   = ((uint32_t)0x2C << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus3dB   = ((uint32_t)0x2D << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus4dB   = ((uint32_t)0x2E << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus5dB   = ((uint32_t)0x2F << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus6dB   = ((uint32_t)0x30 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus7dB   = ((uint32_t)0x31 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus8dB   = ((uint32_t)0x32 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus9dB   = ((uint32_t)0x33 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus10dB  = ((uint32_t)0x34 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus11dB  = ((uint32_t)0x35 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus12dB  = ((uint32_t)0x36 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus13dB  = ((uint32_t)0x37 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus14dB  = ((uint32_t)0x38 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus15dB  = ((uint32_t)0x39 << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus16dB  = ((uint32_t)0x3A << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus17dB  = ((uint32_t)0x3B << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus18dB  = ((uint32_t)0x3C << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus19dB  = ((uint32_t)0x3D << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_plus20dB  = ((uint32_t)0x3E << AUDIO_IP_DACCTL_DAGAIN_Pos),
    AUDIO_IP_DAGain_MUTE      = ((uint32_t)0x3F << AUDIO_IP_DACCTL_DAGAIN_Pos)
} AUDIO_IP_DAGain;

#define IS_AUDIO_IP_DA_GAIN(DA_GAIN)    \
            (((DA_GAIN) == AUDIO_IP_DAGain_minus42dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus41dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus40dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus39dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus38dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus37dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus36dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus35dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus34dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus33dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus32dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus31dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus30dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus29dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus28dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus27dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus26dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus25dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus24dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus23dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus22dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus21dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus20dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus19dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus18dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus17dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus16dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus15dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus14dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus13dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus12dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus11dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus10dB) || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus9dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus8dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus7dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus6dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus5dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus4dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus3dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus2dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_minus1dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_0dB)       || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus1dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus2dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus3dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus4dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus5dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus6dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus7dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus8dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus9dB)   || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus10dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus11dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus12dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus13dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus14dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus15dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus16dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus17dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus18dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus19dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_plus20dB)  || \
             ((DA_GAIN) == AUDIO_IP_DAGain_MUTE))

/**
  * @brief AUDIO_IP AUDIO_IP Interrupt Masks
  */
typedef enum
{
    AUDIO_IP_IT_DAOVFM   = ((uint32_t)0x01), /*!< Mask of DAC overflow interrupt */
    AUDIO_IP_IT_ADCVFM   = ((uint32_t)0x02), /*!< Mask of ADC overflow interrupt */
    AUDIO_IP_IT_ADCNSM   = ((uint32_t)0x04), /*!< Mask of ADC FIFO interrupt */
    AUDIO_IP_IT_DACNSM   = ((uint32_t)0x08), /*!< Mask of DAC FIFO interrupt */
    AUDIO_IP_IT_OVERCURM = ((uint32_t)0x10)  /*!< Mask DAC of output short circuit interrupt */
} AUDIO_IP_ITMask;

#define IS_AUDIO_IP_IT(IT)          (((IT) & (~0x000001F)) == 0)

#define IS_AUDIO_IP_CLEAR_IT(IT)    (((IT) & (~0x00000003)) == 0)

/**
  * @brief AUDIO_IP Flags
  */
typedef enum
{
    AUDIO_IP_FLAG_DAOVF   = ((uint32_t)0x01), /*!< DAC overflow interrupt flag */
    AUDIO_IP_FLAG_ADCVF   = ((uint32_t)0x02), /*!< ADC overflow interrupt flag */
    AUDIO_IP_FLAG_ADCNS   = ((uint32_t)0x04), /*!< ADC FIFO interrupt flag */
    AUDIO_IP_FLAG_DACNS   = ((uint32_t)0x08), /*!< DAC FIFO interrupt flag */
    AUDIO_IP_FLAG_OVERCUR = ((uint32_t)0x10)  /*!< DAC of output short circuit interrupt flag */
} AUDIO_IP_Flags;

#define IS_AUDIO_IP_FLAG(FLAG)  (((FLAG) == AUDIO_IP_FLAG_DAOVF) || \
                                 ((FLAG) == AUDIO_IP_FLAG_ADCVF) || \
                                 ((FLAG) == AUDIO_IP_FLAG_ADCNS) || \
                                 ((FLAG) == AUDIO_IP_FLAG_DACNS) || \
                                 ((FLAG) == AUDIO_IP_FLAG_OVERCUR))

/**
  * @brief AUDIO_IP Short Circuit Detection
  */
typedef enum
{
    AUDIO_IP_SHORT_CIRCUIT_DETECTION_OUTPUT_BUFFER = ((uint32_t)0x100),
    AUDIO_IP_SHORT_CIRCUIT_DETECTION_BIAS_CIRCUIT  = ((uint32_t)0x200)
} AUDIO_IP_Short_Circ_Detect_Out_Buff;

#define IS_AUDIO_IP_SHORT_CIRCUIT_DETECTION(SC) \
                    (((SC) == AUDIO_IP_SHORT_CIRCUIT_DETECTION_OUTPUT_BUFFER) || \
                     ((SC) == AUDIO_IP_SHORT_CIRCUIT_DETECTION_BIAS_CIRCUIT))

#define SHORT_CIRCUIT_DETECTION(SC) (((SC) == AUDIO_IP_DACCTL_OVECBA) || \
                                     ((SC) == AUDIO_IP_DACCTL_OVECBS))


/**
  * @brief AUDIO IP Structure defintion
  * @note  The user should not configure all the AUDIO_IP_InitTypeDef structure's fields.
  *        By calling the AUDIO_IP_StructInit function the structure's fields are set to their default values.
  *        Only the parameters that will be set to a non-default value should be configured.
  */
typedef struct
{
    /**
      * @brief POWCTL The general codec control.
      */
    AUDIO_IP_Filter_Mode    FilterMode;                 /*!< Defines the used filter for inderpolation/decimation.
                                                             This parameter can be a value of @ref AUDIO_IP_Filter_Mode. */
    FunctionalState         AnalogLoopBackMode;         /*!< Enable or disable addition the analog loop back.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DigitalLoopBackMode;        /*!< Enable or disable addition the digital loop back.
                                                             This parameter can be a value of @ref FunctionalState */
    /**
      * @brief ADCCTL    The ADC control.
      */

    AUDIO_IP_ADGain         ADGain;                     /*!< Defines recording level of the input signal.
                                                             This parameter can be a value of @ref AUDIO_IP_ADGain */
    AUDIO_IP_ADC_Input_Ampl ADCInputAmplifier;          /*!< Defines the level of input preamplifier.
                                                             This parameter can be value of the@ref AUDIO_IP_ADC_Input_Ampl. */
    AUDIO_IP_ADC_Input_Src  ADCAnalogInput;             /*!< Select the ADC source of the input signal.
                                                             This parameter can be a value of the @ref AUDIO_IP_ADC_Input_Src. */
    FunctionalState         ADCInputControl;            /*!< Control of analog filter and ADC.
                                                             This parameter can be a value of @ref FunctionalState */
    /**
      * @brief DACCTL    The DAC control.
      */

    AUDIO_IP_DAGain         DAGain;                     /*!< Defines the level of playback output signal.
                                                             This parameter can be a value of @ref AUDIO_IP_DAGain */
    FunctionalState         DACMute;                    /*!< Enable or disable the DAC output mute.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DACOutputAmplifier;         /*!< Enable or disable DAC analog output amplifier.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DACBIASOffsetSchemeState;   /*!< Enable or disable offset scheme BIAS.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DACAnalogState;             /*!< Enable or disable analogue DAC and filters.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DACOverCurrentBufferDet;    /*!< Determines the resolution of a short-circuit detection circuit output buffer.
                                                             This parameter can be a value of @ref FunctionalState */
    FunctionalState         DACOverCurrentNetOffsetDet; /*!< Determines the resolution of a short-circuit detection circuit bias circuitry.
                                                             This parameter can be a value of @ref FunctionalState */
    AUDIO_IP_DAC_Side_Tone  DACSideToneLoopBack;        /*!< Set the value of the side tone loob back.
                                                             This parameter can be a value of @ref AUDIO_IP_DAC_Side_Tone. */
    FunctionalState         DACState;                   /*!< Determines the work mode of the AUDIO DAC.
                                                             This parameter can be a value of @ref FunctionalState */
} AUDIO_IP_InitTypeDef;

/** @} */ /* End of group AUDIO_IP_Exported_Types */


/** @defgroup AUDIO_IP_Exported_Functions AUDIO_IP Exported Functions
  * @{
  */
void AUDIO_IP_DeInit(void);
void AUDIO_IP_StructInit(AUDIO_IP_InitTypeDef * AUDIO_IP_InitStruct);
void AUDIO_IP_Init(AUDIO_IP_InitTypeDef * AUDIO_IP_InitStruct);
void AUDIO_IP_ADCCmd(FunctionalState NewState);
void AUDIO_IP_DACCmd(FunctionalState NewState);
void AUDIO_IP_SetADGain(AUDIO_IP_ADGain GainValue);
void AUDIO_IP_SetDAGain(AUDIO_IP_DAGain GainValue);
void AUDIO_IP_SetADCInputAmplifierLevel(AUDIO_IP_ADC_Input_Ampl InputAmplifierLevel);
void AUDIO_IP_SetADCInputSource(AUDIO_IP_ADC_Input_Src ADCInputSource);
void AUDIO_IP_DACMuteCmd(FunctionalState NewState);
void AUDIO_IP_DACOutputAmplifierCmd(FunctionalState NewState);
void AUDIO_IP_OverCurCmd(AUDIO_IP_Short_Circ_Detect_Out_Buff value, FunctionalState NewState);
void AUDIO_IP_ITConfig(AUDIO_IP_ITMask AUDIO_IT, FunctionalState NewState);
ITStatus AUDIO_IP_GetITStatus(AUDIO_IP_Flags AUDIO_IP_FLAG);
void AUDIO_IP_ClearITFlag(AUDIO_IP_ITMask AUDIO_IT);
uint16_t AUDIO_IP_ReceiveData(void);
void AUDIO_IP_SendData(uint16_t AudioData);

/** @} */ /* End of group AUDIO_IP_Exported_Functions */

/** @} */ /* End of group AUDIO_IP */
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* MDR32FxQI_AUDIO_H_ */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_audio.h */



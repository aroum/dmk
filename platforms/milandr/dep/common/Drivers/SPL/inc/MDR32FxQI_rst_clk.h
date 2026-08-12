/**
  ******************************************************************************
  * @file    MDR32FxQI_rst_clk.h
  * @author  Milandr Application Team
  * @version V2.2.0i
  * @date    24/09/2024
  * @brief   This file contains all the functions prototypes for the RST_CLK
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
#ifndef __MDR32FxQI_RST_CLK_H
#define __MDR32FxQI_RST_CLK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup RST_CLK
  * @{
  */

/** @defgroup RST_CLK_Exported_Types RST_CLK Exported Types
  * @{
  */

/**
  * @brief Structure type for modules clocks frequencies expressed in Hz
  */
typedef struct
{
    uint32_t CPU_CLK_Frequency;
    uint32_t USB_CLK_Frequency;
    uint32_t ADC_CLK_Frequency;
    uint32_t RTCHSI_Frequency;
    uint32_t RTCHSE_Frequency;
} RST_CLK_FreqTypeDef;

/**
  * @brief RST_CLK unit non-volatile settings init structure
  */
typedef struct
{
    uint32_t REG_0F;
} Init_NonVolatile_RST_CLK_TypeDef;

/**
  * @brief RST_CLK HSE (High Speed External) clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_HSE_OFF    = ((uint32_t)0x00), /*!< Switch off HSE clock generator. */
    RST_CLK_HSE_ON     = ((uint32_t)0x01), /*!< Switch on HSE clock generator. */
    RST_CLK_HSE_Bypass = ((uint32_t)0x02)  /*!< Use external clock source. */
} RST_CLK_HSE_Mode;

#define IS_RST_CLK_HSE(HSE)     (((HSE) == RST_CLK_HSE_OFF) || \
                                 ((HSE) == RST_CLK_HSE_ON)  || \
                                 ((HSE) == RST_CLK_HSE_Bypass))

#if defined (USE_K1986VE1xI)
/**
  * @brief RST_CLK HSE2 (High Speed External 2) clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_HSE2_OFF    = ((uint32_t)0x00), /*!< Switch off HSE2 clock generator. */
    RST_CLK_HSE2_ON     = ((uint32_t)0x04), /*!< Switch on HSE2 clock generator. */
    RST_CLK_HSE2_Bypass = ((uint32_t)0x08)  /*!< Use external clock source. */
} RST_CLK_HSE2_Mode;

#define IS_RST_CLK_HSE2(HSE2)   (((HSE2) == RST_CLK_HSE2_OFF) || \
                                 ((HSE2) == RST_CLK_HSE2_ON)  || \
                                 ((HSE2) == RST_CLK_HSE2_Bypass))
#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief RST_CLK LSE (Low Speed External) clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_LSE_OFF    = ((uint32_t)0x00), /*!< Switch off LSE clock generator. */
    RST_CLK_LSE_ON     = ((uint32_t)0x01), /*!< Switch on LSE clock generator. */
    RST_CLK_LSE_Bypass = ((uint32_t)0x02)  /*!< Use external clock source. */
} RST_CLK_LSE_Mode;

#define IS_RST_CLK_LSE(LSE)     (((LSE) == RST_CLK_LSE_OFF) || \
                                 ((LSE) == RST_CLK_LSE_ON)  || \
                                 ((LSE) == RST_CLK_LSE_Bypass))

/**
  * @brief RST_CLK CPU_PLL clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_CPU_PLLsrcHSIdiv1 = ((uint32_t)0x00), /*!< HSI oscillator clock is selected as CPU_PLL clock entry. */
    RST_CLK_CPU_PLLsrcHSIdiv2 = ((uint32_t)0x01), /*!< HSI oscillator clock divided by 2 is selected as CPU_PLL clock entry. */
    RST_CLK_CPU_PLLsrcHSEdiv1 = ((uint32_t)0x02), /*!< HSE oscillator clock is selected as CPU_PLL clock entry. */
    RST_CLK_CPU_PLLsrcHSEdiv2 = ((uint32_t)0x03)  /*!< HSE oscillator clock divided by 2 is selected as CPU_PLL clock entry. */
} RST_CLK_CPU_PLL_Source;

#define IS_RST_CLK_CPU_PLL_SOURCE(SRC)  (((SRC) == RST_CLK_CPU_PLLsrcHSIdiv1) || \
                                         ((SRC) == RST_CLK_CPU_PLLsrcHSIdiv2) || \
                                         ((SRC) == RST_CLK_CPU_PLLsrcHSEdiv1) || \
                                         ((SRC) == RST_CLK_CPU_PLLsrcHSEdiv2))

/**
  * @brief RST_CLK CPU_C1 clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_CPU_C1srcHSIdiv1 = ((uint32_t)0x00),
    RST_CLK_CPU_C1srcHSIdiv2 = ((uint32_t)0x01),
    RST_CLK_CPU_C1srcHSEdiv1 = ((uint32_t)0x02),
    RST_CLK_CPU_C1srcHSEdiv2 = ((uint32_t)0x03)
} RST_CLK_CPU_C1_Source;

#define IS_RST_CLK_CPU_C1_SOURCE(SRC)   (((SRC) == RST_CLK_CPU_C1srcHSIdiv1) || \
                                         ((SRC) == RST_CLK_CPU_C1srcHSIdiv2) || \
                                         ((SRC) == RST_CLK_CPU_C1srcHSEdiv1) || \
                                         ((SRC) == RST_CLK_CPU_C1srcHSEdiv2))

/**
  * @brief RST_CLK PLL_CPU_MUL multiplier of the CPU_C1 clock constants
  */
typedef enum
{
    RST_CLK_CPU_PLLmul1  = ((uint32_t)0x00),
#if defined (USE_MDR32F9xI) || defined (USE_MDR32F1QI) || defined (USE_MDR32FG16S1QI)
    RST_CLK_CPU_PLLmul2  = ((uint32_t)0x01),
    RST_CLK_CPU_PLLmul3  = ((uint32_t)0x02),
#endif
    RST_CLK_CPU_PLLmul4  = ((uint32_t)0x03),
    RST_CLK_CPU_PLLmul5  = ((uint32_t)0x04),
    RST_CLK_CPU_PLLmul6  = ((uint32_t)0x05),
    RST_CLK_CPU_PLLmul7  = ((uint32_t)0x06),
    RST_CLK_CPU_PLLmul8  = ((uint32_t)0x07),
    RST_CLK_CPU_PLLmul9  = ((uint32_t)0x08),
    RST_CLK_CPU_PLLmul10 = ((uint32_t)0x09),
    RST_CLK_CPU_PLLmul11 = ((uint32_t)0x0A),
    RST_CLK_CPU_PLLmul12 = ((uint32_t)0x0B),
    RST_CLK_CPU_PLLmul13 = ((uint32_t)0x0C),
    RST_CLK_CPU_PLLmul14 = ((uint32_t)0x0D),
    RST_CLK_CPU_PLLmul15 = ((uint32_t)0x0E),
    RST_CLK_CPU_PLLmul16 = ((uint32_t)0x0F)
} RST_CLK_CPU_PLL_Multiplier;

#if defined (USE_MDR32F9xI) || defined (USE_MDR32F1QI) || defined (USE_MDR32FG16S1QI)
    #define IS_RST_CLK_CPU_PLL_MUL(SRC)     ((SRC) <= 0x0F)
#else
    #define IS_RST_CLK_CPU_PLL_MUL(SRC)     (((SRC) == 0) || \
                                            (((SRC) >= 0x03) && ((SRC) <= 0x0F)))
#endif

/**
  * @brief RST_CLK USB_PLL clock mode and source selection constants
  */
typedef enum
{
    RST_CLK_USB_PLLsrcHSIdiv1 = ((uint32_t)0x00), /*!< HSI oscillator clock is selected as USB_PLL clock entry. */
    RST_CLK_USB_PLLsrcHSIdiv2 = ((uint32_t)0x01), /*!< HSI oscillator clock divided by 2 is selected as USB_PLL clock entry. */
    RST_CLK_USB_PLLsrcHSEdiv1 = ((uint32_t)0x02), /*!< HSE oscillator clock is selected as USB_PLL clock entry. */
    RST_CLK_USB_PLLsrcHSEdiv2 = ((uint32_t)0x03)  /*!< HSE oscillator clock divided by 2 is selected as USB_PLL clock entry. */
} RST_CLK_USB_PLL_Source;

#define IS_RST_CLK_USB_PLL_SOURCE(SRC)  (((SRC) == RST_CLK_USB_PLLsrcHSIdiv1) ||\
                                         ((SRC) == RST_CLK_USB_PLLsrcHSIdiv2) ||\
                                         ((SRC) == RST_CLK_USB_PLLsrcHSEdiv1) ||\
                                         ((SRC) == RST_CLK_USB_PLLsrcHSEdiv2))

/**
  * @brief RST_CLK USB_C1 clock PLL_USB_MUL multiplier constants
  */
typedef enum
{
    RST_CLK_USB_PLLmul1  = ((uint32_t)0x00),
#if defined (USE_MDR32F9xI) || defined (USE_MDR32F1QI) || defined (USE_MDR32FG16S1QI)
    RST_CLK_USB_PLLmul2  = ((uint32_t)0x01),
    RST_CLK_USB_PLLmul3  = ((uint32_t)0x02),
#endif
    RST_CLK_USB_PLLmul4  = ((uint32_t)0x03),
    RST_CLK_USB_PLLmul5  = ((uint32_t)0x04),
    RST_CLK_USB_PLLmul6  = ((uint32_t)0x05),
    RST_CLK_USB_PLLmul7  = ((uint32_t)0x06),
    RST_CLK_USB_PLLmul8  = ((uint32_t)0x07),
    RST_CLK_USB_PLLmul9  = ((uint32_t)0x08),
    RST_CLK_USB_PLLmul10 = ((uint32_t)0x09),
    RST_CLK_USB_PLLmul11 = ((uint32_t)0x0A),
    RST_CLK_USB_PLLmul12 = ((uint32_t)0x0B),
    RST_CLK_USB_PLLmul13 = ((uint32_t)0x0C),
    RST_CLK_USB_PLLmul14 = ((uint32_t)0x0D),
    RST_CLK_USB_PLLmul15 = ((uint32_t)0x0E),
    RST_CLK_USB_PLLmul16 = ((uint32_t)0x0F)
} RST_CLK_USB_PLL_Multiplier;

#if defined (USE_MDR32F9xI) || defined (USE_MDR32F1QI) || defined (USE_MDR32FG16S1QI)
    #define IS_RST_CLK_USB_PLL_MUL(SRC)     ((SRC) <= 0x0F)
#else
    #define IS_RST_CLK_USB_PLL_MUL(SRC)     (((SRC) == 0) || \
                                            (((SRC) >= 0x03) && ((SRC) <= 0x0F)))
#endif

/**
  * @brief RST_CLK CPU_PLL output clock CPU_CLK_DIV divider constants
  */
typedef enum
{
    RST_CLK_CPUclkDIV1   = ((uint32_t)0x00),
    RST_CLK_CPUclkDIV2   = ((uint32_t)0x08),
    RST_CLK_CPUclkDIV4   = ((uint32_t)0x09),
    RST_CLK_CPUclkDIV8   = ((uint32_t)0x0A),
    RST_CLK_CPUclkDIV16  = ((uint32_t)0x0B),
    RST_CLK_CPUclkDIV32  = ((uint32_t)0x0C),
    RST_CLK_CPUclkDIV64  = ((uint32_t)0x0D),
    RST_CLK_CPUclkDIV128 = ((uint32_t)0x0E),
    RST_CLK_CPUclkDIV256 = ((uint32_t)0x0F)
} RST_CLK_CPU_C3_Divisor;

#define IS_RST_CLK_CPUclkDIV(DIV)   (((DIV) == RST_CLK_CPUclkDIV1)   || \
                                     ((DIV) == RST_CLK_CPUclkDIV2)   || \
                                     ((DIV) == RST_CLK_CPUclkDIV4)   || \
                                     ((DIV) == RST_CLK_CPUclkDIV8)   || \
                                     ((DIV) == RST_CLK_CPUclkDIV16)  || \
                                     ((DIV) == RST_CLK_CPUclkDIV32)  || \
                                     ((DIV) == RST_CLK_CPUclkDIV64)  || \
                                     ((DIV) == RST_CLK_CPUclkDIV128) || \
                                     ((DIV) == RST_CLK_CPUclkDIV256))

/**
  * @brief RST_CLK CPU_CLK source CPU_CLK selector constants
  */
typedef enum
{
    RST_CLK_CPUclkHSI    = ((uint32_t)0x0000),
    RST_CLK_CPUclkCPU_C3 = ((uint32_t)0x0100),
    RST_CLK_CPUclkLSE    = ((uint32_t)0x0200),
    RST_CLK_CPUclkLSI    = ((uint32_t)0x0300)
} RST_CLK_HCLK_Source;

#define IS_RST_CPU_CLK(SRC)     (((SRC) == RST_CLK_CPUclkHSI)    || \
                                 ((SRC) == RST_CLK_CPUclkCPU_C3) || \
                                 ((SRC) == RST_CLK_CPUclkLSE)    || \
                                 ((SRC) == RST_CLK_CPUclkLSI))

/**
  * @brief RST_CLK ADC_CLK selector constants
  */
typedef enum
{
    RST_CLK_ADCclkCPU_C1 = ((uint32_t)0x0020),
    RST_CLK_ADCclkUSB_C1 = ((uint32_t)0x0021),
    RST_CLK_ADCclkCPU_C2 = ((uint32_t)0x0022),
    RST_CLK_ADCclkUSB_C2 = ((uint32_t)0x0023),
    RST_CLK_ADCclkLSE    = ((uint32_t)0x0000),
    RST_CLK_ADCclkLSI    = ((uint32_t)0x0010),
    RST_CLK_ADCclkHSI_C1 = ((uint32_t)0x0030)
} RST_CLK_ADC_Source;

#define IS_RST_CLK_ADCclk(SRC)  (((SRC) == RST_CLK_ADCclkCPU_C1) || \
                                 ((SRC) == RST_CLK_ADCclkUSB_C1) || \
                                 ((SRC) == RST_CLK_ADCclkCPU_C2) || \
                                 ((SRC) == RST_CLK_ADCclkUSB_C2) || \
                                 ((SRC) == RST_CLK_ADCclkLSE)    || \
                                 ((SRC) == RST_CLK_ADCclkLSI)    || \
                                 ((SRC) == RST_CLK_ADCclkHSI_C1))

/**
  * @brief RST_CLK ADC_CS2_SEL output clock ADC_CS3_SEL divider constants
  */
typedef enum
{
    RST_CLK_ADCclkDIV1   = ((uint32_t)0x00),
    RST_CLK_ADCclkDIV2   = ((uint32_t)0x08),
    RST_CLK_ADCclkDIV4   = ((uint32_t)0x09),
    RST_CLK_ADCclkDIV8   = ((uint32_t)0x0A),
    RST_CLK_ADCclkDIV16  = ((uint32_t)0x0B),
    RST_CLK_ADCclkDIV32  = ((uint32_t)0x0C),
    RST_CLK_ADCclkDIV64  = ((uint32_t)0x0D),
    RST_CLK_ADCclkDIV128 = ((uint32_t)0x0E),
    RST_CLK_ADCclkDIV256 = ((uint32_t)0x0F)
} RST_CLK_ADC_C3_Divisor;

#define IS_RST_CLK_ADCclkDivValue(SRC)  (((SRC) == RST_CLK_ADCclkDIV1)   || \
                                         ((SRC) == RST_CLK_ADCclkDIV2)   || \
                                         ((SRC) == RST_CLK_ADCclkDIV4)   || \
                                         ((SRC) == RST_CLK_ADCclkDIV8)   || \
                                         ((SRC) == RST_CLK_ADCclkDIV16)  || \
                                         ((SRC) == RST_CLK_ADCclkDIV32)  || \
                                         ((SRC) == RST_CLK_ADCclkDIV64)  || \
                                         ((SRC) == RST_CLK_ADCclkDIV128) || \
                                         ((SRC) == RST_CLK_ADCclkDIV256))

#if defined (USE_MDR32FG16S1QI)
/**
 * @brief RST_CLK AUC_CLK selector constants
 */
typedef enum
{
    RST_CLK_AUCclkHSI_C1   = ((uint32_t)0x00000000),
    RST_CLK_AUCclkHSI_2_C1 = ((uint32_t)0x00010000),
    RST_CLK_AUCclkHSE_C1   = ((uint32_t)0x00020000),
    RST_CLK_AUCclkHSE_2_C1 = ((uint32_t)0x00030000),
    RST_CLK_AUCclkPLLCPU   = ((uint32_t)0x00100000),
    RST_CLK_AUCclkPLLUSB   = ((uint32_t)0x00200000)
} RST_CLK_AUC_Source;

#define IS_RST_CLK_AUCclk(SRC)  (((SRC) == RST_CLK_AUCclkHSI_C1)   || \
                                 ((SRC) == RST_CLK_AUCclkHSI_2_C1) || \
                                 ((SRC) == RST_CLK_AUCclkHSE_C1)   || \
                                 ((SRC) == RST_CLK_AUCclkHSE_2_C1) || \
                                 ((SRC) == RST_CLK_AUCclkPLLCPU)   || \
                                 ((SRC) == RST_CLK_AUCclkPLLUSB))

 /**
  * @brief AUC_MCO_CLK AUC_CLK output clock AUC_CLK_DIV divider
  */
typedef enum
{
    RST_CLK_AUCclkDIV1   = ((uint32_t)0x00000000),
    RST_CLK_AUCclkDIV2   = ((uint32_t)0x08000000 | 0x00000000),
    RST_CLK_AUCclkDIV4   = ((uint32_t)0x08000000 | 0x01000000),
    RST_CLK_AUCclkDIV8   = ((uint32_t)0x08000000 | 0x02000000),
    RST_CLK_AUCclkDIV16  = ((uint32_t)0x08000000 | 0x03000000),
    RST_CLK_AUCclkDIV32  = ((uint32_t)0x08000000 | 0x04000000),
    RST_CLK_AUCclkDIV64  = ((uint32_t)0x08000000 | 0x05000000),
    RST_CLK_AUCclkDIV128 = ((uint32_t)0x08000000 | 0x06000000),
    RST_CLK_AUCclkDIV256 = ((uint32_t)0x08000000 | 0x07000000)
} RST_CLK_AUC_C3_Divisor;

#define IS_RST_CLK_AUCclkDIV(DIV)   (((DIV) == RST_CLK_AUCclkDIV1)   || \
                                     ((DIV) == RST_CLK_AUCclkDIV2)   || \
                                     ((DIV) == RST_CLK_AUCclkDIV4)   || \
                                     ((DIV) == RST_CLK_AUCclkDIV8)   || \
                                     ((DIV) == RST_CLK_AUCclkDIV16)  || \
                                     ((DIV) == RST_CLK_AUCclkDIV32)  || \
                                     ((DIV) == RST_CLK_AUCclkDIV64)  || \
                                     ((DIV) == RST_CLK_AUCclkDIV128) || \
                                     ((DIV) == RST_CLK_AUCclkDIV256))
#endif /* #if defined (USE_MDR32FG16S1QI) */

/**
  * @brief RST_CLK flag identifiers
  */
typedef enum
{
    RST_CLK_FLAG_HSIRDY    = ((uint32_t)(0x00 | 23)),
    RST_CLK_FLAG_LSIRDY    = ((uint32_t)(0x00 | 21)),
    RST_CLK_FLAG_HSERDY    = ((uint32_t)(0x20 |  2)),
    RST_CLK_FLAG_HSE2RDY   = ((uint32_t)(0x20 |  3)),
    RST_CLK_FLAG_LSERDY    = ((uint32_t)(0x00 | 13)),
    RST_CLK_FLAG_PLLCPURDY = ((uint32_t)(0x20 |  1)),
    RST_CLK_FLAG_PLLUSBRDY = ((uint32_t)(0x20 |  0)),
    RST_CLK_FLAG_PLLDSPRDY = ((uint32_t)(0x20 |  3))
} RST_CLK_Flags;

#define IS_RST_CLK_FLAG(FLAG)   (((FLAG) == RST_CLK_FLAG_HSIRDY)    || \
                                 ((FLAG) == RST_CLK_FLAG_LSIRDY)    || \
                                 ((FLAG) == RST_CLK_FLAG_HSERDY)    || \
                                 ((FLAG) == RST_CLK_FLAG_HSE2RDY)   || \
                                 ((FLAG) == RST_CLK_FLAG_PLLDSPRDY) || \
                                 ((FLAG) == RST_CLK_FLAG_LSERDY)    || \
                                 ((FLAG) == RST_CLK_FLAG_PLLCPURDY) || \
                                 ((FLAG) == RST_CLK_FLAG_PLLUSBRDY))

/**
  * @brief RST_CLK HSI clock HSI_C1_SEL divider constants
  */
typedef enum
{
    RST_CLK_HSIclkDIV1   = ((uint32_t)0x00),
    RST_CLK_HSIclkDIV2   = ((uint32_t)0x08),
    RST_CLK_HSIclkDIV4   = ((uint32_t)0x09),
    RST_CLK_HSIclkDIV8   = ((uint32_t)0x0A),
    RST_CLK_HSIclkDIV16  = ((uint32_t)0x0B),
    RST_CLK_HSIclkDIV32  = ((uint32_t)0x0C),
    RST_CLK_HSIclkDIV64  = ((uint32_t)0x0D),
    RST_CLK_HSIclkDIV128 = ((uint32_t)0x0E),
    RST_CLK_HSIclkDIV256 = ((uint32_t)0x0F)
} RST_CLK_HSI_C1_Divisor;

#define IS_RST_CLK_HSIclkDivValue(SRC)  (((SRC) == RST_CLK_HSIclkDIV1)   || \
                                         ((SRC) == RST_CLK_HSIclkDIV2)   || \
                                         ((SRC) == RST_CLK_HSIclkDIV4)   || \
                                         ((SRC) == RST_CLK_HSIclkDIV8)   || \
                                         ((SRC) == RST_CLK_HSIclkDIV16)  || \
                                         ((SRC) == RST_CLK_HSIclkDIV32)  || \
                                         ((SRC) == RST_CLK_HSIclkDIV64)  || \
                                         ((SRC) == RST_CLK_HSIclkDIV128) || \
                                         ((SRC) == RST_CLK_HSIclkDIV256))

/**
  * @brief RST_CLK HSE clock HSE_C1_SEL divider constants
  */
typedef enum
{
    RST_CLK_HSEclkDIV1   = ((uint32_t)0x00),
    RST_CLK_HSEclkDIV2   = ((uint32_t)0x08),
    RST_CLK_HSEclkDIV4   = ((uint32_t)0x09),
    RST_CLK_HSEclkDIV8   = ((uint32_t)0x0A),
    RST_CLK_HSEclkDIV16  = ((uint32_t)0x0B),
    RST_CLK_HSEclkDIV32  = ((uint32_t)0x0C),
    RST_CLK_HSEclkDIV64  = ((uint32_t)0x0D),
    RST_CLK_HSEclkDIV128 = ((uint32_t)0x0E),
    RST_CLK_HSEclkDIV256 = ((uint32_t)0x0F)
} RST_CLK_HSE_C1_Divisor;

#define IS_RST_CLK_HSEclkDivValue(SRC)  (((SRC) == RST_CLK_HSEclkDIV1)   || \
                                         ((SRC) == RST_CLK_HSEclkDIV2)   || \
                                         ((SRC) == RST_CLK_HSEclkDIV4)   || \
                                         ((SRC) == RST_CLK_HSEclkDIV8)   || \
                                         ((SRC) == RST_CLK_HSEclkDIV16)  || \
                                         ((SRC) == RST_CLK_HSEclkDIV32)  || \
                                         ((SRC) == RST_CLK_HSEclkDIV64)  || \
                                         ((SRC) == RST_CLK_HSEclkDIV128) || \
                                         ((SRC) == RST_CLK_HSEclkDIV256))

#if defined (USE_MDR32FG16S1QI)
/**
  * @brief RST_CLK DSP C1 clock source constants
  */
typedef enum
{
    DSP_C1_CLOCK_HSI  = (uint32_t)(0x00),
    DSP_C1_CLOCK_HSI2 = (uint32_t)(0x01),
    DSP_C1_CLOCK_HSE  = (uint32_t)(0x02),
    DSP_C1_CLOCK_HSE2 = (uint32_t)(0x03)
} RST_CLK_DSP_C1_Source;

#define IS_DSP_C1_CLOCK(CLOCK)  (((CLOCK) == DSP_C1_CLOCK_HSI)  || \
                                 ((CLOCK) == DSP_C1_CLOCK_HSI2) || \
                                 ((CLOCK) == DSP_C1_CLOCK_HSE)  || \
                                 ((CLOCK) == DSP_C1_CLOCK_HSE2))

/**
  * @brief RST_CLK DSP PLL clock DSP_PLL_MUL multiplier constants
  */
typedef enum
{
    DSP_PLL_MUL1  = (uint32_t)(0x00),
    DSP_PLL_MUL2  = (uint32_t)(0x01),
    DSP_PLL_MUL3  = (uint32_t)(0x02),
    DSP_PLL_MUL4  = (uint32_t)(0x03),
    DSP_PLL_MUL5  = (uint32_t)(0x04),
    DSP_PLL_MUL6  = (uint32_t)(0x05),
    DSP_PLL_MUL7  = (uint32_t)(0x06),
    DSP_PLL_MUL8  = (uint32_t)(0x07),
    DSP_PLL_MUL9  = (uint32_t)(0x08),
    DSP_PLL_MUL10 = (uint32_t)(0x09),
    DSP_PLL_MUL11 = (uint32_t)(0x0A),
    DSP_PLL_MUL12 = (uint32_t)(0x0B),
    DSP_PLL_MUL13 = (uint32_t)(0x0C),
    DSP_PLL_MUL14 = (uint32_t)(0x0D),
    DSP_PLL_MUL15 = (uint32_t)(0x0E),
    DSP_PLL_MUL16 = (uint32_t)(0x0F)
} RST_CLK_DSP_PLL_Multiplier;

#define IS_DSP_PLL_MULL(PLL_MUL)    ((PLL_MUL) <= 0x0F)

/**
  * @brief RST_CLK DSP clock DSP_C3_SEL divider constants
  */
typedef enum
{
    DSP_PRESCALER1 = (uint32_t)(0x00),
    DSP_PRESCALER2 = (uint32_t)(0x01)
} RST_CLK_DSP_C3_Divisor;

#define IS_DSP_PRESCALER(PRESCALER)     (((PRESCALER) == DSP_PRESCALER1) || \
                                         ((PRESCALER) == DSP_PRESCALER2))
#endif /* #if defined (USE_MDR32FG16S1QI) */

#if (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
/**
 * @brief RST_CLK DMA_DONE_STICK channels.
 */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_UART1_TX   RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK0  /*!< DMA_DONE signal fixation for channel 0. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_UART1_RX   RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK1  /*!< DMA_DONE signal fixation for channel 1. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_UART2_TX   RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK2  /*!< DMA_DONE signal fixation for channel 2. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_UART2_RX   RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK3  /*!< DMA_DONE signal fixation for channel 3. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP1_TX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK4  /*!< DMA_DONE signal fixation for channel 4. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP1_RX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK5  /*!< DMA_DONE signal fixation for channel 5. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP2_TX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK6  /*!< DMA_DONE signal fixation for channel 6. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP2_RX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK7  /*!< DMA_DONE signal fixation for channel 7. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP3_TX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK8  /*!< DMA_DONE signal fixation for channel 8. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SSP3_RX    RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK9  /*!< DMA_DONE signal fixation for channel 9. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1       RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK10 /*!< DMA_DONE signal fixation for channel 10. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2       RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK11 /*!< DMA_DONE signal fixation for channel 11. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3       RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK12 /*!< DMA_DONE signal fixation for channel 12. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4       RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK13 /*!< DMA_DONE signal fixation for channel 13. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ1  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK14 /*!< DMA_DONE signal fixation for channel 14. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ2  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK15 /*!< DMA_DONE signal fixation for channel 15. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ3  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK16 /*!< DMA_DONE signal fixation for channel 16. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ4  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK17 /*!< DMA_DONE signal fixation for channel 17. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ1  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK18 /*!< DMA_DONE signal fixation for channel 18. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ2  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK19 /*!< DMA_DONE signal fixation for channel 19. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ3  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK20 /*!< DMA_DONE signal fixation for channel 20. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ4  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK21 /*!< DMA_DONE signal fixation for channel 21. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ1  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK22 /*!< DMA_DONE signal fixation for channel 22. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ2  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK23 /*!< DMA_DONE signal fixation for channel 23. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ3  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK24 /*!< DMA_DONE signal fixation for channel 24. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ4  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK25 /*!< DMA_DONE signal fixation for channel 25. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ1  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK26 /*!< DMA_DONE signal fixation for channel 26. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ2  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK27 /*!< DMA_DONE signal fixation for channel 27. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ3  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK28 /*!< DMA_DONE signal fixation for channel 28. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ4  RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK29 /*!< DMA_DONE signal fixation for channel 29. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_ADC        RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK30 /*!< DMA_DONE signal fixation for channel 30. */
#define RST_CLK_DMA_DONE_STICK_CHANNEL_SW1        RST_CLK_DMA_DONE_STICK_DMA_DONE_STICK31  /*!< DMA_DONE signal fixation for channel 31. */


#define IS_RST_CLK_DMA_DONE_STICK_CHANNEL(CH) (((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_UART1_TX)  || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_UART1_RX)  || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_UART2_TX)  || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_UART2_RX)  || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP1_TX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP1_RX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP2_TX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP2_RX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP3_TX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SSP3_RX)   || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1)      || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2)      || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3)      || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4)      || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ1) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ2) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ3) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM1_REQ4) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ1) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ2) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ3) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM2_REQ4) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ1) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ2) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ3) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM3_REQ4) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ1) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ2) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ3) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_TIM4_REQ4) || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_ADC)       || \
                                               ((CH) == RST_CLK_DMA_DONE_STICK_CHANNEL_SW1))
#endif /* #if defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI) */

/** @} */ /* End of group RST_CLK_Exported_Types */


/** @defgroup RST_CLK_Exported_Constants RST_CLK Exported Constants
  * @{
  */

/** @defgroup CLK_peripheral CLK peripheral
  * @{
  */

/**
  * @brief RST_CLK peripheral modules clock constants
  */

#define PCLK_BIT(BASE)                  ((uint32_t)(1U << ((((uint32_t)BASE) >> 15) & 0x1F)))

#if defined (USE_K1986VE9xI)

    #define RST_CLK_PCLK_CAN1           PCLK_BIT(MDR_CAN1_BASE)
    #define RST_CLK_PCLK_CAN2           PCLK_BIT(MDR_CAN2_BASE)
    #define RST_CLK_PCLK_USB            PCLK_BIT(MDR_USB_BASE)
    #define RST_CLK_PCLK_EEPROM         PCLK_BIT(MDR_EEPROM_BASE)
    #define RST_CLK_PCLK_RST_CLK        PCLK_BIT(MDR_RST_CLK_BASE)
    #define RST_CLK_PCLK_DMA            PCLK_BIT(MDR_DMA_BASE)
    #define RST_CLK_PCLK_UART1          PCLK_BIT(MDR_UART1_BASE)
    #define RST_CLK_PCLK_UART2          PCLK_BIT(MDR_UART2_BASE)
    #define RST_CLK_PCLK_SSP1           PCLK_BIT(MDR_SSP1_BASE)
    #define RST_CLK_PCLK_09             PCLK_BIT(0x40048000)
    #define RST_CLK_PCLK_I2C            PCLK_BIT(MDR_I2C_BASE)
    #define RST_CLK_PCLK_POWER          PCLK_BIT(MDR_POWER_BASE)
    #define RST_CLK_PCLK_WWDG           PCLK_BIT(MDR_WWDG_BASE)
    #define RST_CLK_PCLK_IWDG           PCLK_BIT(MDR_IWDG_BASE)
    #define RST_CLK_PCLK_TIMER1         PCLK_BIT(MDR_TIMER1_BASE)
    #define RST_CLK_PCLK_TIMER2         PCLK_BIT(MDR_TIMER2_BASE)
    #define RST_CLK_PCLK_TIMER3         PCLK_BIT(MDR_TIMER3_BASE)
    #define RST_CLK_PCLK_ADC            PCLK_BIT(MDR_ADC_BASE)
    #define RST_CLK_PCLK_DAC            PCLK_BIT(MDR_DAC_BASE)
    #define RST_CLK_PCLK_COMP           PCLK_BIT(MDR_COMP_BASE)
    #define RST_CLK_PCLK_SSP2           PCLK_BIT(MDR_SSP2_BASE)
    #define RST_CLK_PCLK_PORTA          PCLK_BIT(MDR_PORTA_BASE)
    #define RST_CLK_PCLK_PORTB          PCLK_BIT(MDR_PORTB_BASE)
    #define RST_CLK_PCLK_PORTC          PCLK_BIT(MDR_PORTC_BASE)
    #define RST_CLK_PCLK_PORTD          PCLK_BIT(MDR_PORTD_BASE)
    #define RST_CLK_PCLK_PORTE          PCLK_BIT(MDR_PORTE_BASE)
    #define RST_CLK_PCLK_26             PCLK_BIT(0x400D0000)
    #define RST_CLK_PCLK_BKP            PCLK_BIT(MDR_BKP_BASE)
    #define RST_CLK_PCLK_28             PCLK_BIT(0x400E0000)
    #define RST_CLK_PCLK_PORTF          PCLK_BIT(MDR_PORTF_BASE)
    #define RST_CLK_PCLK_EBC            PCLK_BIT(MDR_EBC_BASE)
    #define RST_CLK_PCLK_31             PCLK_BIT(0x400F8000)


    #define IS_RST_CLK_PCLK(PCLK)       ((((PCLK) & RST_CLK_PCLK_09) == 0x00) && \
                                         (((PCLK) & RST_CLK_PCLK_26) == 0x00) && \
                                         (((PCLK) & RST_CLK_PCLK_28) == 0x00) && \
                                         (((PCLK) & RST_CLK_PCLK_31) == 0x00))
#endif /* #if defined (USE_K1986VE9xI) */

#if defined (USE_MDR32FG16S1QI)

    #define RST_CLK_PCLK_SSP3           PCLK_BIT(MDR_SSP3_BASE)
    #define RST_CLK_PCLK_SSP4           PCLK_BIT(MDR_SSP4_BASE)
    #define RST_CLK_PCLK_USB            PCLK_BIT(MDR_USB_BASE)
    #define RST_CLK_PCLK_EEPROM         PCLK_BIT(MDR_EEPROM_BASE)
    #define RST_CLK_PCLK_RST_CLK        PCLK_BIT(MDR_RST_CLK_BASE)
    #define RST_CLK_PCLK_DMA            PCLK_BIT(MDR_DMA_BASE)
    #define RST_CLK_PCLK_UART1          PCLK_BIT(MDR_UART1_BASE)
    #define RST_CLK_PCLK_UART2          PCLK_BIT(MDR_UART2_BASE)
    #define RST_CLK_PCLK_SSP1           PCLK_BIT(MDR_SSP1_BASE)
    #define RST_CLK_PCLK_SDIO           PCLK_BIT(MDR_SDIO_BASE)
    #define RST_CLK_PCLK_I2C            PCLK_BIT(MDR_I2C_BASE)
    #define RST_CLK_PCLK_POWER          PCLK_BIT(MDR_POWER_BASE)
    #define RST_CLK_PCLK_WWDG           PCLK_BIT(MDR_WWDG_BASE)
    #define RST_CLK_PCLK_IWDG           PCLK_BIT(MDR_IWDG_BASE)
    #define RST_CLK_PCLK_TIMER1         PCLK_BIT(MDR_TIMER1_BASE)
    #define RST_CLK_PCLK_TIMER2         PCLK_BIT(MDR_TIMER2_BASE)
    #define RST_CLK_PCLK_TIMER3         PCLK_BIT(MDR_TIMER3_BASE)
    #define RST_CLK_PCLK_ADC            PCLK_BIT(MDR_ADC_BASE)
    #define RST_CLK_PCLK_DAC            PCLK_BIT(MDR_DAC_BASE)
    #define RST_CLK_PCLK_COMP           PCLK_BIT(MDR_COMP_BASE)
    #define RST_CLK_PCLK_SSP2           PCLK_BIT(MDR_SSP2_BASE)
    #define RST_CLK_PCLK_PORTA          PCLK_BIT(MDR_PORTA_BASE)
    #define RST_CLK_PCLK_PORTB          PCLK_BIT(MDR_PORTB_BASE)
    #define RST_CLK_PCLK_PORTC          PCLK_BIT(MDR_PORTC_BASE)
    #define RST_CLK_PCLK_PORTD          PCLK_BIT(MDR_PORTD_BASE)
    #define RST_CLK_PCLK_PORTE          PCLK_BIT(MDR_PORTE_BASE)
    #define RST_CLK_PCLK_UART3          PCLK_BIT(MDR_UART3_BASE)
    #define RST_CLK_PCLK_BKP            PCLK_BIT(MDR_BKP_BASE)
    #define RST_CLK_PCLK_AUDIO_IP       ((uint32_t)0x10000000)
    #define RST_CLK_PCLK_PORTF          PCLK_BIT(MDR_PORTF_BASE)
    #define RST_CLK_PCLK_EBC            PCLK_BIT(MDR_EBC_BASE)
    #define RST_CLK_PCLK_31             PCLK_BIT(0x400F8000)


    #define IS_RST_CLK_PCLK(PCLK)       (((PCLK) & RST_CLK_PCLK_31) == 0x00)
#endif /* #if defined (USE_MDR32FG16S1QI) */


#if defined (USE_K1986VE1xI)
    #define RST_CLK_PCLK_CAN1           PCLK_BIT(MDR_CAN1_BASE)
    #define RST_CLK_PCLK_CAN2           PCLK_BIT(MDR_CAN2_BASE)
    #define RST_CLK_PCLK_USB            PCLK_BIT(MDR_USB_BASE)
    #define RST_CLK_PCLK_EEPROM         PCLK_BIT(MDR_EEPROM_BASE)
    #define RST_CLK_PCLK_RST_CLK        PCLK_BIT(MDR_RST_CLK_BASE)
    #define RST_CLK_PCLK_DMA            PCLK_BIT(MDR_DMA_BASE)
    #define RST_CLK_PCLK_UART1          PCLK_BIT(MDR_UART1_BASE)
    #define RST_CLK_PCLK_UART2          PCLK_BIT(MDR_UART2_BASE)
    #define RST_CLK_PCLK_SSP1           PCLK_BIT(MDR_SSP1_BASE)
    #define RST_CLK_PCLK_MIL_STD_15531  PCLK_BIT(MDR_MIL_STD_15531_BASE)
    #define RST_CLK_PCLK_MIL_STD_15532  PCLK_BIT(MDR_MIL_STD_15532_BASE)
    #define RST_CLK_PCLK_POWER          PCLK_BIT(MDR_POWER_BASE)
    #define RST_CLK_PCLK_WWDG           PCLK_BIT(MDR_WWDG_BASE)
    #define RST_CLK_PCLK_IWDG           PCLK_BIT(MDR_IWDG_BASE)
    #define RST_CLK_PCLK_TIMER1         PCLK_BIT(MDR_TIMER1_BASE)
    #define RST_CLK_PCLK_TIMER2         PCLK_BIT(MDR_TIMER2_BASE)
    #define RST_CLK_PCLK_TIMER3         PCLK_BIT(MDR_TIMER3_BASE)
    #define RST_CLK_PCLK_ADC            PCLK_BIT(MDR_ADC_BASE)
    #define RST_CLK_PCLK_DAC            PCLK_BIT(MDR_DAC_BASE)
    #define RST_CLK_PCLK_TIMER4         PCLK_BIT(MDR_TIMER4_BASE)
    #define RST_CLK_PCLK_SSP2           PCLK_BIT(MDR_SSP2_BASE)
    #define RST_CLK_PCLK_PORTA          PCLK_BIT(MDR_PORTA_BASE)
    #define RST_CLK_PCLK_PORTB          PCLK_BIT(MDR_PORTB_BASE)
    #define RST_CLK_PCLK_PORTC          PCLK_BIT(MDR_PORTC_BASE)
    #define RST_CLK_PCLK_PORTD          PCLK_BIT(MDR_PORTD_BASE)
    #define RST_CLK_PCLK_PORTE          PCLK_BIT(MDR_PORTE_BASE)
    #define RST_CLK_PCLK_ARINC429R      PCLK_BIT(MDR_ARINC429R_BASE)
    #define RST_CLK_PCLK_BKP            PCLK_BIT(MDR_BKP_BASE)
    #define RST_CLK_PCLK_ARINC429T      PCLK_BIT(MDR_ARINC429T_BASE)
    #define RST_CLK_PCLK_PORTF          PCLK_BIT(MDR_PORTF_BASE)
    #define RST_CLK_PCLK_EBC            PCLK_BIT(MDR_EBC_BASE)
    #define RST_CLK_PCLK_SSP3           PCLK_BIT(MDR_SSP3_BASE)


    #define IS_RST_CLK_PCLK(PCLK)       (((PCLK) & ~0xFFFFFFFF) == 0)
#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group CLK_peripheral */

#define IS_RCC_CLK_HSI_TRIM_VALUE(TRIM)       ((TRIM) <= 0x3F)
#define IS_RCC_CLK_LSI_TRIM_VALUE(TRIM)       ((TRIM) <= 0x1F)

/** @} */ /* End of group RST_CLK_Exported_Constants */


/** @defgroup RST_CLK_Exported_Functions RST_CLK Exported Functions
  * @{
  */

void RST_CLK_DeInit(void);
void RST_CLK_WarmDeInit(void);

void RST_CLK_HSEconfig(RST_CLK_HSE_Mode RST_CLK_HSE);
ErrorStatus RST_CLK_HSEstatus(void);
#if defined (USE_K1986VE1xI)
    void RST_CLK_HSE2config(RST_CLK_HSE2_Mode RST_CLK_HSE2);
    ErrorStatus RST_CLK_HSE2status(void);
#endif /* #if defined (USE_K1986VE1xI) */

void RST_CLK_LSEconfig(RST_CLK_LSE_Mode RST_CLK_LSE);
ErrorStatus RST_CLK_LSEstatus(void);

void RST_CLK_HSIcmd(FunctionalState NewState);
void RST_CLK_HSIadjust(uint32_t HSItrimValue);
ErrorStatus RST_CLK_HSIstatus(void);

void RST_CLK_LSIcmd(FunctionalState NewState);
void RST_CLK_LSIadjust(uint32_t LSItrimValue);
ErrorStatus RST_CLK_LSIstatus(void);

void RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLL_Source RST_CLK_CPU_PLLsource, uint32_t RST_CLK_CPU_PLLmul);
void RST_CLK_CPU_PLLuse(FunctionalState UsePLL);
void RST_CLK_CPU_PLLcmd(FunctionalState NewState);
ErrorStatus RST_CLK_CPU_PLLstatus(void);

void RST_CLK_CPUclkPrescaler(RST_CLK_CPU_C3_Divisor CPUclkDivValue);
void RST_CLK_CPUclkSelection(RST_CLK_HCLK_Source CPU_CLK);

void RST_CLK_USB_PLLconfig(RST_CLK_USB_PLL_Source RST_CLK_USB_PLLsource, uint32_t RST_CLK_USB_PLLmul);
void RST_CLK_USB_PLLuse(FunctionalState UsePLL);
void RST_CLK_USB_PLLcmd(FunctionalState NewState);
ErrorStatus RST_CLK_USB_PLLstatus(void);

void RST_CLK_USBclkPrescaler(FunctionalState NewState);
void RST_CLK_USBclkEnable(FunctionalState NewState);

void RST_CLK_ADCclkSelection(RST_CLK_ADC_Source ADC_CLK);
void RST_CLK_ADCclkPrescaler(RST_CLK_ADC_C3_Divisor ADCclkDivValue);
void RST_CLK_ADCclkEnable(FunctionalState NewState);

void RST_CLK_HSIclkPrescaler(RST_CLK_HSI_C1_Divisor HSIclkDivValue);
void RST_CLK_RTC_HSIclkEnable(FunctionalState NewState);

void RST_CLK_HSEclkPrescaler(RST_CLK_HSE_C1_Divisor HSEclkDivValue);
void RST_CLK_RTC_HSEclkEnable(FunctionalState NewState);

void RST_CLK_CPUclkSelectionC1(RST_CLK_CPU_C1_Source CPU_CLK);

void RST_CLK_PCLKcmd(uint32_t RST_CLK_PCLK, FunctionalState NewState);

#if defined (USE_MDR32FG16S1QI)
    void RST_CLK_AUCclkDeInit(void);
    void RST_CLK_AUCclkSelection(RST_CLK_AUC_Source AUC_CLK);
    void RST_CLK_AUCclkPrescaler(RST_CLK_AUC_C3_Divisor AUCclkDIVValue);
    void RST_CLK_AUCclkCMD(FunctionalState NewState);
#endif /* #if defined (USE_MDR32FG16S1QI) */

void RST_CLK_GetClocksFreq(RST_CLK_FreqTypeDef* RST_CLK_Clocks);

FlagStatus RST_CLK_GetFlagStatus(RST_CLK_Flags RST_CLK_FLAG);

#if defined (USE_MDR32FG16S1QI)
    void RST_CLK_DSP_Selection(RST_CLK_DSP_C1_Source DSP_CLK_Source);
    void RST_CLK_DSPPLLConfig(RST_CLK_DSP_PLL_Multiplier PLLMul);
    void RST_CLK_DSPPLL_CMD(FunctionalState NewState);
    ErrorStatus RST_CLK_DSP_PLLStatus(void);
    void RST_CLK_DSP_PLLUse(FunctionalState UsePLL);
    void RST_CLK_DSPPrescaler(RST_CLK_DSP_C3_Divisor DSP_Prescaler);
    void RST_CLK_DSPCmd(FunctionalState NewState);
#endif /* #if defined (USE_MDR32FG16S1QI) */

#if defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI)
FlagStatus RST_CLK_DMADone_GetFlagStatus(uint32_t Channel);
uint32_t   RST_CLK_DMADone_GetStatus(void);
void       RST_CLK_DMADone_ClearFlags(uint32_t Channels);
#endif /* #if defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI) */

/** @} */ /* End of group RST_CLK_Exported_Functions */

/** @} */ /* End of group RST_CLK */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_RST_CLK_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_rst_clk.h */


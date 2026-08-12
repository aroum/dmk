/**
  ******************************************************************************
  * @file    MDR32FxQI_bkp.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/07/2024
  * @brief   This file contains all the functions prototypes for the BKP
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
#ifndef __MDR32FxQI_BKP_H
#define __MDR32FxQI_BKP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"


/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup BKP
  * @{
  */

/** @defgroup BKP_Exported_Types BKP Exported Types
  * @{
  */

/**
  * @brief Data Backup Registers
  */
typedef enum
{
    BKP_REG00 = ((uint32_t)0x0000),
    BKP_REG01 = ((uint32_t)0x0004),
    BKP_REG02 = ((uint32_t)0x0008),
    BKP_REG03 = ((uint32_t)0x000C),
    BKP_REG04 = ((uint32_t)0x0010),
    BKP_REG05 = ((uint32_t)0x0014),
    BKP_REG06 = ((uint32_t)0x0018),
    BKP_REG07 = ((uint32_t)0x001C),
    BKP_REG08 = ((uint32_t)0x0020),
    BKP_REG09 = ((uint32_t)0x0024),
    BKP_REG0A = ((uint32_t)0x0028),
    BKP_REG0B = ((uint32_t)0x002C),
    BKP_REG0C = ((uint32_t)0x0030),
    BKP_REG0D = ((uint32_t)0x0034),
    BKP_REG0E = ((uint32_t)0x0038),
    BKP_REG0F = ((uint32_t)0x003C)
} Data_Backup_Registers;

#define IS_BKP_REG(REG)     (((REG) == BKP_REG00) || ((REG) == BKP_REG01) || \
                             ((REG) == BKP_REG02) || ((REG) == BKP_REG03) || \
                             ((REG) == BKP_REG04) || ((REG) == BKP_REG05) || \
                             ((REG) == BKP_REG06) || ((REG) == BKP_REG07) || \
                             ((REG) == BKP_REG08) || ((REG) == BKP_REG09) || \
                             ((REG) == BKP_REG0A) || ((REG) == BKP_REG0B) || \
                             ((REG) == BKP_REG0C) || ((REG) == BKP_REG0D) || \
                             ((REG) == BKP_REG0E) || ((REG) == BKP_REG0F))

#define IS_USER_BKP_REG(REG)    (((REG) == BKP_REG00) || ((REG) == BKP_REG01) || \
                                 ((REG) == BKP_REG02) || ((REG) == BKP_REG03) || \
                                 ((REG) == BKP_REG04) || ((REG) == BKP_REG05) || \
                                 ((REG) == BKP_REG06) || ((REG) == BKP_REG07) || \
                                 ((REG) == BKP_REG08) || ((REG) == BKP_REG09) || \
                                 ((REG) == BKP_REG0A) || ((REG) == BKP_REG0B) || \
                                 ((REG) == BKP_REG0C) || ((REG) == BKP_REG0D))

/**
  * @brief BKP RTC Clock source
  */
typedef enum
{
    BKP_RTC_LSIclk = ((uint32_t)0x0000), /*!< LSI oscillator as RTC clock source */
    BKP_RTC_LSEclk = ((uint32_t)0x0004), /*!< LSE oscillator as RTC clock source */
    BKP_RTC_HSIclk = ((uint32_t)0x0008), /*!< HSI oscillator as RTC clock source */
    BKP_RTC_HSEclk = ((uint32_t)0x000C)  /*!< HSE oscillator as RTC clock source */
} BKP_RTC_CLK;

#define IS_RTC_CLK_SOURCE(CLK) (((CLK) == BKP_RTC_LSIclk) || \
                                ((CLK) == BKP_RTC_LSEclk) || \
                                ((CLK) == BKP_RTC_HSIclk) || \
                                ((CLK) == BKP_RTC_HSEclk))

/**
  * @brief BKP RTC IT
  */
typedef enum
{
    BKP_RTC_IT_ALRF = ((uint32_t)BKP_RTC_CS_ALRF_IE), /*!< Alarm interrupt */
    BKP_RTC_IT_SECF = ((uint32_t)BKP_RTC_CS_SECF_IE), /*!< Second interrupt */
    BKP_RTC_IT_OWF  = ((uint32_t)BKP_RTC_CS_OWF_IE )  /*!< Overflow interrupt */
} BKP_RTC_IT;

#define IS_RTC_CLK_IT(IE) (((IE) & ~(BKP_RTC_IT_ALRF | BKP_RTC_IT_SECF | BKP_RTC_IT_OWF)) == 0)


/**
  * @brief BKP RTC Flags
  */
typedef enum
{
    BKP_RTC_FLAG_WEC  = ((uint32_t)BKP_RTC_CS_WEC ), /*!< RTC Write operation ended flag */
    BKP_RTC_FLAG_OWF  = ((uint32_t)BKP_RTC_CS_OWF ), /*!< Overflow flag */
    BKP_RTC_FLAG_ALRF = ((uint32_t)BKP_RTC_CS_ALRF), /*!< Alarm flag */
    BKP_RTC_FLAG_SECF = ((uint32_t)BKP_RTC_CS_SECF)  /*!< Second flag */
} BKP_RTC_FLAGS;

#define IS_BKP_RTC_GET_FLAG(F) (((F) == BKP_RTC_CS_WEC)  || \
                                ((F) == BKP_RTC_CS_OWF)  || \
                                ((F) == BKP_RTC_CS_ALRF) || \
                                ((F) == BKP_RTC_CS_SECF))

#define IS_BKP_RTC_CLEAR_FLAG(F) (((F) == BKP_RTC_CS_OWF)  || \
                                  ((F) == BKP_RTC_CS_ALRF) || \
                                  ((F) == BKP_RTC_CS_SECF))


#define BKP_DUCC_MODE(par) ((uint32_t)((par) | (par << BKP_REG_0E_SELECTRI_Pos)))
/**
  * @brief BKP DUcc Mode
  */
typedef enum
{
    BKP_DUcc_upto_10MHz  = BKP_DUCC_MODE(0), /*!< system clock is up to 10 MHz */
    BKP_DUcc_upto_200kHz = BKP_DUCC_MODE(1), /*!< system clock is up to 200 kHz */
    BKP_DUcc_upto_500kHz = BKP_DUCC_MODE(2), /*!< system clock is up to 500 kHz */
    BKP_DUcc_upto_1MHz   = BKP_DUCC_MODE(3), /*!< system clock is up to 1 MHz */
    BKP_DUcc_clk_off     = BKP_DUCC_MODE(4), /*!< all clocks are switched off */
    BKP_DUcc_upto_40MHz  = BKP_DUCC_MODE(5), /*!< system clock is up to 40 MHz */
    BKP_DUcc_upto_80MHz  = BKP_DUCC_MODE(6), /*!< system clock is up to 80 MHz */
    BKP_DUcc_over_80MHz  = BKP_DUCC_MODE(7)  /*!< system clock is over 80 MHz */
} BKP_DUcc_Mode;

#define IS_BKP_DUCC_MODE(MODE) (((MODE) == BKP_DUcc_upto_10MHz) || \
                                ((MODE) == BKP_DUcc_upto_200kHz)|| \
                                ((MODE) == BKP_DUcc_upto_500kHz)|| \
                                ((MODE) == BKP_DUcc_clk_off)    || \
                                ((MODE) == BKP_DUcc_upto_1MHz)  || \
                                ((MODE) == BKP_DUcc_upto_40MHz) || \
                                ((MODE) == BKP_DUcc_upto_80MHz) || \
                                ((MODE) == BKP_DUcc_over_80MHz))

/**
  * @brief BKP DUcc Trim
  */
typedef enum
{
    BKP_DUcc_plus_100mV  = ((uint32_t)(0x0000 << 8)), /*!< trim DUcc with + 100mV */
    BKP_DUcc_plus_060mV  = ((uint32_t)(0x0001 << 8)), /*!< trim DUcc with + 060mV */
    BKP_DUcc_plus_040mV  = ((uint32_t)(0x0002 << 8)), /*!< trim DUcc with + 040mV */
    BKP_DUcc_plus_010mV  = ((uint32_t)(0x0003 << 8)), /*!< trim DUcc with + 010mV */
    BKP_DUcc_minus_010mV = ((uint32_t)(0x0004 << 8)), /*!< trim DUcc with - 010mV */
    BKP_DUcc_minus_040mV = ((uint32_t)(0x0005 << 8)), /*!< trim DUcc with - 040mV */
    BKP_DUcc_minus_060mV = ((uint32_t)(0x0006 << 8)), /*!< trim DUcc with - 060mV */
    BKP_DUcc_minus_100mV = ((uint32_t)(0x0007 << 8))  /*!< trim DUcc with - 100mV */
} BKP_DUcc_Trim;

#define IS_BKP_DUCC_TRIM(TRIM) (((TRIM) == BKP_DUcc_plus_100mV) || \
                                ((TRIM) == BKP_DUcc_plus_060mV) || \
                                ((TRIM) == BKP_DUcc_plus_040mV) || \
                                ((TRIM) == BKP_DUcc_plus_010mV) || \
                                ((TRIM) == BKP_DUcc_minus_010mV)|| \
                                ((TRIM) == BKP_DUcc_minus_040mV)|| \
                                ((TRIM) == BKP_DUcc_minus_060mV)|| \
                                ((TRIM) == BKP_DUcc_minus_100mV))

#if defined (USE_K1986VE1xI)
/**
  * @brief BKP Trim
  */
typedef enum
{
    BKP_TRIM_1_8_V = (0 << BKP_REG_0E_TRIM_34_Pos),
    BKP_TRIM_1_6_V = (1 << BKP_REG_0E_TRIM_34_Pos),
    BKP_TRIM_1_4_V = (2 << BKP_REG_0E_TRIM_34_Pos)
} BKP_TRIM;

#define IS_BKP_TRIM(TRIM)   (((TRIM) == BKP_TRIM_1_4_V) ||\
                             ((TRIM) == BKP_TRIM_1_6_V) ||\
                             ((TRIM) == BKP_TRIM_1_8_V))
#endif // #if defined (USE_K1986VE1xI)

#if (defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI))
/**
  * @brief BKP Stop Entry Mode
  */
typedef enum
{
    BKP_STOPentry_WFE = ((uint32_t)0x00), /*!< enter STOP mode with WFE instruction */
    BKP_STOPentry_WFI = ((uint32_t)0x01)  /*!< enter STOP mode with WFI instruction */
} BKP_Stop_Entry_Mode;

#define IS_BKP_STOP_ENTRY(F) (((F) == BKP_STOPentry_WFI) || ((F) == BKP_STOPentry_WFE))

#endif // #if (defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI))

/** @} */ /* End of group BKP_Exported_Types */


/** @defgroup BKP_Exported_Constants BKP Exported Constants
  * @{
  */

/** @defgroup BKP_RTC_CALIBRATION BKP RTC CALIBRATION
  * @{
  */
#define IS_RTC_CALIBRATION(CAL) (((CAL) & ~0xFF) == 0)
/** @} */ /* End of group BKP_RTC_CALIBRATION */

/** @defgroup BKP_PRESCALER_VALUE BKP PRESCALER VALUE
  * @{
  */
#define IS_BKP_PRESCALER_VALUE(VAL) ((uint32_t)(VAL) <= 0x000FFFFF)
/** @} */ /* End of group BKP_PRESCALER_VALUE */

/** @} */ /* End of group BKP_Exported_Constants */


/** @defgroup BKP_Exported_Functions BKP Exported Functions
  * @{
  */
void BKP_DeInit(void);
#if (defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI))
    void BKP_JTAGA_CMD(FunctionalState NewState);
    void BKP_JTAGB_CMD(FunctionalState NewState);
#endif // #if (defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI))
void BKP_RTCclkSource(BKP_RTC_CLK RTC_CLK);
void BKP_RTC_WorkPermit(FunctionalState NewState);
void BKP_RTC_Calibration(uint32_t RTC_Calibration);
void BKP_RTC_Reset(FunctionalState NewState);
void BKP_RTC_ITConfig(uint32_t BKP_RTC_IT_Source, FunctionalState NewState);
uint32_t BKP_RTC_GetCounter(void);
void BKP_RTC_SetCounter(uint32_t CounterValue);
void BKP_RTC_SetAlarm(uint32_t AlarmValue);
uint32_t BKP_RTC_GetDivider(void);
void BKP_RTC_SetPrescaler(uint32_t PrescalerValue);
void BKP_RTC_WaitForUpdate(void);
FlagStatus BKP_RTC_GetFlagStatus(BKP_RTC_FLAGS RTC_FLAG);
void BKP_RTC_ClearFlagStatus(uint32_t BKP_RTC_FLAG);
void BKP_DUccMode(BKP_DUcc_Mode DUccMode);
void BKP_DUccTrim(BKP_DUcc_Trim DUccTrim);
void BKP_SetFlagPOR(void);
ErrorStatus BKP_FlagPORstatus(void);
#if (defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI))
    void BKP_EnterSTOPMode(FunctionalState BKP_Regulator_state, BKP_Stop_Entry_Mode BKP_STOPEntry);
#elif defined (USE_K1986VE1xI)
    void BKP_EnterSLEEPMode(void);
    void BKP_SetTrim(BKP_TRIM trim);
    void BKP_CurrentLimitProtection150mA(FunctionalState NewState);
    void BKP_EnterStandAloneMode(void);
#endif
void BKP_EnterSTANDBYMode(void);

/** @} */ /* End of group BKP_Exported_Functions */

/** @} */ /* End of group BKP */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_BKP_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_bkp.h */



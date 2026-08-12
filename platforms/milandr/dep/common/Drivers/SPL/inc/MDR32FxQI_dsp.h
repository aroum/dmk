/**
  ******************************************************************************
  * @file    MDR32FxQI_dsp.h
  * @author  Milandr Application Team
  * @version V2.0.2i
  * @date    10/03/2022
  * @brief   This file contains all the functions prototypes for the DSP
  *          firmware library for MCU MDR32FG16S1QI.
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
#ifndef __MDR32FxQI_DSP_H
#define __MDR32FxQI_DSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @addtogroup DSP
  * @{
  */

/** @defgroup DSP_Exported_Types DSP Exported Types
  * @{
  */

/**
  * @brief DSP Flags
  */
typedef enum
{
    DSP_FLAG_BIO   = RST_CLK_DSP_CONTROL_STATUS_BIO,
    DSP_FLAG_XF    = RST_CLK_DSP_CONTROL_STATUS_XF,
    DSP_FLAG_HOLDA = RST_CLK_DSP_CONTROL_STATUS_HOLDA,
    DSP_FLAG_BRTRD = RST_CLK_DSP_CONTROL_STATUS_RD_BUFF_EN
} DSP_Flags;

#define IS_DSP_FLAG(FLAG)   (((FLAG) == DSP_FLAG_BIO)   || \
                             ((FLAG) == DSP_FLAG_XF)    || \
                             ((FLAG) == DSP_FLAG_HOLDA) || \
                             ((FLAG) == DSP_FLAG_BRTRD))

/**
  * @brief DSP_CLK_PCLKx peripheral modules clock constants
  */
typedef enum
{
    DSP_CLK_PCLK_CRP = DSP_CORE_CLKCMD_CRP,
    DSP_CLK_PCLK_CDC = DSP_CORE_CLKCMD_CDC,
    DSP_CLK_PCLK_TMR = DSP_CORE_CLKCMD_TMR,
    DSP_CLK_PCLK_DMA = DSP_CORE_CLKCMD_DMA,
    DSP_CLK_PCLK_MC3 = DSP_CORE_CLKCMD_MC3,
    DSP_CLK_PCLK_PC3 = DSP_CORE_CLKCMD_PC3,
    DSP_CLK_PCLK_MC2 = DSP_CORE_CLKCMD_MC2,
    DSP_CLK_PCLK_PC2 = DSP_CORE_CLKCMD_PC2,
    DSP_CLK_PCLK_MC1 = DSP_CORE_CLKCMD_MC1,
    DSP_CLK_PCLK_PC1 = DSP_CORE_CLKCMD_PC1,
    DSP_CLK_PCLK_CDM = DSP_CORE_CLKCMD_CDM,
    DSP_CLK_PCLK_CPM = DSP_CORE_CLKCMD_CPM,
    DSP_CLK_PCLK_CPU = DSP_CORE_CLKCMD_CPU
} DSP_CLK_PCLKx;

#define DSP_CLK_PCLK_Msk       ((DSP_CLK_PCLK_CRP)  | \
                                (DSP_CLK_PCLK_CDC)  | \
                                (DSP_CLK_PCLK_TMR)  | \
                                (DSP_CLK_PCLK_DMA)  | \
                                (DSP_CLK_PCLK_MC3)  | \
                                (DSP_CLK_PCLK_PC3)  | \
                                (DSP_CLK_PCLK_MC2)  | \
                                (DSP_CLK_PCLK_PC2)  | \
                                (DSP_CLK_PCLK_MC1)  | \
                                (DSP_CLK_PCLK_PC1)  | \
                                (DSP_CLK_PCLK_CDM)  | \
                                (DSP_CLK_PCLK_CPM)  | \
                                (DSP_CLK_PCLK_CPU))

#define IS_DSP_CLK_PCLK(PCLK)  (((PCLK) & (~DSP_CLK_PCLK_Msk)) == 0)

/**
  * @brief DSP_AIRQ
  */
typedef enum
{
    DSP_AIRQ0 = DSP_CORE_AIR_AIRQ0,
    DSP_AIRQ1 = DSP_CORE_AIR_AIRQ1,
    DSP_AIRQ2 = DSP_CORE_AIR_AIRQ2,
    DSP_AIRQ3 = DSP_CORE_AIR_AIRQ3,
    DSP_ANMI  = DSP_CORE_AIR_ANMI
} DSP_AIRQ;

#define DSP_AIRQ_Msk       ((DSP_AIRQ0) | (DSP_AIRQ1) | \
                            (DSP_AIRQ2) | (DSP_AIRQ3) | (DSP_ANMI))

#define IS_DSP_AIRQ(AIRQ)  (((AIRQ) & (~DSP_AIRQ_Msk)) == 0)

/**
  * @brief DSP_ADMA
  */
typedef enum
{
    DSP_ADMARQ0   = DSP_CORE_AIR_ADMARQ0,
    DSP_ADMARQ1   = DSP_CORE_AIR_ADMARQ1,
    DSP_ADMARQ2   = DSP_CORE_AIR_ADMARQ2,
    DSP_ADMARQ3   = DSP_CORE_AIR_ADMARQ3,
    DSP_ADMADONE0 = DSP_CORE_AIR_ADMADONE0,
    DSP_ADMADONE1 = DSP_CORE_AIR_ADMADONE1,
    DSP_ADMADONE2 = DSP_CORE_AIR_ADMADONE2,
    DSP_ADMADONE3 = DSP_CORE_AIR_ADMADONE3
} DSP_ADMA;

#define DSP_ADMA_Msk       ((DSP_ADMARQ0)   | (DSP_ADMARQ1)   | \
                            (DSP_ADMARQ2)   | (DSP_ADMARQ3)   | \
                            (DSP_ADMADONE0) | (DSP_ADMADONE1) | \
                            (DSP_ADMADONE2) | (DSP_ADMADONE3) )

#define IS_DSP_ADMA(ADMA)  (((ADMA) & (~DSP_ADMA_Msk)) == 0)

/**
  * @brief DSP_DIR
  */
typedef enum
{
    DSP_DIRQ0  = DSP_CORE_DIR_DIRQ0,
    DSP_DIRQ1  = DSP_CORE_DIR_DIRQ1,
    DSP_DIRQ2  = DSP_CORE_DIR_DIRQ2,
    DSP_DIRQ3  = DSP_CORE_DIR_DIRQ3,
    DSP_TIRQ   = DSP_CORE_DIR_TIR,
    DSP_RIRQ1  = DSP_CORE_DIR_RIRQ1,
    DSP_XIRQ1  = DSP_CORE_DIR_XIRQ1,
    DSP_RIRQ2  = DSP_CORE_DIR_RIRQ2,
    DSP_XIRQ2  = DSP_CORE_DIR_XIRQ2,
    DSP_RIRQ3  = DSP_CORE_DIR_RIRQ3,
    DSP_XIRQ3  = DSP_CORE_DIR_XIRQ3,
    DSP_CDIRQ  = DSP_CORE_DIR_CDIRQ,
    DSP_CRIRQ  = DSP_CORE_DIR_CRIRQ,
    DSP_DMAIRQ = DSP_CORE_DIR_DMAIRQ
} DSP_DIR;

#define DSP_DIR_Msk     ((DSP_DIRQ0) | (DSP_DIRQ1) | (DSP_DIRQ2) | \
                         (DSP_DIRQ3) | (DSP_TIRQ)  | (DSP_RIRQ1) | \
                         (DSP_XIRQ1) | (DSP_RIRQ2) | (DSP_XIRQ2) | \
                         (DSP_RIRQ3) | (DSP_XIRQ3) | (DSP_CDIRQ) | \
                         (DSP_CRIRQ) | (DSP_DMAIRQ))

#define IS_DSP_DIR(DIR) (((DIR) & (~DSP_DIR_Msk)) == 0)

/** @} */ /* End of group DSP_Exported_Types */


/** @defgroup DSP_Exported_Functions DSP Exported Functions
  * @{
  */

void DSP_DeInit(void);
void DSP_RST_Cmd(FunctionalState NewState);
void DSP_CPU_Cmd(FunctionalState NewState);
void DSP_MEM_Cmd(FunctionalState NewState);
void DSP_PER_Cmd(FunctionalState NewState);
void DSP_ReadBufferCmd(FunctionalState NewState);
FlagStatus DSP_GetStatus(DSP_Flags flag);
void DSP_CLKPCLKCmd(uint32_t DSP_CLK_PCLK, FunctionalState NewState);
void DSP_RISCToDSPInterruptCmd(uint16_t Interrupt, FlagStatus NewStatus);
void DSP_DMA_RISC_Cmd(uint16_t ADMA, FlagStatus NewStatus);
void DSP_DSPToRISCInterruptCmd(uint16_t Interrupt, FlagStatus NewStatus);

#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group DSP_Exported_Functions */

/** @} */ /* End of group DSP */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_DSP_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_dsp.h */



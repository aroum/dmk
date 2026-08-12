/**
  ******************************************************************************
  * @file    MDR32FxQI_timer.h
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    24/03/2024
  * @brief   This file contains all the functions prototypes for the TIMER
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
#ifndef __MDR32FxQI_TIMER_H
#define __MDR32FxQI_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup TIMER
  * @{
  */

/** @defgroup TIMER_Exported_Types TIMER Exported Types
  * @{
  */

/**
  * @brief TIMER Counter Mode
  */
typedef enum
{
    TIMER_CntMode_ClkFixedDir  = (((uint32_t)0x0) << TIMER_CNTRL_CNT_MODE_Pos), /*!< The Timer clock changes the TIMERx_CNT value. The count direction is not changed. */
    TIMER_CntMode_ClkChangeDir = (((uint32_t)0x1) << TIMER_CNTRL_CNT_MODE_Pos), /*!< The Timer clock changes the TIMERx_CNT value. The count direction changes. */
    TIMER_CntMode_EvtFixedDir  = (((uint32_t)0x2) << TIMER_CNTRL_CNT_MODE_Pos), /*!< The event changes the TIMERx_CNT value. The count direction is not changed. */
    TIMER_CntMode_EvtChangeDir = (((uint32_t)0x3) << TIMER_CNTRL_CNT_MODE_Pos)  /*!< The event changes the TIMERx_CNT value. The count direction changes. */
} TIMER_Counter_Mode_TypeDef;

#define IS_TIMER_COUNTER_MODE(MODE) (((MODE) == TIMER_CntMode_ClkFixedDir ) || \
                                     ((MODE) == TIMER_CntMode_ClkChangeDir) || \
                                     ((MODE) == TIMER_CntMode_EvtFixedDir ) || \
                                     ((MODE) == TIMER_CntMode_EvtChangeDir))

/**
  * @brief TIMER Counter Direction
  */
typedef enum
{
    TIMER_CntDir_Up = (((uint32_t)0x0) << TIMER_CNTRL_DIR_Pos), /*!< Increments the Timer TIMERx_CNT counter value. */
    TIMER_CntDir_Dn = (((uint32_t)0x1) << TIMER_CNTRL_DIR_Pos)  /*!< Decrements the Timer TIMERx_CNT counter value. */
} TIMER_Counter_Dir_TypeDef;

#define IS_TIMER_COUNTER_DIR(DIR) (((DIR) == TIMER_CntDir_Up) || \
                                   ((DIR) == TIMER_CntDir_Dn))

/**
  * @brief TIMER Event Source
  */
typedef enum
{
    TIMER_EvSrc_TIM_CLK = (((uint32_t)0x0) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects rising edge TIM_CLK event. */
    TIMER_EvSrc_TM1     = (((uint32_t)0x1) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects TIMER1 (CNT == ARR) event. */
    TIMER_EvSrc_TM2     = (((uint32_t)0x2) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects TIMER2 (CNT == ARR) event. */
    TIMER_EvSrc_TM3     = (((uint32_t)0x3) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects TIMER3 (CNT == ARR) event. */
    TIMER_EvSrc_CH1     = (((uint32_t)0x4) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects Channel 1 event. */
    TIMER_EvSrc_CH2     = (((uint32_t)0x5) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects Channel 2 event. */
    TIMER_EvSrc_CH3     = (((uint32_t)0x6) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects Channel 3 event. */
    TIMER_EvSrc_CH4     = (((uint32_t)0x7) << TIMER_CNTRL_EVENT_SEL_Pos), /*!< Selects Channel 4 event. */
    TIMER_EvSrc_ETR     = (((uint32_t)0x8) << TIMER_CNTRL_EVENT_SEL_Pos)  /*!< Selects ETR event. */
} TIMER_Event_Src_TypeDef;

#define IS_TIMER_EVENT_SOURCE(SOURCE) (((SOURCE) == TIMER_EvSrc_TIM_CLK) || \
                                       ((SOURCE) == TIMER_EvSrc_TM1 )    || \
                                       ((SOURCE) == TIMER_EvSrc_TM2 )    || \
                                       ((SOURCE) == TIMER_EvSrc_TM3 )    || \
                                       ((SOURCE) == TIMER_EvSrc_CH1 )    || \
                                       ((SOURCE) == TIMER_EvSrc_CH2 )    || \
                                       ((SOURCE) == TIMER_EvSrc_CH3 )    || \
                                       ((SOURCE) == TIMER_EvSrc_CH4 )    || \
                                       ((SOURCE) == TIMER_EvSrc_ETR ))

/**
  * @brief TIMER Filter Sampling Clock
  */
typedef enum
{
    TIMER_FDTS_TIMER_CLK_div_1 = (((uint32_t)0x0) << TIMER_CNTRL_FDTS_Pos), /*!< The FDTS clock occures each TIMER_CLK clock. */
    TIMER_FDTS_TIMER_CLK_div_2 = (((uint32_t)0x1) << TIMER_CNTRL_FDTS_Pos), /*!< The FDTS clock occures each 2-nd TIMER_CLK clock. */
    TIMER_FDTS_TIMER_CLK_div_3 = (((uint32_t)0x2) << TIMER_CNTRL_FDTS_Pos), /*!< The FDTS clock occures each 3-rd TIMER_CLK clock. */
    TIMER_FDTS_TIMER_CLK_div_4 = (((uint32_t)0x3) << TIMER_CNTRL_FDTS_Pos)  /*!< The FDTS clock occures each 4-th TIMER_CLK clock. */
} TIMER_Filter_Sampl_Clk_TypeDef;

#define IS_TIMER_FILTER_SAMPLING(CONFIG) (((CONFIG) == TIMER_FDTS_TIMER_CLK_div_1) || \
                                          ((CONFIG) == TIMER_FDTS_TIMER_CLK_div_2) || \
                                          ((CONFIG) == TIMER_FDTS_TIMER_CLK_div_3) || \
                                          ((CONFIG) == TIMER_FDTS_TIMER_CLK_div_4))

/**
  * @brief TIMER ARR Update Mode
  */
typedef enum
{
    TIMER_ARR_Update_Immediately     =  (((uint32_t)0x0) << TIMER_CNTRL_ARRB_EN_Pos), /*!< The ARR register is updated immediately. */
    TIMER_ARR_Update_On_CNT_Overflow =  (((uint32_t)0x1) << TIMER_CNTRL_ARRB_EN_Pos)  /*!< The ARR register is updated at CNT count end. */
} TIMER_ARR_Update_Mode_TypeDef;

#define IS_TIMER_ARR_UPDATE_MODE(MODE) (((MODE) == TIMER_ARR_Update_Immediately) || \
                                        ((MODE) == TIMER_ARR_Update_On_CNT_Overflow))

/**
  * @brief TIMER Filter configuration
  */
typedef enum
{
    TIMER_Filter_1FF_at_TIMER_CLK   = ((uint32_t)0x0), /*!< Signal is latched by 1 trigger  at TIMER_CLK clock frequency. */
    TIMER_Filter_2FF_at_TIMER_CLK   = ((uint32_t)0x1), /*!< Signal is latched by 2 triggers at TIMER_CLK clock frequency. */
    TIMER_Filter_4FF_at_TIMER_CLK   = ((uint32_t)0x2), /*!< Signal is latched by 4 triggers at TIMER_CLK clock frequency. */
    TIMER_Filter_8FF_at_TIMER_CLK   = ((uint32_t)0x3), /*!< Signal is latched by 8 triggers at TIMER_CLK clock frequency. */
    TIMER_Filter_6FF_at_FTDS_div_2  = ((uint32_t)0x4), /*!< Signal is latched by 6 triggers at FDTS/2  clock frequency. */
    TIMER_Filter_8FF_at_FTDS_div_2  = ((uint32_t)0x5), /*!< Signal is latched by 8 triggers at FDTS/2  clock frequency. */
    TIMER_Filter_6FF_at_FTDS_div_4  = ((uint32_t)0x6), /*!< Signal is latched by 6 triggers at FDTS/4  clock frequency. */
    TIMER_Filter_8FF_at_FTDS_div_4  = ((uint32_t)0x7), /*!< Signal is latched by 8 triggers at FDTS/4  clock frequency. */
    TIMER_Filter_6FF_at_FTDS_div_8  = ((uint32_t)0x8), /*!< Signal is latched by 6 triggers at FDTS/8  clock frequency. */
    TIMER_Filter_8FF_at_FTDS_div_8  = ((uint32_t)0x9), /*!< Signal is latched by 8 triggers at FDTS/8  clock frequency. */
    TIMER_Filter_5FF_at_FTDS_div_16 = ((uint32_t)0xA), /*!< Signal is latched by 5 triggers at FDTS/16 clock frequency. */
    TIMER_Filter_6FF_at_FTDS_div_16 = ((uint32_t)0xB), /*!< Signal is latched by 6 triggers at FDTS/16 clock frequency. */
    TIMER_Filter_8FF_at_FTDS_div_16 = ((uint32_t)0xC), /*!< Signal is latched by 8 triggers at FDTS/16 clock frequency. */
    TIMER_Filter_5FF_at_FTDS_div_32 = ((uint32_t)0xD), /*!< Signal is latched by 5 triggers at FDTS/32 clock frequency. */
    TIMER_Filter_6FF_at_FTDS_div_32 = ((uint32_t)0xE), /*!< Signal is latched by 6 triggers at FDTS/32 clock frequency. */
    TIMER_Filter_8FF_at_FTDS_div_32 = ((uint32_t)0xF)  /*!< Signal is latched by 8 triggers at FDTS/32 clock frequency. */
} TIMER_Filter_Config_TypeDef;

#define IS_TIMER_FILTER_CONF(CONF) (((CONF) == TIMER_Filter_1FF_at_TIMER_CLK  ) || \
                                    ((CONF) == TIMER_Filter_2FF_at_TIMER_CLK  ) || \
                                    ((CONF) == TIMER_Filter_4FF_at_TIMER_CLK  ) || \
                                    ((CONF) == TIMER_Filter_8FF_at_TIMER_CLK  ) || \
                                    ((CONF) == TIMER_Filter_6FF_at_FTDS_div_2 ) || \
                                    ((CONF) == TIMER_Filter_8FF_at_FTDS_div_2 ) || \
                                    ((CONF) == TIMER_Filter_6FF_at_FTDS_div_4 ) || \
                                    ((CONF) == TIMER_Filter_8FF_at_FTDS_div_4 ) || \
                                    ((CONF) == TIMER_Filter_6FF_at_FTDS_div_8 ) || \
                                    ((CONF) == TIMER_Filter_8FF_at_FTDS_div_8 ) || \
                                    ((CONF) == TIMER_Filter_5FF_at_FTDS_div_16) || \
                                    ((CONF) == TIMER_Filter_6FF_at_FTDS_div_16) || \
                                    ((CONF) == TIMER_Filter_8FF_at_FTDS_div_16) || \
                                    ((CONF) == TIMER_Filter_5FF_at_FTDS_div_32) || \
                                    ((CONF) == TIMER_Filter_6FF_at_FTDS_div_32) || \
                                    ((CONF) == TIMER_Filter_8FF_at_FTDS_div_32))

/**
  * @brief TIMER ETR Prescaler
  */
typedef enum
{
    TIMER_ETR_Prescaler_None  = (((uint32_t)0x0) << TIMER_BRKETR_CNTRL_ETR_PSC_Pos), /*!< No division of input frequency. */
    TIMER_ETR_Prescaler_div_2 = (((uint32_t)0x1) << TIMER_BRKETR_CNTRL_ETR_PSC_Pos), /*!< Selects input frequency divider by 2. */
    TIMER_ETR_Prescaler_div_4 = (((uint32_t)0x2) << TIMER_BRKETR_CNTRL_ETR_PSC_Pos), /*!< Selects input frequency divider by 4. */
    TIMER_ETR_Prescaler_div_8 = (((uint32_t)0x3) << TIMER_BRKETR_CNTRL_ETR_PSC_Pos)  /*!< Selects input frequency divider by 8. */
} TIMER_ETR_Prescaler_TypeDef;

#define IS_TIMER_ETR_PRESCALER(VALUE) (((VALUE) == TIMER_ETR_Prescaler_None ) || \
                                       ((VALUE) == TIMER_ETR_Prescaler_div_2) || \
                                       ((VALUE) == TIMER_ETR_Prescaler_div_4) || \
                                       ((VALUE) == TIMER_ETR_Prescaler_div_8))

/**
  * @brief TIMER ETR Polarity
  */
typedef enum
{
   TIMER_ETRPolarity_NonInverted = (((uint32_t)0x0) << TIMER_BRKETR_CNTRL_ETR_INV_Pos), /*!< No inversion of the ETR input. */
   TIMER_ETRPolarity_Inverted    = (((uint32_t)0x1) << TIMER_BRKETR_CNTRL_ETR_INV_Pos)  /*!< The ETR input is inverted. */
} TIMER_ETR_Polarity_TypeDef;

#define IS_TIMER_ETR_POLARITY(STATE) (((STATE) == TIMER_ETRPolarity_NonInverted) || \
                                      ((STATE) == TIMER_ETRPolarity_Inverted   ))

/**
  * @brief TIMER BRK Polarity
  */
typedef enum
{
    TIMER_BRKPolarity_NonInverted = (((uint32_t)0x0) << TIMER_BRKETR_CNTRL_BRK_INV_Pos), /*!< No inversion of the BRK input. */
    TIMER_BRKPolarity_Inverted    = (((uint32_t)0x1) << TIMER_BRKETR_CNTRL_BRK_INV_Pos)  /*!< The BRK input is inverted. */
} TIMER_BRK_Polarity_TypeDef;

#define IS_TIMER_BRK_POLARITY(STATE) (((STATE) == TIMER_BRKPolarity_NonInverted) || \
                                      ((STATE) == TIMER_BRKPolarity_Inverted   ))

/**
  * @brief TIMER Channel Number
  */
typedef enum
{
    TIMER_CHANNEL1 = ((uint32_t)0x0), /*!< Specifies the TIMER channel 1. */
    TIMER_CHANNEL2 = ((uint32_t)0x1), /*!< Specifies the TIMER channel 2. */
    TIMER_CHANNEL3 = ((uint32_t)0x2), /*!< Specifies the TIMER channel 3. */
    TIMER_CHANNEL4 = ((uint32_t)0x3)  /*!< Specifies the TIMER channel 4. */
} TIMER_Channel_Number_TypeDef;

#define IS_TIMER_CHANNEL_NUMBER(NUMBER) (((NUMBER) == TIMER_CHANNEL1) || \
                                         ((NUMBER) == TIMER_CHANNEL2) || \
                                         ((NUMBER) == TIMER_CHANNEL3) || \
                                         ((NUMBER) == TIMER_CHANNEL4))

/**
  * @brief TIMER Channel Mode
  */
typedef enum
{
    TIMER_CH_MODE_PWM     = (((uint32_t)0x0) << TIMER_CH_CNTRL_CAP_NPWM_Pos), /*!< Specifies the PWM channel mode. */
    TIMER_CH_MODE_CAPTURE = (((uint32_t)0x1) << TIMER_CH_CNTRL_CAP_NPWM_Pos)  /*!< Specifies the Capture channel mode. */
} TIMER_CH_Mode_TypeDef;

#define IS_TIMER_CHANNEL_MODE(MODE) (((MODE) == TIMER_CH_MODE_PWM) || \
                                     ((MODE) == TIMER_CH_MODE_CAPTURE))

/**
  * @brief TIMER Channel ETR Reset configuration
  */
typedef enum
{
    TIMER_CH_ETR_RESET_Disable = (((uint32_t)0x0) << TIMER_CH_CNTRL_ETREN_Pos), /*!< Disables ETR Reset. */
    TIMER_CH_ETR_RESET_Enable  = (((uint32_t)0x1) << TIMER_CH_CNTRL_ETREN_Pos)  /*!< Enables ETR Reset. */
} TIMER_CH_ETR_RESET_TypeDef;

#define IS_TIMER_CHANNEL_ETR_RESET_CONFIG(CONFIG) (((CONFIG) == TIMER_CH_ETR_RESET_Disable) || \
                                                   ((CONFIG) == TIMER_CH_ETR_RESET_Enable ))

/**
  * @brief TIMER Channel BRK Reset configuration
  */
typedef enum
{
    TIMER_CH_BRK_RESET_Disable = (((uint32_t)0x0) << TIMER_CH_CNTRL_BRKEN_Pos), /*!< Disables BRK Reset. */
    TIMER_CH_BRK_RESET_Enable  = (((uint32_t)0x1) << TIMER_CH_CNTRL_BRKEN_Pos)  /*!< Enables BRK Reset. */
} TIMER_CH_BRK_RESET_TypeDef;

#define IS_TIMER_CHANNEL_BRK_RESET_CONFIG(CONFIG) (((CONFIG) == TIMER_CH_BRK_RESET_Disable) || \
                                                   ((CONFIG) == TIMER_CH_BRK_RESET_Enable ))

/**
  * @brief TIMER Channel REF Format
  */
typedef enum
{
    TIMER_CH_REF_Format0 = (((uint32_t)0x0) << TIMER_CH_CNTRL_OCCM_Pos), /*!< REF = 0. */
    TIMER_CH_REF_Format1 = (((uint32_t)0x1) << TIMER_CH_CNTRL_OCCM_Pos), /*!< REF = 1 if: (CNT == CCR) or ((CNT == CCR1) and CCR1_EN = 1),
                                                                              else REF = 0. */
    TIMER_CH_REF_Format2 = (((uint32_t)0x2) << TIMER_CH_CNTRL_OCCM_Pos), /*!< REF = 0 if: (CNT == CCR) or ((CNT == CCR1) and CCR1_EN = 1),
                                                                              else REF = 1. */
    TIMER_CH_REF_Format3 = (((uint32_t)0x3) << TIMER_CH_CNTRL_OCCM_Pos), /*!< Toggle REF state if: (CNT == CCR) or 
                                                                              ((CNT == CCR1) and CCR1_EN = 1). */
    TIMER_CH_REF_Format4 = (((uint32_t)0x4) << TIMER_CH_CNTRL_OCCM_Pos), /*!< REF = 0. */
    TIMER_CH_REF_Format5 = (((uint32_t)0x5) << TIMER_CH_CNTRL_OCCM_Pos), /*!< REF = 1. */
    TIMER_CH_REF_Format6 = (((uint32_t)0x6) << TIMER_CH_CNTRL_OCCM_Pos), /*!< If CCR1_EN = 0:
                                                                                 DIR = 0: REF = 1 if (CNT < CCR), else REF = 0,
                                                                                 DIR = 1: REF = 0 if (CNT > CCR), else REF = 1,
                                                                              else if CCR1_EN = 1:
                                                                                 DIR = 0: REF = 0 if (CCR ≤ CNT ≤ CCR1), else REF = 1,
                                                                                 DIR = 1: REF = 0 if (CCR < CNT < CCR1), else REF = 1.
                                                                              The condition (CCR < CCR1) must be observed. */
    TIMER_CH_REF_Format7 = (((uint32_t)0x7) << TIMER_CH_CNTRL_OCCM_Pos)  /*!<  If CCR1_EN = 0:
                                                                                 DIR = 0: REF = 0 if (CNT < CCR), else REF = 1,
                                                                                 DIR = 1: REF = 1 if (CNT > CCR), else REF = 0,
                                                                              else if CCR1_EN = 1:
                                                                                 DIR = 0: REF = 1 if (CCR ≤ CNT ≤ CCR1), else REF = 0,
                                                                                 DIR = 1: REF = 1 if (CCR < CNT < CCR1), else REF = 0.
                                                                              The condition (CCR < CCR1) must be observed. */
} TIMER_CH_REF_Format_TypeDef;

#define IS_TIMER_CHANNEL_REF_FORMAT(FORMAT) (((FORMAT) == TIMER_CH_REF_Format0) || \
                                             ((FORMAT) == TIMER_CH_REF_Format1) || \
                                             ((FORMAT) == TIMER_CH_REF_Format2) || \
                                             ((FORMAT) == TIMER_CH_REF_Format3) || \
                                             ((FORMAT) == TIMER_CH_REF_Format4) || \
                                             ((FORMAT) == TIMER_CH_REF_Format5) || \
                                             ((FORMAT) == TIMER_CH_REF_Format6) || \
                                             ((FORMAT) == TIMER_CH_REF_Format7))

/**
  * @brief TIMER Channel Prescaler
  */
typedef enum
{
    TIMER_CH_Prescaler_None  = ((uint32_t)0x0), /*!< No division of input frequency. */
    TIMER_CH_Prescaler_div_2 = ((uint32_t)0x1), /*!< Selects input frequency divider by 2. */
    TIMER_CH_Prescaler_div_4 = ((uint32_t)0x2), /*!< Selects input frequency divider by 4. */
    TIMER_CH_Prescaler_div_8 = ((uint32_t)0x3)  /*!< Selects input frequency divider by 8. */
} TIMER_CH_Prescaler_TypeDef;

#define IS_TIMER_CHANNEL_PRESCALER(VALUE) (((VALUE) == TIMER_CH_Prescaler_None ) || \
                                           ((VALUE) == TIMER_CH_Prescaler_div_2) || \
                                           ((VALUE) == TIMER_CH_Prescaler_div_4) || \
                                           ((VALUE) == TIMER_CH_Prescaler_div_8))

/**
  * @brief TIMER Channel Event Source
  */
typedef enum
{
    TIMER_CH_EvSrc_PE     = (((uint32_t)0x0) << TIMER_CH_CNTRL_CHSEL_Pos), /*!< Selects positive edge from current TIMER channel. */
    TIMER_CH_EvSrc_NE     = (((uint32_t)0x1) << TIMER_CH_CNTRL_CHSEL_Pos), /*!< Selects negative edge from current TIMER channel. */
    TIMER_CH_EvSrc_PE_OC1 = (((uint32_t)0x2) << TIMER_CH_CNTRL_CHSEL_Pos), /*!< Selects positive edge from other TIMER channel (variant 1). */
    TIMER_CH_EvSrc_PE_OC2 = (((uint32_t)0x3) << TIMER_CH_CNTRL_CHSEL_Pos)  /*!< Selects positive edge from other TIMER channel (variant 2). */
} TIMER_CH_Event_Src_TypeDef;

#define IS_TIMER_CHANNEL_EVENT_SOURCE(SOURCE) (((SOURCE) == TIMER_CH_EvSrc_PE    ) || \
                                               ((SOURCE) == TIMER_CH_EvSrc_NE    ) || \
                                               ((SOURCE) == TIMER_CH_EvSrc_PE_OC1) || \
                                               ((SOURCE) == TIMER_CH_EvSrc_PE_OC2))

/**
  * @brief TIMER Channel CCR1 Event Source
  */
typedef enum
{
    TIMER_CH_CCR1EvSrc_PE     = (((uint32_t)0x0) << TIMER_CH_CNTRL2_CHSEL1_Pos), /*!< Selects positive edge from current TIMER channel. */
    TIMER_CH_CCR1EvSrc_NE     = (((uint32_t)0x1) << TIMER_CH_CNTRL2_CHSEL1_Pos), /*!< Selects negative edge from current TIMER channel. */
    TIMER_CH_CCR1EvSrc_NE_OC1 = (((uint32_t)0x2) << TIMER_CH_CNTRL2_CHSEL1_Pos), /*!< Selects negative edge from other TIMER channel (variant 1). */
    TIMER_CH_CCR1EvSrc_NE_OC2 = (((uint32_t)0x3) << TIMER_CH_CNTRL2_CHSEL1_Pos)  /*!< Selects negative edge from other TIMER channel (variant 2). */
} TIMER_CH_CCR1_Event_Src_TypeDef;

#define IS_TIMER_CHANNEL_CCR1_EVENT_SOURCE(SOURCE) (((SOURCE) == TIMER_CH_CCR1EvSrc_PE    ) || \
                                                    ((SOURCE) == TIMER_CH_CCR1EvSrc_NE    ) || \
                                                    ((SOURCE) == TIMER_CH_CCR1EvSrc_NE_OC1) || \
                                                    ((SOURCE) == TIMER_CH_CCR1EvSrc_NE_OC2))

/**
  * @brief TIMER CCR, CCR1 Update mode
  */
typedef enum
{
    TIMER_CH_CCR_Update_Immediately = (((uint32_t)0x0) << TIMER_CH_CNTRL2_CCRRLD_Pos), /*!< The CCR, CCR1 registers is updated immediately. */
    TIMER_CH_CCR_Update_On_CNT_eq_0 = (((uint32_t)0x1) << TIMER_CH_CNTRL2_CCRRLD_Pos)  /*!< The CCR, CCR1 registers is updated at (CNT == 0) condition. */
} TIMER_CH_CCR_Update_Mode_TypeDef;

#define IS_TIMER_CHANNEL_CCR_UPDATE_MODE(MODE) (((MODE) == TIMER_CH_CCR_Update_Immediately) || \
                                                ((MODE) == TIMER_CH_CCR_Update_On_CNT_eq_0))

/**
  * @brief TIMER Channel Output Polarity
  */
typedef enum
{
    TIMER_CHOPolarity_NonInverted = ((uint32_t)0x0), /*!< No inversion on the Channel output. */
    TIMER_CHOPolarity_Inverted    = ((uint32_t)0x1)  /*!< The Channel output is inverted. */
} TIMER_CH_OUT_Polarity_TypeDef;

#define IS_TIMER_CHO_POLARITY(STATE) (((STATE) == TIMER_CHOPolarity_NonInverted) || \
                                      ((STATE) == TIMER_CHOPolarity_Inverted   ))

/**
  * @brief TIMER Channel Output Source
  */
typedef enum
{
    TIMER_CH_OutSrc_Only_0 = ((uint32_t)0x0), /*!< Selects the '0' state on the CHx (or CHxN) line. */
    TIMER_CH_OutSrc_Only_1 = ((uint32_t)0x1), /*!< Selects the '1' state on the CHx (or CHxN) line. */
    TIMER_CH_OutSrc_REF    = ((uint32_t)0x2), /*!< Selects the REF state on the CHx line or the nREF state on the CHxN line. */
    TIMER_CH_OutSrc_DTG    = ((uint32_t)0x3)  /*!< Selects the DTG output state on the CHx (or CHxN) line. */
} TIMER_CH_OUT_Src_TypeDef;

#define IS_TIMER_CHO_SOURCE(SOURCE) (((SOURCE) == TIMER_CH_OutSrc_Only_0) || \
                                     ((SOURCE) == TIMER_CH_OutSrc_Only_1) || \
                                     ((SOURCE) == TIMER_CH_OutSrc_REF   ) || \
                                     ((SOURCE) == TIMER_CH_OutSrc_DTG   ))

/**
  * @brief TIMER Channel Output Mode
  */
typedef enum
{
    TIMER_CH_OutMode_Input     = ((uint32_t)0x0), /*!< Selects the Input mode on the CHx (or CHxN) line. */
    TIMER_CH_OutMode_Output    = ((uint32_t)0x1), /*!< Selects the Output mode on the CHx (or CHxN) line. */
    TIMER_CH_OutMode_REF_as_OE = ((uint32_t)0x2), /*!< The REF signal specifies the CHx line mode or the nREF signal specifies the CHxN line mode. */
    TIMER_CH_OutMode_DTG_as_OE = ((uint32_t)0x3)  /*!< The DTG output specifies the CHx (or CHxN) line mode. */
} TIMER_CH_OUT_Mode_TypeDef;

#define IS_TIMER_CHO_MODE(MODE) (((MODE) == TIMER_CH_OutMode_Input    ) || \
                                 ((MODE) == TIMER_CH_OutMode_Output   ) || \
                                 ((MODE) == TIMER_CH_OutMode_REF_as_OE) || \
                                 ((MODE) == TIMER_CH_OutMode_DTG_as_OE))

/**
  * @brief TIMER Channel DTG Clock Source
  */
typedef enum
{
    TIMER_CH_DTG_ClkSrc_TIMER_CLK = (((uint32_t)0x0) << TIMER_CH_DTG_EDTS_Pos), /*!< Specifies the TIMER_CLK signal as DTG clock source. */
    TIMER_CH_DTG_ClkSrc_FDTS      = (((uint32_t)0x1) << TIMER_CH_DTG_EDTS_Pos)  /*!< Specifies the FDTS signal as DTG clock source. */
} TIMER_CH_DTG_Clk_Src_TypeDef;

#define IS_TIMER_CHANNEL_DTG_CLK_SOURCE(SOURCE) (((SOURCE) == TIMER_CH_DTG_ClkSrc_TIMER_CLK) || \
                                                 ((SOURCE) == TIMER_CH_DTG_ClkSrc_FDTS     ))

/**
  * @brief TIMER Status flags
  */
typedef enum
{
    TIMER_STATUS_CNT_ZERO         = (((uint32_t)0x1) <<  0), /*!< The (CNT == 0) condition occurred. */
    TIMER_STATUS_CNT_ARR          = (((uint32_t)0x1) <<  1), /*!< The (CNT == ARR) condition occurred. */
    TIMER_STATUS_ETR_RISING_EDGE  = (((uint32_t)0x1) <<  2), /*!< The ETR rising edge occurred. */
    TIMER_STATUS_ETR_FALLING_EDGE = (((uint32_t)0x1) <<  3), /*!< The ETR falling edge occurred. */
    TIMER_STATUS_BRK              = (((uint32_t)0x1) <<  4), /*!< The (BRK == 1) condition occurred. */
    TIMER_STATUS_CCR_CAP_CH1      = (((uint32_t)0x1) <<  5), /*!< The Channel 1 CCR capture condition occurred. */
    TIMER_STATUS_CCR_CAP_CH2      = (((uint32_t)0x1) <<  6), /*!< The Channel 2 CCR capture condition occurred. */
    TIMER_STATUS_CCR_CAP_CH3      = (((uint32_t)0x1) <<  7), /*!< The Channel 3 CCR capture condition occurred. */
    TIMER_STATUS_CCR_CAP_CH4      = (((uint32_t)0x1) <<  8), /*!< The Channel 4 CCR capture condition occurred. */
    TIMER_STATUS_CCR_REF_CH1      = (((uint32_t)0x1) <<  9), /*!< The Channel 1 (REF == 1) condition occurred. */
    TIMER_STATUS_CCR_REF_CH2      = (((uint32_t)0x1) << 10), /*!< The Channel 2 (REF == 1) condition occurred. */
    TIMER_STATUS_CCR_REF_CH3      = (((uint32_t)0x1) << 11), /*!< The Channel 3 (REF == 1) condition occurred. */
    TIMER_STATUS_CCR_REF_CH4      = (((uint32_t)0x1) << 12), /*!< The Channel 4 (REF == 1) condition occurred. */
    TIMER_STATUS_CCR_CAP1_CH1     = (((uint32_t)0x1) << 13), /*!< The Channel 1 CCR1 capture condition occurred. */
    TIMER_STATUS_CCR_CAP1_CH2     = (((uint32_t)0x1) << 14), /*!< The Channel 2 CCR1 capture condition occurred. */
    TIMER_STATUS_CCR_CAP1_CH3     = (((uint32_t)0x1) << 15), /*!< The Channel 3 CCR1 capture condition occurred. */
    TIMER_STATUS_CCR_CAP1_CH4     = (((uint32_t)0x1) << 16)  /*!< The Channel 4 CCR1 capture condition occurred. */
} TIMER_Status_Flags_TypeDef;

#define TIMER_STATUS_Msk            (TIMER_STATUS_CNT_ZERO         | \
                                     TIMER_STATUS_CNT_ARR          | \
                                     TIMER_STATUS_ETR_RISING_EDGE  | \
                                     TIMER_STATUS_ETR_FALLING_EDGE | \
                                     TIMER_STATUS_BRK              | \
                                     TIMER_STATUS_CCR_CAP_CH1      | \
                                     TIMER_STATUS_CCR_CAP_CH2      | \
                                     TIMER_STATUS_CCR_CAP_CH3      | \
                                     TIMER_STATUS_CCR_CAP_CH4      | \
                                     TIMER_STATUS_CCR_REF_CH1      | \
                                     TIMER_STATUS_CCR_REF_CH2      | \
                                     TIMER_STATUS_CCR_REF_CH3      | \
                                     TIMER_STATUS_CCR_REF_CH4      | \
                                     TIMER_STATUS_CCR_CAP1_CH1     | \
                                     TIMER_STATUS_CCR_CAP1_CH2     | \
                                     TIMER_STATUS_CCR_CAP1_CH3     | \
                                     TIMER_STATUS_CCR_CAP1_CH4)

#define IS_TIMER_STATUS_FLAG(FLAG)  (((FLAG) == TIMER_STATUS_CNT_ZERO        ) || \
                                     ((FLAG) == TIMER_STATUS_CNT_ARR         ) || \
                                     ((FLAG) == TIMER_STATUS_ETR_RISING_EDGE ) || \
                                     ((FLAG) == TIMER_STATUS_ETR_FALLING_EDGE) || \
                                     ((FLAG) == TIMER_STATUS_BRK             ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP_CH1     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP_CH2     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP_CH3     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP_CH4     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_REF_CH1     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_REF_CH2     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_REF_CH3     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_REF_CH4     ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP1_CH1    ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP1_CH2    ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP1_CH3    ) || \
                                     ((FLAG) == TIMER_STATUS_CCR_CAP1_CH4    ))

#define IS_TIMER_STATUS(STATUS)    (((STATUS) & TIMER_STATUS_Msk) == (STATUS))

#if defined (USE_K1986VE1xI)
/**
  * @brief TIMER DMA Channels
  * @note Available only for MCU MDR32F1QI, K1986VE1xI
  */
typedef enum
{
    TIMER_DMA_Channel0 = ((uint32_t)0x04),
    TIMER_DMA_Channel1 = ((uint32_t)0x00),
    TIMER_DMA_Channel2 = ((uint32_t)0x01),
    TIMER_DMA_Channel3 = ((uint32_t)0x02),
    TIMER_DMA_Channel4 = ((uint32_t)0x03)
} TIMER_DMA_Channel_TypeDef;

#define IS_TIMER_DMA_Channel(DMA_Channel)   ((DMA_Channel == TIMER_DMA_Channel0) || \
                                             (DMA_Channel == TIMER_DMA_Channel1) || \
                                             (DMA_Channel == TIMER_DMA_Channel2) || \
                                             (DMA_Channel == TIMER_DMA_Channel3) || \
                                             (DMA_Channel == TIMER_DMA_Channel4))

#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief TIMER Clock BRG
  */
typedef enum
{
    TIMER_HCLKdiv1   = ((uint32_t)0x00),
    TIMER_HCLKdiv2   = ((uint32_t)0x01),
    TIMER_HCLKdiv4   = ((uint32_t)0x02),
    TIMER_HCLKdiv8   = ((uint32_t)0x03),
    TIMER_HCLKdiv16  = ((uint32_t)0x04),
    TIMER_HCLKdiv32  = ((uint32_t)0x05),
    TIMER_HCLKdiv64  = ((uint32_t)0x06),
    TIMER_HCLKdiv128 = ((uint32_t)0x07)
} TIMER_Clock_BRG_TypeDef;

#define IS_TIMER_CLOCK_BRG(BRG) (((BRG) == TIMER_HCLKdiv1  ) || \
                                 ((BRG) == TIMER_HCLKdiv2  ) || \
                                 ((BRG) == TIMER_HCLKdiv4  ) || \
                                 ((BRG) == TIMER_HCLKdiv8  ) || \
                                 ((BRG) == TIMER_HCLKdiv16 ) || \
                                 ((BRG) == TIMER_HCLKdiv32 ) || \
                                 ((BRG) == TIMER_HCLKdiv64 ) || \
                                 ((BRG) == TIMER_HCLKdiv128))


/**
  * @brief TIMER Counter Init structure definition
  */
typedef struct
{
#if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    uint16_t TIMER_IniCounter;       /*!< Specifies the initial counter value.
                                          This parameter can be a number between 0x0000 and 0xFFFF. */
    uint16_t TIMER_Prescaler;        /*!< Specifies the prescaler value used to divide the TIMER clock.
                                          This parameter can be a number between 0x0000 and 0xFFFF.
                                          CLK = TIMER_CLK/(TIMER_Prescaler + 1) */
    uint16_t TIMER_Period;           /*!< Specifies the period = ARR + 1 value to be loaded into the
                                          Auto-Reload Register (ARR) at the next update event.
                                          This parameter must be a number between 0x0000 and 0xFFFF.  */
#elif defined (USE_K1986VE1xI)
    uint32_t TIMER_IniCounter;       /*!< Specifies the initial counter value.
                                          This parameter can be a number between 0x00000000 and 0xFFFFFFFF. */
    uint32_t TIMER_Prescaler;        /*!< Specifies the prescaler value used to divide the TIMER clock.
                                          This parameter can be a number between 0x00000000 and 0xFFFFFFFF.
                                          CLK = TIMER_CLK/(TIMER_Prescaler + 1) */
    uint32_t TIMER_Period;           /*!< Specifies the period = ARR + 1 value to be loaded into the
                                          Auto-Reload Register (ARR) at the next update event.
                                          This parameter must be a number between 0x00000000 and 0xFFFFFFFF. */
#endif
    uint16_t TIMER_CounterMode;      /*!< Specifies the counter mode.
                                          This parameter can be a value of @ref TIMER_Counter_Mode_TypeDef */
    uint16_t TIMER_CounterDirection; /*!< Specifies the counter direction.
                                          This parameter can be a value of @ref TIMER_Counter_Dir_TypeDef */
    uint16_t TIMER_EventSource;      /*!< Specifies the Counter Event source.
                                          This parameter can be a value of @ref TIMER_Event_Src_TypeDef */
    uint16_t TIMER_FilterSampling;   /*!< Specifies the filter sampling clock (FDTS).
                                          This parameter can be a value of @ref TIMER_Filter_Sampl_Clk_TypeDef */
    uint16_t TIMER_ARR_UpdateMode;   /*!< Specifies the Auto-Reload Register (ARR) updating mode.
                                          This parameter can be a value of @ref TIMER_ARR_Update_Mode_TypeDef */
    uint16_t TIMER_ETR_FilterConf;   /*!< Specifies the ETR Filter configuration.
                                          This parameter can be a value of @ref TIMER_Filter_Config_TypeDef */
    uint16_t TIMER_ETR_Prescaler;    /*!< Specifies the ETR Prescaler configuration.
                                          This parameter can be a value of @ref TIMER_ETR_Prescaler_TypeDef */
    uint16_t TIMER_ETR_Polarity;     /*!< Specifies the ETR Polarity configuration.
                                          This parameter can be a value of @ref TIMER_ETR_Polarity_TypeDef */
    uint16_t TIMER_BRK_Polarity;     /*!< Specifies the BRK Polarity configuration.
                                          This parameter can be a value of @ref TIMER_BRK_Polarity_TypeDef */
} TIMER_CntInitTypeDef;

/**
  * @brief TIMER Channel Init structure definition
  */
typedef struct
{
    uint16_t TIMER_CH_Number;           /*!< Specifies the TIMER Channel number to be configured.
                                             This parameter can be a value of @ref TIMER_Channel_Number_TypeDef */
    uint16_t TIMER_CH_Mode;             /*!< Specifies the TIMER Channel mode.
                                             This parameter can be a value of @ref TIMER_CH_Mode_TypeDef */
    uint16_t TIMER_CH_ETR_Ena;          /*!< Enables or disables ETR.
                                             This parameter can be a value of @ref FunctionalState */
    uint16_t TIMER_CH_ETR_Reset;        /*!< Enables or disables ETR Reset.
                                             This parameter can be a value of @ref TIMER_CH_ETR_RESET_TypeDef */
    uint16_t TIMER_CH_BRK_Reset;        /*!< Enables or disables BRK Reset.
                                             This parameter can be a value of @ref TIMER_CH_BRK_RESET_TypeDef */
    uint16_t TIMER_CH_REF_Format;       /*!< Specifies the REF signal format.
                                             This parameter can be a value of @ref TIMER_CH_REF_Format_TypeDef */
    uint16_t TIMER_CH_Prescaler;        /*!< Specifies the TIMER Channel Prescaler configuration.
                                             This parameter can be a value of @ref TIMER_CH_Prescaler_TypeDef */
    uint16_t TIMER_CH_EventSource;      /*!< Specifies the Channel Event source.
                                             This parameter can be a value of @ref TIMER_CH_Event_Src_TypeDef */
    uint16_t TIMER_CH_FilterConf;       /*!< Specifies the TIMER Channel Filter configuration.
                                             This parameter can be a value of @ref TIMER_Filter_Config_TypeDef */
    uint16_t TIMER_CH_CCR_UpdateMode;   /*!< Specifies the TIMER CCR, CCR1 update mode.
                                             This parameter can be a value of @ref TIMER_CH_CCR_Update_Mode_TypeDef */
    uint16_t TIMER_CH_CCR1_Ena;         /*!< Enables or disables the CCR1 register.
                                             This parameter can be a value of @ref FunctionalState */
    uint16_t TIMER_CH_CCR1_EventSource; /*!< Specifies the Channel CCR1 Event source.
                                             This parameter can be a value of @ref TIMER_CH_CCR1_Event_Src_TypeDef */
#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
    uint16_t TIMER_CH_EventDelay;       /*!< Enables or disables synchronization between the capture event and the update of registers CCR and CCR1.
                                             This parameter can be a value of @ref FunctionalState */
#endif
} TIMER_ChnInitTypeDef;

/**
  * @brief TIMER Channel Output Init structure definition
  */
typedef struct
{
    uint16_t TIMER_CH_Number;            /*!< Specifies the TIMER Channel number to be configured.
                                              This parameter can be a value of @ref TIMER_Channel_Number_TypeDef */
    uint16_t TIMER_CH_DirOut_Polarity;   /*!< Specifies the TIMER CHx output polarity.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Polarity_TypeDef */
    uint16_t TIMER_CH_DirOut_Source;     /*!< Specifies the TIMER CHx output source.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Src_TypeDef */
    uint16_t TIMER_CH_DirOut_Mode;       /*!< Specifies the TIMER CHx output enable source.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Mode_TypeDef */
    uint16_t TIMER_CH_NegOut_Polarity;   /*!< Enables or disables the TIMER CHxN output inversion.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Polarity_TypeDef */
    uint16_t TIMER_CH_NegOut_Source;     /*!< Specifies the TIMER CHxN output source.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Src_TypeDef */
    uint16_t TIMER_CH_NegOut_Mode;       /*!< Specifies the TIMER CHxN output enable source.
                                              This parameter can be a value of @ref TIMER_CH_OUT_Mode_TypeDef */
    uint16_t TIMER_CH_DTG_MainPrescaler; /*!< Specifies the main prescaler of TIMER DTG.
                                              This parameter can be a number between 0x0000 and 0x00FF.
                                              Delay DTGdel = TIMER_CH_DTG_MainPrescaler*(TIMER_CH_DTG_AuxPrescaler + 1) clocks. */
    uint16_t TIMER_CH_DTG_AuxPrescaler;  /*!< Specifies the auxiliary prescaler of TIMER DTG.
                                              This parameter can be a number between 0x0000 and 0x000F.
                                              Delay DTGdel = TIMER_CH_DTG_MainPrescaler*(TIMER_CH_DTG_AuxPrescaler + 1) clocks. */
    uint16_t TIMER_CH_DTG_ClockSource;   /*!< Specifies the TIMER DTG clock source.
                                              This parameter can be a value of @ref TIMER_CH_DTG_Clk_Src_TypeDef */
} TIMER_ChnOutInitTypeDef;

/** @} */ /* End of group TIMER_Exported_Types */


/** @defgroup TIMER_Exported_Constants TIMER Exported Constants
  * @{
  */

/** @defgroup TIMER_CH_DTG_Prescaler TIMER Channel DTG Prescaler
  * @{
  */

#define IS_TIMER_CHANNEL_DTG_MAIN_PRESCALER(VALUE) (((VALUE) >> 8) == 0)

#define IS_TIMER_CHANNEL_DTG_AUX_PRESCALER(VALUE) (((VALUE) >> 4) == 0)

/** @} */ /* End of group TIMER_CH_DTG_Prescaler */

/** @} */ /* End of group TIMER_Exported_Constants */


/** @defgroup TIMER_Exported_Macros TIMER Exported Macros
  * @{
  */

/**
  * @brief Clears the TIMERx's interrupt pending (TIMERx's Status) bits.
  * @note  For more info see @ref TIMER_ClearFlag function
  */
#define TIMER_ClearITPendingBit(TIMERx, TIMER_IT) TIMER_ClearFlag(TIMERx, TIMER_IT)

/** @} */ /* End of group TIMER_Exported_Macros */


/** @defgroup TIMER_Exported_Functions TIMER Exported Functions
  * @{
  */

void TIMER_DeInit(MDR_TIMER_TypeDef* TIMERx);

void TIMER_CntInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_CntInitTypeDef* TIMER_CntInitStruct);
void TIMER_CntStructInit(TIMER_CntInitTypeDef* TIMER_CntInitStruct);

void TIMER_Cmd(MDR_TIMER_TypeDef* TIMERx, FunctionalState NewState);

#if defined (USE_K1986VE1xI)
    void TIMER_SetCounter(MDR_TIMER_TypeDef* TIMERx, uint32_t Counter);
    uint32_t TIMER_GetCounter(MDR_TIMER_TypeDef* TIMERx);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_SetCounter(MDR_TIMER_TypeDef* TIMERx, uint16_t Counter);
    uint16_t TIMER_GetCounter(MDR_TIMER_TypeDef* TIMERx);
#endif

#if defined (USE_K1986VE1xI)
    void TIMER_SetCntPrescaler(MDR_TIMER_TypeDef* TIMERx, uint32_t Prescaler);
    uint32_t TIMER_GetCntPrescaler(MDR_TIMER_TypeDef* TIMERx);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_SetCntPrescaler(MDR_TIMER_TypeDef* TIMERx, uint16_t Prescaler);
    uint16_t TIMER_GetCntPrescaler(MDR_TIMER_TypeDef* TIMERx);
#endif

#if defined (USE_K1986VE1xI)
    void TIMER_SetCntAutoreload(MDR_TIMER_TypeDef* TIMERx, uint32_t Autoreload);
    void TIMER_CntAutoreloadConfig(MDR_TIMER_TypeDef* TIMERx, uint32_t Autoreload, TIMER_ARR_Update_Mode_TypeDef UpdateMode);
    uint32_t TIMER_GetCntAutoreload(MDR_TIMER_TypeDef* TIMERx);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_SetCntAutoreload(MDR_TIMER_TypeDef* TIMERx, uint16_t Autoreload);
    void TIMER_CntAutoreloadConfig(MDR_TIMER_TypeDef* TIMERx, uint16_t Autoreload, TIMER_ARR_Update_Mode_TypeDef UpdateMode);
    uint16_t TIMER_GetCntAutoreload(MDR_TIMER_TypeDef* TIMERx);
#endif

void TIMER_CntEventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Event_Src_TypeDef EventSource);
void TIMER_FilterSamplingConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Filter_Sampl_Clk_TypeDef Prescaler);
void TIMER_CounterModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Counter_Mode_TypeDef Mode);
void TIMER_SetCounterDirection(MDR_TIMER_TypeDef* TIMERx, TIMER_Counter_Dir_TypeDef Direction);
void TIMER_ETRInputConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Prescaler_TypeDef Prescaler, TIMER_ETR_Polarity_TypeDef Polarity, TIMER_Filter_Config_TypeDef Filter);
void TIMER_ETRFilterConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Filter_Config_TypeDef Filter);
void TIMER_ETRPrescalerConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Prescaler_TypeDef Prescaler);
void TIMER_ETRPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Polarity_TypeDef Polarity);
void TIMER_BRKPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_BRK_Polarity_TypeDef Polarity);
TIMER_Counter_Dir_TypeDef TIMER_GetCounterDirection(MDR_TIMER_TypeDef* TIMERx);
FlagStatus TIMER_GetCntWriteComplete(MDR_TIMER_TypeDef* TIMERx);

void TIMER_ChnInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_ChnInitTypeDef* TIMER_ChnInitStruct);
void TIMER_ChnStructInit(TIMER_ChnInitTypeDef* TIMER_ChnInitStruct);

#if defined (USE_K1986VE1xI)
    void TIMER_SetChnCompare(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare);
    void TIMER_ChnCompareConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode);
    uint32_t TIMER_GetChnCapture(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_SetChnCompare(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare);
    void TIMER_ChnCompareConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode);
    uint16_t TIMER_GetChnCapture(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
#endif

#if defined (USE_K1986VE1xI)
    void TIMER_SetChnCompare1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare);
    void TIMER_ChnCompare1Config(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode);
    uint32_t TIMER_GetChnCapture1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_SetChnCompare1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare);
    void TIMER_ChnCompare1Config(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode);
    uint16_t TIMER_GetChnCapture1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
#endif

void TIMER_ChnETR_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState);
void TIMER_ChnETRResetConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_ETR_RESET_TypeDef NewState);
void TIMER_ChnBRKResetConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_BRK_RESET_TypeDef NewState);
void TIMER_ChnREFFormatConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_REF_Format_TypeDef Format);
void TIMER_ChnCapturePrescalerConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_Prescaler_TypeDef Prescaler);
void TIMER_ChnEventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_Event_Src_TypeDef EventSource);
#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
void TIMER_ChnEventDelay_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState);
#endif
void TIMER_ChnFilterConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_Filter_Config_TypeDef Filter);
FlagStatus TIMER_GetChnWriteComplete(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
FlagStatus TIMER_GetChnWriteComplete1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel);
void TIMER_ChnCCR1_EventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_CCR1_Event_Src_TypeDef EventSource);
void TIMER_ChnCCR1_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState);

void TIMER_ChnOutInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_ChnOutInitTypeDef* TIMER_ChnOutInitStruct);
void TIMER_ChnOutStructInit(TIMER_ChnOutInitTypeDef* TIMER_ChnOutInitStruct);
void TIMER_ChnOutConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource, TIMER_CH_OUT_Mode_TypeDef Mode, TIMER_CH_OUT_Polarity_TypeDef Polarity);
void TIMER_ChnOutSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource);
void TIMER_ChnOutModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Mode_TypeDef Mode);
void TIMER_ChnOutPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Polarity_TypeDef Polarity);
void TIMER_ChnNOutConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource, TIMER_CH_OUT_Mode_TypeDef Mode, TIMER_CH_OUT_Polarity_TypeDef Polarity);
void TIMER_ChnNOutSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource);
void TIMER_ChnNOutModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Mode_TypeDef Mode);
void TIMER_ChnNOutPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Polarity_TypeDef Polarity);
void TIMER_ChnOutDTGConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t MainPrescaler, uint32_t AuxPrescaler, TIMER_CH_DTG_Clk_Src_TypeDef ClockSource);

uint32_t TIMER_GetStatus(MDR_TIMER_TypeDef* TIMERx);
FlagStatus TIMER_GetFlagStatus(MDR_TIMER_TypeDef* TIMERx, TIMER_Status_Flags_TypeDef Flag);
void TIMER_ClearFlag(MDR_TIMER_TypeDef* TIMERx, uint32_t Flags);
#if defined (USE_K1986VE1xI)
    void TIMER_DMACmd(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_DMASource, TIMER_DMA_Channel_TypeDef TIMER_DMAChannel, FunctionalState NewState);
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    void TIMER_DMACmd(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_DMASource, FunctionalState NewState);
#endif

void TIMER_ITConfig(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_IT, FunctionalState NewState);
ITStatus TIMER_GetITStatus(MDR_TIMER_TypeDef* TIMERx, TIMER_Status_Flags_TypeDef TIMER_IT);

void TIMER_BRGInit(MDR_TIMER_TypeDef* TIMERx, TIMER_Clock_BRG_TypeDef TIMER_BRG);

/** @} */ /* End of group TIMER_Exported_Functions */

/** @} */ /* End of group TIMER */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_TIMER_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_timer.h */



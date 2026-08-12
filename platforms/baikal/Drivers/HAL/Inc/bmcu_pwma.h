/**
 * *****************************************************************************
 *  @file       bmcu_pwma.h
 *  @author     Baikal electronics SDK team
 *  @brief      PWMA module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - PWMA related constants, type definitions and enumerations
 *      - Macros for PWMA control and status check
 *      - Macros for PWMA interrupts control and status check
 *      - PWMA module export functions prototype
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMCU_PWMA_H
#define __BMCU_PWMA_H

/* Includes ------------------------------------------------------------------*/
#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
static const uint8_t OFFSET_TAB_CCMRx[] =
{
    0x00U,  /* 0: PWMAx_CH0  */
    0x00U,  /* 1: PWMAx_CH0N */
    0x00U,  /* 2: PWMAx_CH1  */
    0x00U,  /* 3: PWMAx_CH1N */
    0x04U,  /* 4: PWMAx_CH2  */
    0x04U,  /* 5: PWMAx_CH2N */
    0x04U   /* 6: PWMAx_CH3  */
};

static const uint8_t SHIFT_TAB_OCxx[] =
{
    0U,     /* 0: OC0M, OC0PE */
    0U,     /* 1: equivalent to 0 */
    8U,     /* 2: OC1M, OC1PE */
    8U,     /* 3: equivalent to 2 */
    0U,     /* 4: OC2M, OC2PE */
    0U,     /* 5: equivalent to 4 */
    8U      /* 6: OC3M, OC3PE */
};

static const uint8_t SHIFT_TAB_ICxx[] =
{
    0U,     /* 0: CC0S, IC0PSC, IC0F */
    0U,     /* 1: equivalent to 0 */
    8U,     /* 2: CC1S, IC1PSC, IC1F */
    8U,     /* 3: equivalent to 2 */
    0U,     /* 4: CC2S, IC2PSC, IC2F */
    0U,     /* 5: equivalent to 4 */
    8U      /* 6: CC3S, IC3PSC, IC3F */
};

static const uint8_t SHIFT_TAB_CCxP[] =
{
    0U,     /* 0: CC0P */
    2U,     /* 1: CC0NP */
    4U,     /* 2: CC1P */
    6U,     /* 3: CC1NP */
    8U,     /* 4: CC2P */
    10U,    /* 5: CC2NP */
    12U     /* 6: CC3P */
};

static const uint8_t SHIFT_TAB_OISx[] =
{
    0U,     /* 0: OIS0 */
    1U,     /* 1: OIS0N */
    2U,     /* 2: OIS1 */
    3U,     /* 3: OIS1N */
    4U,     /* 4: OIS2 */
    5U,     /* 5: OIS2N */
    6U      /* 6: NA */
};

/* Private macros ------------------------------------------------------------*/

/** 
 * @brief  Convert channel id into channel index.
 * @param  __CHANNEL__ This parameter can be one of the following values:
 *         @arg @ref PWMA_CH0
 *         @arg @ref PWMA_CH0N
 *         @arg @ref PWMA_CH1
 *         @arg @ref PWMA_CH1N
 *         @arg @ref PWMA_CH2
 *         @arg @ref PWMA_CH2N
 *         @arg @ref PWMA_CH3
 * @retval The channel index.
 */
#define PWMA_GET_CHANNEL_INDEX( __CHANNEL__)    (((__CHANNEL__) == PWMA_CH0) ? 0U : \
                                                 ((__CHANNEL__) == PWMA_CH0N) ? 1U : \
                                                 ((__CHANNEL__) == PWMA_CH1) ? 2U : \
                                                 ((__CHANNEL__) == PWMA_CH1N) ? 3U : \
                                                 ((__CHANNEL__) == PWMA_CH2) ? 4U : \
                                                 ((__CHANNEL__) == PWMA_CH2N) ? 5U : 6U)

/* Exported types ------------------------------------------------------------*/

/**
 * @typedef
 * @brief   Counter one pulse mode enumeration
 * @note    PWMA_ONEPULSEMODE_REPETITIVE: Counter is not stopped at update event
 *          PWMA_ONEPULSEMODE_SINGLE: Counter stops counting at the next update event
 * @see     PWMA_SetOnePulseMode(), PWMA_GetOnePulseMode()
 */
typedef enum {
    PWMA_ONEPULSEMODE_REPETITIVE = PWMA_CR1_OPM_REPETITIVE,
    PWMA_ONEPULSEMODE_SINGLE     = PWMA_CR1_OPM_SINGLE
} PWMA_OnePulseMode_TypeDef;

/**
 * @typedef
 * @brief   Counter mode enumeration
 * @note    PWMA_COUNTERMODE_UP: Counter used as upcounter
 *          PWMA_COUNTERMODE_DOWN: Counter used as downcounter
 *          PWMA_COUNTERMODE_CENTER_UP: The counter counts up and down alternatively. Output compare interrupt flags of output channels are set only when the counter is counting down.
 *          PWMA_COUNTERMODE_CENTER_DOWN: The counter counts up and down alternatively. Output compare interrupt flags of output channels are set only when the counter is counting up.
 *          PWMA_COUNTERMODE_CENTER_UP_DOWN: The counter counts up and down alternatively. Output compare interrupt flags of output channels are set only when the counter is counting up or down.
 * @see     PWMA_SetCounterMode(), PWMA_GetCounterMode()
 */
typedef enum {
    PWMA_COUNTERMODE_UP             = PWMA_CR1_DIR_UP,
    PWMA_COUNTERMODE_DOWN           = PWMA_CR1_DIR_DOWN,
    PWMA_COUNTERMODE_CENTER_UP      = PWMA_CR1_CMS_CENTER_ALIGNED_1,
    PWMA_COUNTERMODE_CENTER_DOWN    = PWMA_CR1_CMS_CENTER_ALIGNED_2,
    PWMA_COUNTERMODE_CENTER_UP_DOWN = PWMA_CR1_CMS_CENTER_ALIGNED_3
} PWMA_CounterMode_TypeDef;

/**
 * @typedef
 * @brief   Clock division enumeration
 *          This bit-field indicates the division ratio between the timer clock (CK_INT) frequency
 *          and the dead-time and sampling clock (tDTS)used by the dead-time generators
 *          and the digital filters (ETR, TIx)
 * @note    PWMA_CLOCKDIVISION_DIV1: tDTS=tCK_INT
 *          PWMA_CLOCKDIVISION_DIV2: tDTS=2*tCK_INT
 *          PWMA_CLOCKDIVISION_DIV4: tDTS=4*tCK_INT
 * @see     PWMA_EnableARRPreload(), PWMA_DisableARRPreload()
 */
typedef enum {
    PWMA_CLOCKDIVISION_DIV1 = PWMA_CR1_CKD_DIV1,
    PWMA_CLOCKDIVISION_DIV2 = PWMA_CR1_CKD_DIV2,
    PWMA_CLOCKDIVISION_DIV4 = PWMA_CR1_CKD_DIV4
} PWMA_ClockDivision_TypeDef;

/**
 * @typedef
 * @brief   Counter direction enumeration
 *          This bit is read only when the timer is configured in
 *          Center-aligned mode or Encoder mode
 * @note    PWMA_COUNTERDIRECTION_UP:   Counter used as upcounter
 *          PWMA_COUNTERDIRECTION_DOWN: Counter used as downcounter
 * @see     PWMA_GetDirection()
 */
typedef enum {
    PWMA_COUNTERDIRECTION_UP   = PWMA_CR1_DIR_UP,
    PWMA_COUNTERDIRECTION_DOWN = PWMA_CR1_DIR_DOWN
} PWMA_CounterDirection_TypeDef;

/**
 * @typedef
 * @brief   Capture/compare update source enumeration
 * @note    PWMA_CC_UPDATESOURCE_COMG_ONLY:     Capture/compare control bits are updated by setting the COMG bit only
 *          PWMA_CC_UPDATESOURCE_COMG_AND_TRGI: Capture/compare control bits are updated by setting the COMG bit or when a rising edge occurs on trigger input (TRGI)
 * @see     PWMA_CC_SetUpdate()
 */
typedef enum {
    PWMA_CC_UPDATESOURCE_COMG_ONLY     = PWMA_CR2_CCUS_COMG_ONLY,
    PWMA_CC_UPDATESOURCE_COMG_AND_TRGI = PWMA_CR2_CCUS_COMG_TRGI
} PWMA_CC_UpdateSource_TypeDef;

/**
 * @typedef
 * @brief   Trigger output enumeration
 * @note    PWMA_TRGO_RESET:   UG bit from the PWMAx_EGR register is used as trigger output
 *          PWMA_TRGO_ENABLE:  Counter Enable signal (CNT_EN) is used as trigger output
 *          PWMA_TRGO_UPDATE:  Update event is used as trigger output
 *          PWMA_TRGO_CC0IF:   CC1 capture or a compare match is used as trigger output
 *          PWMA_TRGO_OC0REF:  OC0REF signal is used as trigger output
 *          PWMA_TRGO_OC1REF:  OC1REF signal is used as trigger output
 *          PWMA_TRGO_OC2REF:  OC2REF signal is used as trigger output
 *          PWMA_TRGO_OC3REF:  OC3REF signal is used as trigger output
 * @see     PWMA_SetTriggerOutput()
 */
typedef enum {
    PWMA_TRGO_RESET  = PWMA_CR2_MMS_RESET,
    PWMA_TRGO_ENABLE = PWMA_CR2_MMS_ENABLE,
    PWMA_TRGO_UPDATE = PWMA_CR2_MMS_UPDATE,
    PWMA_TRGO_CC0IF  = PWMA_CR2_MMS_CC0IF,
    PWMA_TRGO_OC0REF = PWMA_CR2_MMS_OC0REF,
    PWMA_TRGO_OC1REF = PWMA_CR2_MMS_OC1REF,
    PWMA_TRGO_OC2REF = PWMA_CR2_MMS_OC2REF,
    PWMA_TRGO_OC3REF = PWMA_CR2_MMS_OC3REF
} PWMA_TRGO_TypeDef;

/**
 * @typedef
 * @brief   Output Compare State enumeration
 * @note    PWMA_OC_STATE_DISABLE :  OCx is not active
 *          PWMA_OC_STATE_ENABLE:    OCx signal is output on the corresponding output pin
 */
typedef enum {
    PWMA_OC_STATE_DISABLE = PWMA_CCER_CC0E_DISABLE,
    PWMA_OC_STATE_ENABLE  = PWMA_CCER_CC0E_ENABLE
} PWMA_OC_State_TypeDef;

/**
 * @typedef
 * @brief   Output Compare Mode enumeration
 * @note    PWMA_OC_MODE_FROZEN :  The comparison between the output compare register PWMAx_CCRy and the
 *                                counter PWMAx_CNT has no effect on the outputs.(this mode is used to generate a timing base)
 *          PWMA_OC_MODE_ACTIVE:   OCyREF is forced high on compare match
 *          PWMA_OC_MODE_INACTIVE: OCyREF is forced low on compare match
 *          PWMA_OC_MODE_TOGGLE:   OCyREF toggles on compare match
 *          PWMA_OC_MODE_FORCED_INACTIVE: OCyREF is forced low
 *          PWMA_OC_MODE_FORCED_ACTIVE: OCyREF is forced high
 *          PWMA_OC_MODE_PWM1: In upcounting, channel y is active as long as PWMAx_CNT<PWMAx_CCRy else inactive. In downcounting, channel y is inactive as long as PWMAx_CNT>PWMAx_CCRy else active.
 *          PWMA_OC_MODE_PWM2: In upcounting, channel y is inactive as long as PWMAx_CNT<PWMAx_CCRy else active. In downcounting, channel y is active as long as PWMAx_CNT>PWMAx_CCRy else inactive.
 * @see     PWMA_OC_SetMode()
 */
typedef enum {
    PWMA_OC_MODE_FROZEN          = PWMA_CCMR1_OC_OC0M_FROZEN,
    PWMA_OC_MODE_ACTIVE          = PWMA_CCMR1_OC_OC0M_ACTIVE,
    PWMA_OC_MODE_INACTIVE        = PWMA_CCMR1_OC_OC0M_INACTIVE,
    PWMA_OC_MODE_TOGGLE          = PWMA_CCMR1_OC_OC0M_TOGGLE,
    PWMA_OC_MODE_FORCED_INACTIVE = PWMA_CCMR1_OC_OC0M_FORCED_INACTIVE,
    PWMA_OC_MODE_FORCED_ACTIVE   = PWMA_CCMR1_OC_OC0M_FORCED_ACTIVE,
    PWMA_OC_MODE_PWM1            = PWMA_CCMR1_OC_OC0M_PWM1,
    PWMA_OC_MODE_PWM2            = PWMA_CCMR1_OC_OC0M_PWM2
} PWMA_OC_Mode_TypeDef;

/**
 * @typedef
 * @brief   Counter Output Compare Polarity enumeration
 * @note    PWMA_OC_POLARITY_HIGH :  OCx active high
 *          PWMA_OC_POLARITY_LOW:    OCx active low
 * @see     PWMA_OC_Config()
 */
typedef enum {
    PWMA_OC_POLARITY_HIGH = PWMA_CCER_CC0P_HIGH,
    PWMA_OC_POLARITY_LOW  = PWMA_CCER_CC0P_LOW
} PWMA_OC_Polarity_TypeDef;

/**
 * @typedef
 * @brief   Output Compare Idle State enumeration
 * @note    PWMA_OC_IDLESTATE_LOW :   OCx=0 (after a dead-time if OC is implemented) when MOE=0
 *          PWMA_OC_IDLESTATE_HIGH:   OCx=1 (after a dead-time if OC is implemented) when MOE=0
 * @see     PWMA_OC_Config()
 */
typedef enum {
    PWMA_OC_IDLESTATE_LOW  = PWMA_CR2_OIS0_IDLE_LOW,
    PWMA_OC_IDLESTATE_HIGH = PWMA_CR2_OIS0_IDLE_HIGH
} PWMA_OC_IdleState_TypeDef;

/**
 * @typedef
 * @brief   Active Input Selection enumeration
 * @note    PWMA_IC_ACTIVEINPUT_DIRECTTI :   ICx is mapped on TIx
 *          PWMA_IC_ACTIVEINPUT_INDIRECTTI:  ICx is mapped on TIy
 *          PWMA_IC_ACTIVEINPUT_TRC:         ICx is mapped on TRC
 * @see     PWMA_IC_Config()
 */
typedef enum {
    PWMA_IC_ACTIVEINPUT_DIRECTTI   = PWMA_CCMR1_CC0S_IC_TI0,
    PWMA_IC_ACTIVEINPUT_INDIRECTTI = PWMA_CCMR1_CC0S_IC_TI1,
    PWMA_IC_ACTIVEINPUT_TRC        = PWMA_CCMR1_CC0S_IC_TRC
} PWMA_IC_ActiveInput_TypeDef;

/**
 * @typedef
 * @brief   Input Capture Prescaler enumeration
 * @note    PWMA_IC_PRESCALER_DIV1 :  No prescaler, capture is done each time an edge is detected on the capture input
 *          PWMA_IC_PRESCALER_DIV2:   Capture is done once every 2 events
 *          PWMA_IC_PRESCALER_DIV4:   Capture is done once every 4 events
 *          PWMA_IC_PRESCALER_DIV8:   Capture is done once every 8 events
 * @see     PWMA_IC_Config()
 */
typedef enum {
    PWMA_IC_PRESCALER_DIV1 = PWMA_CCMR1_IC_IC0PSC_DIV1,
    PWMA_IC_PRESCALER_DIV2 = PWMA_CCMR1_IC_IC0PSC_DIV2,
    PWMA_IC_PRESCALER_DIV4 = PWMA_CCMR1_IC_IC0PSC_DIV4,
    PWMA_IC_PRESCALER_DIV8 = PWMA_CCMR1_IC_IC0PSC_DIV8
} PWMA_IC_Prescaler_TypeDef;

/**
 * @typedef
 * @brief   Input Capture Filter enumeration
 * @note    PWMA_IC_FILTER_FDIV1 :    No filter, sampling is done at fDTS
 *          PWMA_IC_FILTER_FDIV1_N2:  fSAMPLING=fCK_INT, N=2
 *          PWMA_IC_FILTER_FDIV1_N4:  fSAMPLING=fCK_INT, N=4
 *          PWMA_IC_FILTER_FDIV1_N8:  fSAMPLING=fCK_INT, N=8
 *          PWMA_IC_FILTER_FDIV2_N6:  fSAMPLING=fDTS/2, N=6
 *          PWMA_IC_FILTER_FDIV2_N8:  fSAMPLING=fDTS/2, N=8
 *          PWMA_IC_FILTER_FDIV4_N6:  fSAMPLING=fDTS/4, N=6
 *          PWMA_IC_FILTER_FDIV4_N8:  fSAMPLING=fDTS/4, N=8
 *          PWMA_IC_FILTER_FDIV8_N6:  fSAMPLING=fDTS/8, N=6
 *          PWMA_IC_FILTER_FDIV8_N8:  fSAMPLING=fDTS/8, N=8
 *          PWMA_IC_FILTER_FDIV16_N5: fSAMPLING=fDTS/16, N=5
 *          PWMA_IC_FILTER_FDIV16_N6: fSAMPLING=fDTS/16, N=6
 *          PWMA_IC_FILTER_FDIV16_N8: fSAMPLING=fDTS/16, N=8
 *          PWMA_IC_FILTER_FDIV32_N5: fSAMPLING=fDTS/32, N=5
 *          PWMA_IC_FILTER_FDIV32_N6: fSAMPLING=fDTS/32, N=6
 *          PWMA_IC_FILTER_FDIV32_N8: fSAMPLING=fDTS/32, N=8
 * @see     PWMA_IC_Config()
 */
typedef enum {
    PWMA_IC_FILTER_FDIV1     = PWMA_CCMR1_IC_IC0F_FDIV1,
    PWMA_IC_FILTER_FDIV1_N2  = PWMA_CCMR1_IC_IC0F_FDIV1_N2,
    PWMA_IC_FILTER_FDIV1_N4  = PWMA_CCMR1_IC_IC0F_FDIV1_N4,
    PWMA_IC_FILTER_FDIV1_N8  = PWMA_CCMR1_IC_IC0F_FDIV1_N8,
    PWMA_IC_FILTER_FDIV2_N6  = PWMA_CCMR1_IC_IC0F_FDIV2_N6,
    PWMA_IC_FILTER_FDIV2_N8  = PWMA_CCMR1_IC_IC0F_FDIV2_N8,
    PWMA_IC_FILTER_FDIV4_N6  = PWMA_CCMR1_IC_IC0F_FDIV4_N6,
    PWMA_IC_FILTER_FDIV4_N8  = PWMA_CCMR1_IC_IC0F_FDIV4_N8,
    PWMA_IC_FILTER_FDIV8_N6  = PWMA_CCMR1_IC_IC0F_FDIV8_N6,
    PWMA_IC_FILTER_FDIV8_N8  = PWMA_CCMR1_IC_IC0F_FDIV8_N8,
    PWMA_IC_FILTER_FDIV16_N5 = PWMA_CCMR1_IC_IC0F_FDIV16_N5,
    PWMA_IC_FILTER_FDIV16_N6 = PWMA_CCMR1_IC_IC0F_FDIV16_N6,
    PWMA_IC_FILTER_FDIV16_N8 = PWMA_CCMR1_IC_IC0F_FDIV16_N8,
    PWMA_IC_FILTER_FDIV32_N5 = PWMA_CCMR1_IC_IC0F_FDIV32_N5,
    PWMA_IC_FILTER_FDIV32_N6 = PWMA_CCMR1_IC_IC0F_FDIV32_N6,
    PWMA_IC_FILTER_FDIV32_N8 = PWMA_CCMR1_IC_IC0F_FDIV32_N8
} PWMA_IC_Filter_TypeDef;

/**
 * @typedef
 * @brief   Input Capture Polarity enumeration
 * @note    PWMA_IC_POLARITY_RISING :   Input signal is not inverted, capture on rising edge
 *          PWMA_IC_POLARITY_FALLING:   Input signal is inverted, capture on falling edge
 * @see     PWMA_IC_Config()
 */
typedef enum {
    PWMA_IC_POLARITY_RISING  = PWMA_CCER_CC0P_NONINVERTED,
    PWMA_IC_POLARITY_FALLING = PWMA_CCER_CC0P_INVERTED
} PWMA_IC_Polarity_TypeDef;

/**
 * @typedef
 * @brief   Clock Source enumeration
 * @note    PWMA_CLOCKSOURCE_INTERNAL : The timer is clocked by the internal clock provided from the RCC
 *          PWMA_CLOCKSOURCE_EXT_MODE1: Counter counts at each rising or falling edge on a selected input
 *          PWMA_CLOCKSOURCE_EXT_MODE2: Counter counts at each rising or falling edge on the external trigger input ETR
 * @see     PWMA_SetClockSource()
 */
typedef enum {
    PWMA_CLOCKSOURCE_INTERNAL  = PWMA_SMCR_SMS_DISABLE,
    PWMA_CLOCKSOURCE_EXT_MODE1 = PWMA_SMCR_SMS_EXTCLK_1,
    PWMA_CLOCKSOURCE_EXT_MODE2 = PWMA_SMCR_ECE
} PWMA_ClockSource_TypeDef;

/**
 * @typedef
 * @brief   Slave Mode enumeration
 * @note    PWMA_SLAVEMODE_DISABLED : Slave mode disabled
 *          PWMA_SLAVEMODE_RESET: Reset Mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter
 *          PWMA_SLAVEMODE_GATED: Gated Mode - The counter clock is enabled when the trigger input (TRGI) is high
 *          PWMA_SLAVEMODE_TRIGGER: Trigger Mode - The counter starts at a rising edge of the trigger TRGI
 * @see     PWMA_SetSlaveMode()
 */
typedef enum {
    PWMA_SLAVEMODE_DISABLED = PWMA_SMCR_SMS_DISABLE,
    PWMA_SLAVEMODE_RESET    = PWMA_SMCR_SMS_RESET,
    PWMA_SLAVEMODE_GATED    = PWMA_SMCR_SMS_GATED,
    PWMA_SLAVEMODE_TRIGGER  = PWMA_SMCR_SMS_TRIGGER
} PWMA_SlaveMode_TypeDef;

/**
 * @typedef
 * @brief   Trigger Selection enumeration
 * @note    PWMA_TS_ITR0:    Internal Trigger 0 (ITR0) is used as trigger input
 *          PWMA_TS_ITR1:    Internal Trigger 1 (ITR1) is used as trigger input
 *          PWMA_TS_ITR2:    Internal Trigger 2 (ITR2) is used as trigger input
 *          PWMA_TS_TI0F_ED: TI0 Edge Detector (TI0F_ED) is used as trigger input
 *          PWMA_TS_TI0FP0:  Filtered Timer Input 0 (TI0FP0) is used as trigger input
 *          PWMA_TS_TI1FP1:  Filtered Timer Input 1 (TI1FP1) is used as trigger input
 *          PWMA_TS_ETRF:    Filtered external Trigger (ETRF) is used as trigger input
 * @see     PWMA_SetTriggerInput()
 */
typedef enum {
    PWMA_TS_ITR0    = PWMA_SMCR_TS_ITR0,
    PWMA_TS_ITR1    = PWMA_SMCR_TS_ITR1,
    PWMA_TS_ITR2    = PWMA_SMCR_TS_ITR2,
    PWMA_TS_TI0F_ED = PWMA_SMCR_TS_TI0F_ED,
    PWMA_TS_TI0FP0  = PWMA_SMCR_TS_TI0FP0,
    PWMA_TS_TI1FP1  = PWMA_SMCR_TS_TI1FP1,
    PWMA_TS_ETRF    = PWMA_SMCR_TS_ETRF
} PWMA_TS_TypeDef;

/**
 * @typedef
 * @brief   External Trigger Polarity enumeration
 * @note    PWMA_ETR_POLARITY_NONINVERTED: ETR is non-inverted, active at high level or rising edge
 *          PWMA_ETR_POLARITY_INVERTED:    ETR is inverted, active at low level or falling edge
 * @see     PWMA_ConfigETR()
 */
typedef enum {
    PWMA_ETR_POLARITY_NONINVERTED = PWMA_SMCR_ETP_NONINVERTED,
    PWMA_ETR_POLARITY_INVERTED    = PWMA_SMCR_ETP_INVERTED
} PWMA_ETR_Polarity_TypeDef;

/**
 * @typedef
 * @brief   External Trigger Prescaler enumeration
 * @note    PWMA_ETR_PRESCALER_DIV1: ETR prescaler OFF
 *          PWMA_ETR_PRESCALER_DIV2: ETR frequency is divided by 2
 *          PWMA_ETR_PRESCALER_DIV4: ETR frequency is divided by 4
 *          PWMA_ETR_PRESCALER_DIV8: ETR frequency is divided by 8
 * @see     PWMA_ConfigETR()
 */
typedef enum {
    PWMA_ETR_PRESCALER_DIV1 = PWMA_SMCR_ETPS_DIV1,
    PWMA_ETR_PRESCALER_DIV2 = PWMA_SMCR_ETPS_DIV2,
    PWMA_ETR_PRESCALER_DIV4 = PWMA_SMCR_ETPS_DIV4,
    PWMA_ETR_PRESCALER_DIV8 = PWMA_SMCR_ETPS_DIV8
} PWMA_ETR_Prescaler_TypeDef;

/**
 * @typedef
 * @brief   External Trigger Filter enumeration
 * @note    PWMA_ETR_FILTER_FDIV1 :    No filter, sampling is done at fDTS
 *          PWMA_ETR_FILTER_FDIV1_N2:  fSAMPLING=fCK_INT, N=2
 *          PWMA_ETR_FILTER_FDIV1_N4:  fSAMPLING=fCK_INT, N=4
 *          PWMA_ETR_FILTER_FDIV1_N8:  fSAMPLING=fCK_INT, N=8
 *          PWMA_ETR_FILTER_FDIV2_N6:  fSAMPLING=fDTS/2, N=6
 *          PWMA_ETR_FILTER_FDIV2_N8:  fSAMPLING=fDTS/2, N=8
 *          PWMA_ETR_FILTER_FDIV4_N6:  fSAMPLING=fDTS/4, N=6
 *          PWMA_ETR_FILTER_FDIV4_N8:  fSAMPLING=fDTS/4, N=8
 *          PWMA_ETR_FILTER_FDIV8_N6:  fSAMPLING=fDTS/8, N=6
 *          PWMA_ETR_FILTER_FDIV8_N8:  fSAMPLING=fDTS/8, N=8
 *          PWMA_ETR_FILTER_FDIV16_N5: fSAMPLING=fDTS/16, N=5
 *          PWMA_ETR_FILTER_FDIV16_N6: fSAMPLING=fDTS/16, N=6
 *          PWMA_ETR_FILTER_FDIV16_N8: fSAMPLING=fDTS/16, N=8
 *          PWMA_ETR_FILTER_FDIV32_N5: fSAMPLING=fDTS/32, N=5
 *          PWMA_ETR_FILTER_FDIV32_N6: fSAMPLING=fDTS/32, N=6
 *          PWMA_ETR_FILTER_FDIV32_N8: fSAMPLING=fDTS/32, N=8
 * @see     PWMA_ConfigETR()
 */
typedef enum {
    PWMA_ETR_FILTER_FDIV1     = PWMA_SMCR_ETF_FDIV1,
    PWMA_ETR_FILTER_FDIV1_N2  = PWMA_SMCR_ETF_FDIV1_N2,
    PWMA_ETR_FILTER_FDIV1_N4  = PWMA_SMCR_ETF_FDIV1_N4,
    PWMA_ETR_FILTER_FDIV1_N8  = PWMA_SMCR_ETF_FDIV1_N8,
    PWMA_ETR_FILTER_FDIV2_N6  = PWMA_SMCR_ETF_FDIV2_N6,
    PWMA_ETR_FILTER_FDIV2_N8  = PWMA_SMCR_ETF_FDIV2_N8,
    PWMA_ETR_FILTER_FDIV4_N6  = PWMA_SMCR_ETF_FDIV4_N6,
    PWMA_ETR_FILTER_FDIV4_N8  = PWMA_SMCR_ETF_FDIV4_N8,
    PWMA_ETR_FILTER_FDIV8_N6  = PWMA_SMCR_ETF_FDIV8_N6,
    PWMA_ETR_FILTER_FDIV8_N8  = PWMA_SMCR_ETF_FDIV8_N8,
    PWMA_ETR_FILTER_FDIV16_N5 = PWMA_SMCR_ETF_FDIV16_N5,
    PWMA_ETR_FILTER_FDIV16_N6 = PWMA_SMCR_ETF_FDIV16_N6,
    PWMA_ETR_FILTER_FDIV16_N8 = PWMA_SMCR_ETF_FDIV16_N8,
    PWMA_ETR_FILTER_FDIV32_N5 = PWMA_SMCR_ETF_FDIV32_N5,
    PWMA_ETR_FILTER_FDIV32_N6 = PWMA_SMCR_ETF_FDIV32_N6,
    PWMA_ETR_FILTER_FDIV32_N8 = PWMA_SMCR_ETF_FDIV32_N8
} PWMA_ETR_Filter_TypeDef;

/**
 * @typedef
 * @brief   Break state enumeration
 * @note    PWMA_BRK_DISABLE:  Break function disabled
 *          PWMA_BRK_ENABLE:   Break function enabled
 */
typedef enum {
    PWMA_BRK_DISABLE = PWMA_BDTR_BKE_DISABLE,
    PWMA_BRK_ENABLE  = PWMA_BDTR_BKE_ENABLE
} PWMA_BRK_State_TypeDef;

/**
 * @typedef
 * @brief   Break polarity enumeration
 * @note    PWMA_BRK_POLARITY_LOW:  Break input (BRK) is active low
 *          PWMA_BRK_POLARITY_HIGH: Break input (BRK) is active high
 * @see     PWMA_SetBreakPolarity()
 */
typedef enum {
    PWMA_BRK_POLARITY_LOW  = PWMA_BDTR_BKP_LOW,
    PWMA_BRK_POLARITY_HIGH = PWMA_BDTR_BKP_HIGH
} PWMA_BRK_Polarity_TypeDef;

/**
 * @typedef
 * @brief   Off-state selection for Run mode enumeration
 * @note    PWMA_OSSR_DISABLE:  When inactive, OCx/OCxN outputs are disabled
 *          PWMA_OSSR_ENABLE:   When inactive, OC/OCN outputs are enabled with their inactive level as soon as CCxE=1 or CCxNE=1
 * @see     PWMA_SetOffStates()
 */
typedef enum {
    PWMA_OSSR_DISABLE = PWMA_BDTR_OSSR_DISABLE,
    PWMA_OSSR_ENABLE  = PWMA_BDTR_OSSR_ENABLE
} PWMA_OSSR_TypeDef;

/**
 * @typedef
 * @brief   Automatic output state enumeration
 * @note    PWMA_AUTOMATICOUTPUT_DISABLE:  MOE can be set only by software
 *          PWMA_AUTOMATICOUTPUT_ENABLE:   MOE can be set by software or automatically at the next update event
 */
typedef enum {
    PWMA_AUTOMATICOUTPUT_DISABLE = PWMA_BDTR_AOE_DISABLE,
    PWMA_AUTOMATICOUTPUT_ENABLE  = PWMA_BDTR_AOE_ENABLE
} PWMA_AutomaticOutput_TypeDef;

/**
 * @typedef
 * @brief   DMA Burst Base Address enumeration
 * @note    PWMA_DMABURST_BASEADDR_CR1:   PWMAx_CR1 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CR2:   PWMAx_CR2 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_SMCR:  PWMAx_SMCR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_DIER:  PWMAx_DIER register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_SR:    PWMAx_SR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_EGR:   PWMAx_EGR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCMR1: PWMAx_CCMR1 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCMR2: PWMAx_CCMR2 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCER:  PWMAx_CCER register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CNT:   PWMAx_CNT register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_PSC:   PWMAx_PSC register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_ARR:   PWMAx_ARR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_RCR:   PWMAx_RCR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCR0:  PWMAx_CCR0 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCR1:  PWMAx_CCR1 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCR2:  PWMAx_CCR2 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_CCR3:  PWMAx_CCR3 register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_BDTR:  PWMAx_BDTR register is the DMA base address for DMA burst
            PWMA_DMABURST_BASEADDR_DCR:   PWMAx_DCR register is the DMA base address for DMA burst
 * @see     PWMA_DMA_Config()
 */
typedef enum {
    PWMA_DMABURST_BASEADDR_CR1   = PWMA_DCR_DBA_CR1,
    PWMA_DMABURST_BASEADDR_CR2   = PWMA_DCR_DBA_CR2,
    PWMA_DMABURST_BASEADDR_SMCR  = PWMA_DCR_DBA_SMCR,
    PWMA_DMABURST_BASEADDR_DIER  = PWMA_DCR_DBA_DIER,
    PWMA_DMABURST_BASEADDR_SR    = PWMA_DCR_DBA_SR,
    PWMA_DMABURST_BASEADDR_EGR   = PWMA_DCR_DBA_EGR,
    PWMA_DMABURST_BASEADDR_CCMR1 = PWMA_DCR_DBA_CCMR1,
    PWMA_DMABURST_BASEADDR_CCMR2 = PWMA_DCR_DBA_CCMR2,
    PWMA_DMABURST_BASEADDR_CCER  = PWMA_DCR_DBA_CCER,
    PWMA_DMABURST_BASEADDR_CNT   = PWMA_DCR_DBA_CNT,
    PWMA_DMABURST_BASEADDR_PSC   = PWMA_DCR_DBA_PSC,
    PWMA_DMABURST_BASEADDR_ARR   = PWMA_DCR_DBA_ARR,
    PWMA_DMABURST_BASEADDR_RCR   = PWMA_DCR_DBA_RCR,
    PWMA_DMABURST_BASEADDR_CCR0  = PWMA_DCR_DBA_CCR0,
    PWMA_DMABURST_BASEADDR_CCR1  = PWMA_DCR_DBA_CCR1,
    PWMA_DMABURST_BASEADDR_CCR2  = PWMA_DCR_DBA_CCR2,
    PWMA_DMABURST_BASEADDR_CCR3  = PWMA_DCR_DBA_CCR3,
    PWMA_DMABURST_BASEADDR_BDTR  = PWMA_DCR_DBA_BDTR,
    PWMA_DMABURST_BASEADDR_DCR   = PWMA_DCR_DBA_DCR
} PWMA_DMABurst_BaseAddress_TypeDef;

/**
 * @typedef
 * @brief   DMA Burst Length enumeration
 * @note    PWMA_DMABURST_LENGTH_1TRANSFER:   Transfer is done to 1 register starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_2TRANSFER:   Transfer is done to 2 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_3TRANSFER:   Transfer is done to 3 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_4TRANSFER:   Transfer is done to 4 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_5TRANSFER:   Transfer is done to 5 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_6TRANSFER:   Transfer is done to 6 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_7TRANSFER:   Transfer is done to 7 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_8TRANSFER:   Transfer is done to 8 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_9TRANSFER:   Transfer is done to 9 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_10TRANSFER:  Transfer is done to 10 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_11TRANSFER:  Transfer is done to 11 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_12TRANSFER:  Transfer is done to 12 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_13TRANSFER:  Transfer is done to 13 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_14TRANSFER:  Transfer is done to 14 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_15TRANSFER:  Transfer is done to 15 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_16TRANSFER:  Transfer is done to 16 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_17TRANSFER:  Transfer is done to 17 registers starting from the DMA burst base address
            PWMA_DMABURST_LENGTH_18TRANSFER:  Transfer is done to 19 registers starting from the DMA burst base address
 * @see     PWMA_DMA_Config()
 */
typedef enum {
    PWMA_DMABURST_LENGTH_1TRANSFER   = PWMA_DCR_DBL_1TRANSFER,
    PWMA_DMABURST_LENGTH_2TRANSFERS  = PWMA_DCR_DBL_2TRANSFERS,
    PWMA_DMABURST_LENGTH_3TRANSFERS  = PWMA_DCR_DBL_3TRANSFERS,
    PWMA_DMABURST_LENGTH_4TRANSFERS  = PWMA_DCR_DBL_4TRANSFERS,
    PWMA_DMABURST_LENGTH_5TRANSFERS  = PWMA_DCR_DBL_5TRANSFERS,
    PWMA_DMABURST_LENGTH_6TRANSFERS  = PWMA_DCR_DBL_6TRANSFERS,
    PWMA_DMABURST_LENGTH_7TRANSFERS  = PWMA_DCR_DBL_7TRANSFERS,
    PWMA_DMABURST_LENGTH_8TRANSFERS  = PWMA_DCR_DBL_8TRANSFERS,
    PWMA_DMABURST_LENGTH_9TRANSFERS  = PWMA_DCR_DBL_9TRANSFERS,
    PWMA_DMABURST_LENGTH_10TRANSFERS = PWMA_DCR_DBL_10TRANSFERS,
    PWMA_DMABURST_LENGTH_11TRANSFERS = PWMA_DCR_DBL_11TRANSFERS,
    PWMA_DMABURST_LENGTH_12TRANSFERS = PWMA_DCR_DBL_12TRANSFERS,
    PWMA_DMABURST_LENGTH_13TRANSFERS = PWMA_DCR_DBL_13TRANSFERS,
    PWMA_DMABURST_LENGTH_14TRANSFERS = PWMA_DCR_DBL_14TRANSFERS,
    PWMA_DMABURST_LENGTH_15TRANSFERS = PWMA_DCR_DBL_15TRANSFERS,
    PWMA_DMABURST_LENGTH_16TRANSFERS = PWMA_DCR_DBL_16TRANSFERS,
    PWMA_DMABURST_LENGTH_17TRANSFERS = PWMA_DCR_DBL_17TRANSFERS,
    PWMA_DMABURST_LENGTH_18TRANSFERS = PWMA_DCR_DBL_18TRANSFERS
} PWMA_DMABurst_Length_TypeDef;

/** @brief Flags */
typedef enum {
    PWMA_FLAG_UPDATE = 0,   /*!< Update interrupt flag */
    PWMA_FLAG_CC0    = 1,   /*!< Capture/Compare 0 interrupt flag */
    PWMA_FLAG_CC1    = 2,   /*!< Capture/Compare 1 interrupt flag */
    PWMA_FLAG_CC2    = 3,   /*!< Capture/Compare 2 interrupt flag */
    PWMA_FLAG_CC3    = 4,   /*!< Capture/Compare 3 interrupt flag */
    PWMA_FLAG_COM    = 5,   /*!< COM interrupt flag */
    PWMA_FLAG_TRIG   = 6,   /*!< Trigger interrupt flag */
    PWMA_FLAG_BRK    = 7,   /*!< Break interrupt flag */
    PWMA_FLAG_CC0OVR = 9,   /*!< Capture/Compare 0 overcapture flag */
    PWMA_FLAG_CC1OVR = 10,  /*!< Capture/Compare 1 overcapture flag */
    PWMA_FLAG_CC2OVR = 11,  /*!< Capture/Compare 2 overcapture flag */
    PWMA_FLAG_CC3OVR = 12   /*!< Capture/Compare 3 overcapture flag */
} PWMA_Flag_TypeDef;

/** @brief Interrupts */
typedef enum {
    PWMA_IT_UPDATE = 0,     /*!< Update interrupt */
    PWMA_IT_CC0    = 1,     /*!< Capture/Compare 0 interrupt */
    PWMA_IT_CC1    = 2,     /*!< Capture/Compare 1 interrupt */
    PWMA_IT_CC2    = 3,     /*!< Capture/Compare 2 interrupt */
    PWMA_IT_CC3    = 4,     /*!< Capture/Compare 3 interrupt */
    PWMA_IT_COM    = 5,     /*!< COM interrupt */
    PWMA_IT_TRIG   = 6,     /*!< Trigger interrupt */
    PWMA_IT_BRK    = 7      /*!< Break interrupt */
} PWMA_IT_TypeDef;

/** @brief DMA requests */
typedef enum {
    PWMA_DMA_REQ_UPDATE = 8,    /*!< Update DMA request */
    PWMA_DMA_REQ_CC0    = 9,    /*!< Capture/Compare 0 DMA request */
    PWMA_DMA_REQ_CC1    = 10,   /*!< Capture/Compare 1 DMA request */
    PWMA_DMA_REQ_CC2    = 11,   /*!< Capture/Compare 2 DMA request */
    PWMA_DMA_REQ_CC3    = 12,   /*!< Capture/Compare 3 DMA request */
    PWMA_DMA_REQ_COM    = 13,   /*!< COM DMA request */
    PWMA_DMA_REQ_TRIG   = 14    /*!< Trigger DMA request */
} PWMA_DMAReq_TypeDef;

/** @brief Events */
typedef enum {
    PWMA_EVENT_UPDATE = 0,  /*!< Update event */
    PWMA_EVENT_CC0    = 1,  /*!< Capture/Compare 0 event */
    PWMA_EVENT_CC1    = 2,  /*!< Capture/Compare 0 event */
    PWMA_EVENT_CC2    = 3,  /*!< Capture/Compare 0 event */
    PWMA_EVENT_CC3    = 4,  /*!< Capture/Compare 0 event */
    PWMA_EVENT_COM    = 5,  /*!< COM event */
    PWMA_EVENT_TRIG   = 6,  /*!< Trigger event */
    PWMA_EVENT_BRK    = 7   /*!< Break event */
} PWMA_Event_TypeDef;

/**
 * @brief  PWMA Time Base configuration structure definition.
 */
typedef struct
{
    uint16_t Prescaler;                         /*!< Specifies the prescaler value used to divide the PWMA clock.
                                                     This parameter can be a number between Min_Data=0x0000 and Max_Data=0xFFFF.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetPrescaler().*/

    PWMA_CounterMode_TypeDef CounterMode;       /*!< Specifies the counter mode.
                                                     This parameter can be a value of @ref PWMA_CounterMode_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetCounterMode().*/

    uint16_t Autoreload;                        /*!< Specifies the auto reload value to be loaded into the active
                                                     Auto-Reload Register at the next update event.
                                                     This parameter must be a number between Min_Data=0x0000 and Max_Data=0xFFFF.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetAutoReload().*/

    PWMA_ClockDivision_TypeDef ClockDivision;   /*!< Specifies the clock division.
                                                     This parameter can be a value of @ref PWMA_ClockDivision_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetClockDivision().*/

    uint8_t RepetitionCounter;                  /*!< Specifies the repetition counter value. Each time the RCR downcounter
                                                     reaches zero, an update event is generated and counting restarts
                                                     from the RCR value (N).
                                                     This means in PWM mode that (N+1) corresponds to:
                                                     - the number of PWM periods in edge-aligned mode
                                                     - the number of half PWM period in center-aligned mode
                                                     This parameter must be a number between 0x00 and 0xFF.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetRepetitionCounter().*/
} PWMA_InitStruct_TypeDef;

/**
 * @brief  PWMA Output Compare configuration structure definition.
 */
typedef struct
{
    PWMA_OC_Mode_TypeDef OCMode;                /*!< Specifies the output mode.
                                                     This parameter can be a value of @ref PWMA_OC_Mode_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_OC_SetMode().*/

    PWMA_OC_State_TypeDef OCState;              /*!< Specifies the PWMA Output Compare state.
                                                     This parameter can be a value of @ref PWMA_OC_State_TypeDef.

                                                     This feature can be modified afterwards using unitary functions @ref PWMA_CC_EnableChannel() or @ref PWMA_CC_DisableChannel().*/

    PWMA_OC_State_TypeDef OCNState;             /*!< Specifies the PWMA complementary Output Compare state.
                                                     This parameter can be a value of @ref PWMA_OC_State_TypeDef.

                                                     This feature can be modified afterwards using unitary functions @ref PWMA_CC_EnableChannel() or @ref PWMA_CC_DisableChannel().*/

    uint16_t CompareValue;                      /*!< Specifies the Compare value to be loaded into the Capture Compare Register.
                                                     This parameter can be a number between Min_Data=0x0000 and Max_Data=0xFFFF.

                                                     This feature can be modified afterwards using unitary function PWMA_OC_SetCompareCHx (x=1..6).*/

    PWMA_OC_Polarity_TypeDef OCPolarity;        /*!< Specifies the output polarity.
                                                     This parameter can be a value of @ref PWMA_OC_Polarity_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_OC_SetPolarity().*/

    PWMA_OC_Polarity_TypeDef OCNPolarity;       /*!< Specifies the complementary output polarity.
                                                     This parameter can be a value of @ref PWMA_OC_Polarity_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_OC_SetPolarity().*/

    PWMA_OC_IdleState_TypeDef OCIdleState;      /*!< Specifies the PWMA Output Compare pin state during Idle state.
                                                     This parameter can be a value of @ref PWMA_OC_IdleState_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_OC_SetIdleState().*/

    PWMA_OC_IdleState_TypeDef OCNIdleState;     /*!< Specifies the PWMA Output Compare pin state during Idle state.
                                                     This parameter can be a value of @ref PWMA_OC_IdleState_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_OC_SetIdleState().*/
} PWMA_OC_InitStruct_TypeDef;

/**
 * @brief  PWMA Input Capture configuration structure definition.
 */
typedef struct
{
    PWMA_IC_Polarity_TypeDef ICPolarity;        /*!< Specifies the active edge of the input signal.
                                                     This parameter can be a value of @ref PWMA_IC_Polarity_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_IC_SetPolarity().*/

    PWMA_IC_ActiveInput_TypeDef ICActiveInput;  /*!< Specifies the input.
                                                     This parameter can be a value of @ref PWMA_IC_ActiveInput_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_IC_SetActiveInput().*/

    PWMA_IC_Prescaler_TypeDef ICPrescaler;      /*!< Specifies the Input Capture Prescaler.
                                                     This parameter can be a value of @ref PWMA_IC_Prescaler_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_IC_SetPrescaler().*/

    PWMA_IC_Filter_TypeDef ICFilter;            /*!< Specifies the input capture filter.
                                                     This parameter can be a value of @ref PWMA_IC_Filter_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_IC_SetFilter().*/
} PWMA_IC_InitStruct_TypeDef;

/**
 * @brief  ETR (External Trigger) structure definition
 */
typedef struct
{
    PWMA_ClockSource_TypeDef ClockSource;       /*!< Specifies the clock source of the counter.
                                                     This parameter can be a value of @ref PWMA_ClockSource_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetClockSource().*/

    PWMA_SlaveMode_TypeDef SlaveMode;           /*!< Specifies the synchronization mode of a slave timer.
                                                     This parameter can be a value of @ref PWMA_SlaveMode_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetSlaveMode().*/

    PWMA_TS_TypeDef TriggerInput;               /*!< Specifies the timer input synchronization signal.
                                                     This parameter can be a value of @ref PWMA_TS_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_SetTriggerInput().*/

    PWMA_ETR_Polarity_TypeDef ETRPolarity;      /*!< Specifies the external trigger signal polarity.
                                                     This parameter can be a value of @ref PWMA_ETR_Polarity_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_ETR_SetPolarity().*/

    PWMA_ETR_Prescaler_TypeDef ETRPrescaler;    /*!< Specifies the external trigger signal prescaler.
                                                     This parameter can be a value of @ref PWMA_ETR_Prescaler_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_ETR_SetPrescaler().*/

    PWMA_ETR_Filter_TypeDef ETRFilter;          /*!< Specifies the external trigger signal filter.
                                                     This parameter can be a value of @ref PWMA_ETR_Filter_TypeDef.

                                                     This feature can be modified afterwards using unitary function @ref PWMA_ETR_SetFilter().*/
} PWMA_ETR_InitStruct_TypeDef;

/**
 * @brief  BDTR (Break and Dead Time) structure definition
 */
typedef struct
{
    PWMA_OSSR_TypeDef OSSRState;                    /*!< Specifies the Off-State selection used in Run mode.
                                                         This parameter can be a value of @ref PWMA_OSSR_TypeDef

                                                         This feature can be modified afterwards using unitary function @ref PWMA_SetOffStates()*/

    uint8_t DeadTime;                               /*!< Specifies the delay time between the switching-off and the switching-on of the outputs.
                                                         This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF.

                                                         This feature can be modified afterwards using unitary function @ref PWMA_OC_SetDeadTime() */

    PWMA_BRK_State_TypeDef BreakState;              /*!< Specifies whether the PWMA break input (BRK) is enabled or not.
                                                         This parameter can be a value of @ref PWMA_BRK_State_TypeDef

                                                         This feature can be modified afterwards using unitary functions @ref PWMA_EnableBreak() or @ref PWMA_DisableBreak() */

    PWMA_BRK_Polarity_TypeDef BreakPolarity;        /*!< Specifies the PWMA break input (BRK) polarity.
                                                         This parameter can be a value of @ref PWMA_BRK_Polarity_TypeDef

                                                         This feature can be modified afterwards using unitary function @ref PWMA_SetBreakPolarity() */

    PWMA_AutomaticOutput_TypeDef AutomaticOutput;   /*!< Specifies whether the PWMA Automatic Output feature is enabled or not.
                                                         This parameter can be a value of @ref PWMA_AutomaticOutput_TypeDef

                                                         This feature can be modified afterwards using unitary functions @ref PWMA_EnableAutomaticOutput() or @ref PWMA_DisableAutomaticOutput() */
} PWMA_BDTR_InitStruct_TypeDef;

/* Exported macro ------------------------------------------------------------*/

/** @brief PWMA channels. */
#define PWMA_CH0            PWMA_CCER_CC0E      /*!< Channel 0 */
#define PWMA_CH0N           PWMA_CCER_CC0NE     /*!< Channel 0 (complementary) */
#define PWMA_CH1            PWMA_CCER_CC1E      /*!< Channel 1 */
#define PWMA_CH1N           PWMA_CCER_CC1NE     /*!< Channel 1 (complementary) */
#define PWMA_CH2            PWMA_CCER_CC2E      /*!< Channel 2 */
#define PWMA_CH2N           PWMA_CCER_CC2NE     /*!< Channel 2 (complementary) */
#define PWMA_CH3            PWMA_CCER_CC3E      /*!< Channel 3 */

/**
 * @brief  HELPER macro calculating the prescaler value to achieve the required counter clock frequency.
 * @note ex: @ref __PWMA_CALC_PSC (80000000, 1000000);
 * @param  __PWMACLK__ timer input clock frequency (in Hz)
 * @param  __CNTCLK__ counter clock frequency (in Hz)
 * @retval Prescaler value  (between Min_Data=0 and Max_Data=65535)
 */
#define __PWMA_CALC_PSC(__PWMACLK__, __CNTCLK__)            (((__PWMACLK__) >= (__CNTCLK__)) ? \
                                                             ((uint16_t)(((__PWMACLK__) + (__CNTCLK__) / 2U) / (__CNTCLK__)) - 1U) : 0U)

/**
 * @brief  HELPER macro calculating the auto-reload value to achieve the required output signal frequency.
 * @note ex: @ref __PWMA_CALC_ARR (1000000, @ref PWMA_GetPrescaler (), 10000);
 * @param  __PWMACLK__ timer input clock frequency (in Hz)
 * @param  __PSC__ prescaler
 * @param  __FREQ__ output signal frequency (in Hz)
 * @retval  Auto-reload value  (between Min_Data=0 and Max_Data=65535)
 */
#define __PWMA_CALC_ARR(__PWMACLK__, __PSC__, __FREQ__)     ((((__PWMACLK__) / ((__PSC__) + 1U)) >= (__FREQ__)) ? \
                                                             ((uint16_t)((__PWMACLK__) / ((__PSC__) + 1U) / (__FREQ__)) - 1U) : 0U)

/**
 * @brief  Write a value in PWMA register.
 * @param  __INSTANCE__ PWMA Instance
 * @param  __REG__ Register to be written
 * @param  __VALUE__ Value to be written in the register
 * @retval None
 */
#define PWMA_WriteReg(__INSTANCE__, __REG__, __VALUE__)     WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
 * @brief  Read a value in PWMA register.
 * @param  __INSTANCE__ PWMA Instance
 * @param  __REG__ Register to be read
 * @retval Register value
 */
#define PWMA_ReadReg(__INSTANCE__, __REG__)                 READ_REG(__INSTANCE__->__REG__)

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Enable PWMA counter
 *          External clock, gated mode and encoder mode can work only if the
 *          CEN bit has been previously set by software. However trigger mode
 *          can set the CEN bit automatically by hardware.
 * @param   PWMAx The PWMA instance
 * @retval  None
 * @see     PWMA_DisableCounter()
 */
__STATIC_INLINE void PWMA_EnableCounter(PWMA_TypeDef* PWMAx)
{
    SET_BIT(PWMAx->CR1, PWMA_CR1_CEN);
}

/**
 * @brief   Disable PWMA counter
 * @param   PWMAx The PWMA instance
 * @retval  None
 * @see     PWMA_EnableCounter()
 */
__STATIC_INLINE void PWMA_DisableCounter(PWMA_TypeDef* PWMAx)
{
    CLEAR_BIT(PWMAx->CR1, PWMA_CR1_CEN);
}

/**
 * @brief   Indicates whether the timer counter is enabled.
 * @param   PWMAx The PWMA instance
 * @retval  State of bit (1 or 0).
 * @see     PWMA_Enable()
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledCounter(PWMA_TypeDef* PWMAx)
{
    return (READ_BIT(PWMAx->CR1, PWMA_CR1_CEN) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief   Enable PWMA update event generation.
 *          This bit is set and cleared by software to enable/disable UEV event generation.
 * @param   PWMAx The PWMA instance
 * @retval  None
 * @see     PWMA_DisableUpdateEvent()
 */
__STATIC_INLINE void PWMA_EnableUpdateEvent(PWMA_TypeDef* PWMAx)
{
    SET_BIT(PWMAx->CR1, PWMA_CR1_UDIS);
}

/**
 * @brief   Disable PWMA update event generation.
 *          This bit is set and cleared by software to enable/disable UEV event generation.
 * @param   PWMAx The PWMA instance
 * @retval  None
 * @see     PWMA_EnableUpdateEvent()
 */
__STATIC_INLINE void PWMA_DisableUpdateEvent(PWMA_TypeDef* PWMAx)
{
    CLEAR_BIT(PWMAx->CR1, PWMA_CR1_UDIS);
}

/**
 * @brief   Indicates whether update event generation is enabled.
 * @param   PWMAx The PWMA instance
 * @retval  State of bit (1 or 0).
 * @see     PWMA_EnableUpdateEvent()
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledUpdateEvent(PWMA_TypeDef* PWMAx)
{
    return (READ_BIT(PWMAx->CR1, PWMA_CR1_UDIS) == 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief   Set one pulse mode (one shot v.s. repetitive).
 * @param   PWMAx The PWMA instance
 * @param   OnePulseMode This parameter can be one of PWMA_OnePulseMode_TypeDef values
 * @retval  None
 * @see     PWMA_GetOnePulseMode()
 */
__STATIC_INLINE void PWMA_SetOnePulseMode(PWMA_TypeDef *PWMAx, PWMA_OnePulseMode_TypeDef OnePulseMode)
{
    MODIFY_REG(PWMAx->CR1, PWMA_CR1_OPM, (uint32_t)OnePulseMode);
}

/**
 * @brief   Get actual one pulse mode.
 * @param   PWMAx The PWMA instance
 * @retval  Returned value can be one of PWMA_OnePulseMode_TypeDef values
 * @see     PWMA_SetOnePulseMode()
 */
__STATIC_INLINE PWMA_OnePulseMode_TypeDef PWMA_GetOnePulseMode(PWMA_TypeDef *PWMAx)
{
    return (PWMA_OnePulseMode_TypeDef)READ_BIT(PWMAx->CR1, PWMA_CR1_OPM);
}

/**
 * @brief  Set the timer counter counting mode.
 * @note Switching from Center Aligned counter mode to Edge counter mode (or reverse)
 *       requires a timer reset to avoid unexpected direction
 *       due to DIR bit readonly in center aligned mode.
 * @param  PWMAx The PWMA instance
 * @param  CounterMode This parameter can be one of PWMA_CounterMode_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_SetCounterMode(PWMA_TypeDef *PWMAx, PWMA_CounterMode_TypeDef CounterMode)
{
    MODIFY_REG(PWMAx->CR1, (PWMA_CR1_CMS | PWMA_CR1_DIR), (uint32_t)CounterMode);
}

/**
 * @brief  Get actual counter mode.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_CounterMode_TypeDef values:
 */
__STATIC_INLINE PWMA_CounterMode_TypeDef PWMA_GetCounterMode(PWMA_TypeDef *PWMAx)
{
    return (PWMA_CounterMode_TypeDef)READ_BIT(PWMAx->CR1, PWMA_CR1_CMS | PWMA_CR1_DIR);
}

/**
 * @brief  Enable auto-reload (ARR) preload.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableARRPreload(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->CR1, PWMA_CR1_ARPE);
}

/**
 * @brief  Disable auto-reload (ARR) preload.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableARRPreload(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->CR1, PWMA_CR1_ARPE);
}

/**
 * @brief  Indicates whether auto-reload (ARR) preload is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledARRPreload(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->CR1, PWMA_CR1_ARPE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Set the division ratio between the timer clock and the sampling clock
 *         used by the dead-time generators (when supported) and the digital filters.
 * @param  PWMAx Timer instance
 * @param  ClockDivision This parameter can be one of the PWMA_ClockDivision_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_SetClockDivision(PWMA_TypeDef *PWMAx, PWMA_ClockDivision_TypeDef ClockDivision)
{
    MODIFY_REG(PWMAx->CR1, PWMA_CR1_CKD, (uint32_t)ClockDivision);
}

/**
 * @brief  Get the actual division ratio between the timer clock and the sampling clock
 *         used by the dead-time generators (when supported) and the digital filters.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_ClockDivision_TypeDef values.
 */
__STATIC_INLINE PWMA_ClockDivision_TypeDef PWMA_GetClockDivision(PWMA_TypeDef *PWMAx)
{
    return (PWMA_ClockDivision_TypeDef)READ_BIT(PWMAx->CR1, PWMA_CR1_CKD);
}

/**
 * @brief  Set the counter value.
 * @param  PWMAx Timer instance
 * @param  Counter Counter value (between Min_Data=0 and Max_Data=0xFFFF)
 * @retval None
 */
__STATIC_INLINE void PWMA_SetCounter(PWMA_TypeDef *PWMAx, uint16_t Counter)
{
    WRITE_REG(PWMAx->CNT, Counter);
}

/**
 * @brief  Get the counter value.
 * @param  PWMAx Timer instance
 * @retval Counter value (between Min_Data=0 and Max_Data=0xFFFF)
 */
__STATIC_INLINE uint16_t PWMA_GetCounter(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)READ_REG(PWMAx->CNT);
}

/**
 * @brief  Get the current direction of the counter
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of the PWMA_CounterDirection_TypeDef values.
 */
__STATIC_INLINE PWMA_CounterDirection_TypeDef PWMA_GetDirection(PWMA_TypeDef *PWMAx)
{
    return (PWMA_CounterDirection_TypeDef)READ_BIT(PWMAx->CR1, PWMA_CR1_DIR);
}

/**
 * @brief  Set the prescaler value.
 * @note The counter clock frequency CK_CNT is equal to fCK_PSC / (PSC[15:0] + 1).
 * @note The prescaler can be changed on the fly as this control register is buffered. The new
 *       prescaler ratio is taken into account at the next update event.
 * @note Helper macro @ref __PWMA_CALC_PSC can be used to calculate the Prescaler parameter
 * @param  PWMAx Timer instance
 * @param  Prescaler between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_SetPrescaler(PWMA_TypeDef *PWMAx, uint16_t Prescaler)
{
    WRITE_REG(PWMAx->PSC, Prescaler);
}

/**
 * @brief  Get the prescaler value.
 * @param  PWMAx Timer instance
 * @retval  Prescaler value between Min_Data=0 and Max_Data=65535
 */
__STATIC_INLINE uint16_t PWMA_GetPrescaler(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)READ_REG(PWMAx->PSC);
}

/**
 * @brief  Set the auto-reload value.
 * @note ARR is the value to be loaded in the actual auto-reload register.
 *       Buffer can be enabled for this register (see ARPE in CR1).
 *       The counter is blocked while the auto-reload value is null.
 * @note Helper macro @ref __PWMA_CALC_ARR can be used to calculate the AutoReload parameter
 * @param  PWMAx Timer instance
 * @param  AutoReload between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_SetAutoReload(PWMA_TypeDef *PWMAx, uint16_t AutoReload)
{
    WRITE_REG(PWMAx->ARR, AutoReload);
}

/**
 * @brief  Get the auto-reload value.
 * @param  PWMAx Timer instance
 * @retval Auto-reload value
 */
__STATIC_INLINE uint16_t PWMA_GetAutoReload(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)READ_REG(PWMAx->ARR);
}

/**
 * @brief  Set the repetition counter value.
 * @param  PWMAx Timer instance
 * @param  RepetitionCounter between Min_Data=0 and Max_Data=255
 * @retval None
 */
__STATIC_INLINE void PWMA_SetRepetitionCounter(PWMA_TypeDef *PWMAx, uint8_t RepetitionCounter)
{
    WRITE_REG(PWMAx->RCR, RepetitionCounter);
}

/**
 * @brief  Get the repetition counter value.
 * @param  PWMAx Timer instance
 * @retval Repetition counter value
 */
__STATIC_INLINE uint8_t PWMA_GetRepetitionCounter(PWMA_TypeDef *PWMAx)
{
    return (uint8_t)READ_REG(PWMAx->RCR);
}

/**
 * @brief  Enable the capture/compare control bits (CCxE, CCxNE and OCxM) preload.
 * @note CCxE, CCxNE and OCxM bits are preloaded, after having been written,
 *       they are updated only when a commutation event (COM) occurs.
 * @note Only on channels that have a complementary output.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_CC_EnablePreload(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->CR2, PWMA_CR2_CCPC);
}

/**
 * @brief  Disable the capture/compare control bits (CCxE, CCxNE and OCxM) preload.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_CC_DisablePreload(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->CR2, PWMA_CR2_CCPC);
}

/**
 * @brief  Indicates whether the capture/compare control bits (CCxE, CCxNE and OCxM)
 *         preload is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_CC_IsEnabledPreload(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->CR2, PWMA_CR2_CCPC)) != 0x0UL ? 1UL : 0UL;
}

/**
 * @brief  Set the update source of the capture/compare control bits (CCxE, CCxNE and OCxM).
 * @note   Only on channels that have a complementary output.
 * @param  PWMAx Timer instance
 * @param  UpdateSource This parameter can be one of PWMA_CC_UpdateSource_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_CC_SetUpdateSource(PWMA_TypeDef *PWMAx, PWMA_CC_UpdateSource_TypeDef UpdateSource)
{
    MODIFY_REG(PWMAx->CR2, PWMA_CR2_CCUS, (uint32_t)UpdateSource);
}

/**
 * @brief  Get the update source of the capture/compare control bits (CCxE, CCxNE and OCxM).
 * @param  PWMAx Timer instance
 * @param  The update source. This parameter can be one of PWMA_CC_UpdateSource_TypeDef values.
 */
__STATIC_INLINE PWMA_CC_UpdateSource_TypeDef PWMA_CC_GetUpdateSource(PWMA_TypeDef *PWMAx)
{
    return (PWMA_CC_UpdateSource_TypeDef)READ_BIT(PWMAx->CR2, PWMA_CR2_CCUS);
}

/**
 * @brief  Enable capture/compare channels.
 * @param  PWMAx Timer instance
 * @param  Channels This parameter can be a combination of PWMA_CHx(N) values.
 * @retval None
 */
__STATIC_INLINE void PWMA_CC_EnableChannel(PWMA_TypeDef *PWMAx, uint32_t Channels)
{
    SET_BIT(PWMAx->CCER, Channels);
}

/**
 * @brief  Disable capture/compare channels.
 * @param  PWMAx Timer instance
 * @param  Channels This parameter can be a combination of PWMA_CHx(N) values.
 * @retval None
 */
__STATIC_INLINE void PWMA_CC_DisableChannel(PWMA_TypeDef *PWMAx, uint32_t Channels)
{
    CLEAR_BIT(PWMAx->CCER, Channels);
}

/**
 * @brief  Indicate whether channel(s) is(are) enabled.
 * @param  PWMAx Timer instance
 * @param  Channels This parameter can be a combination of PWMA_CHx(N) values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_CC_IsEnabledChannel(PWMA_TypeDef *PWMAx, uint32_t Channels)
{
    return ((READ_BIT(PWMAx->CCER, Channels) == Channels) ? 1UL : 0UL);
}

/**
 * @brief  Configure an output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx(N) values.
 * @param  PWMA_OC_InitStruct The pointer to the @ref PWMA_OC_InitStruct_TypeDef structure.
 *                            The next parameters will be configured:
 *                            - OCMode (only for PWMA_CHx)
 *                            - OCPolarity or OCNPolarity
 *                            - OCIdleState or OCNIdleState
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_Config(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    if ((iChannel & 0x1U) == 0x0U)
    {
        /* OCx output */
        __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
        MODIFY_REG(*pReg,
                   ((PWMA_CCMR1_OC_OC0M | PWMA_CCMR1_CC0S) << SHIFT_TAB_OCxx[iChannel]),
                   (((uint32_t)PWMA_OC_InitStruct->OCMode | PWMA_CCMR1_CC0S_OC) << SHIFT_TAB_OCxx[iChannel]));

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel]),
                   ((uint32_t)PWMA_OC_InitStruct->OCPolarity << SHIFT_TAB_CCxP[iChannel]));

        MODIFY_REG(PWMAx->CR2,
                   (PWMA_CR2_OIS0 << SHIFT_TAB_OISx[iChannel]),
                   ((uint32_t)PWMA_OC_InitStruct->OCIdleState << SHIFT_TAB_OISx[iChannel]));
    }
    else
    {
        /* OCxN output */
        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel]),
                   ((uint32_t)PWMA_OC_InitStruct->OCNPolarity << SHIFT_TAB_CCxP[iChannel]));

        MODIFY_REG(PWMAx->CR2,
                   (PWMA_CR2_OIS0 << SHIFT_TAB_OISx[iChannel]),
                   ((uint32_t)PWMA_OC_InitStruct->OCNIdleState << SHIFT_TAB_OISx[iChannel]));
    }
}

/**
 * @brief  Define the behavior of the output reference signal OCxREF from which
 *         OCx and OCxN (when relevant) are derived.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  Mode This parameter can be one of the PWMA_OC_Mode_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetMode(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_Mode_TypeDef Mode)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    MODIFY_REG(*pReg,
               ((PWMA_CCMR1_OC_OC0M | PWMA_CCMR1_CC0S) << SHIFT_TAB_OCxx[iChannel]),
               (((uint32_t)Mode | PWMA_CCMR1_CC0S_OC) << SHIFT_TAB_OCxx[iChannel]));
}

/*
 * @brief  Get the output compare mode of an output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval None
 */
__STATIC_INLINE PWMA_OC_Mode_TypeDef PWMA_OC_GetMode(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (PWMA_OC_Mode_TypeDef)(READ_BIT(*pReg, (PWMA_CCMR1_OC_OC0M << SHIFT_TAB_OCxx[iChannel])) >> SHIFT_TAB_OCxx[iChannel]);
}

/**
 * @brief  Set the polarity of an output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx(N) values.
 * @param  Polarity This parameter can be one of PWMA_OC_Polarity_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetPolarity(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_Polarity_TypeDef Polarity)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    MODIFY_REG(PWMAx->CCER,
               (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel]),
               ((uint32_t)Polarity << SHIFT_TAB_CCxP[iChannel]));
}

/**
 * @brief  Get the polarity of an output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx(N) values.
 * @retval Returned value can be one of PWMA_OC_Polarity_TypeDef values
 */
__STATIC_INLINE PWMA_OC_Polarity_TypeDef PWMA_OC_GetPolarity(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    return (PWMA_OC_Polarity_TypeDef)(READ_BIT(PWMAx->CCER, (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel])) >> SHIFT_TAB_CCxP[iChannel]);
}

/**
 * @brief  Set the IDLE state of an output channel
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx(N) values.
 * @param  IdleState This parameter can be one of PWMA_OC_IdleState_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetIdleState(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_IdleState_TypeDef IdleState)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    MODIFY_REG(PWMAx->CR2,
               (PWMA_CR2_OIS0 << SHIFT_TAB_OISx[iChannel]),
               ((uint32_t)IdleState << SHIFT_TAB_OISx[iChannel]));
}

/**
 * @brief  Get the IDLE state of an output channel
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx(N) values.
 * @retval Returned value can be one of PWMA_OC_IdleState_TypeDef values
 */
__STATIC_INLINE PWMA_OC_IdleState_TypeDef PWMA_OC_GetIdleState(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    return (PWMA_OC_IdleState_TypeDef)(READ_BIT(PWMAx->CR2, (PWMA_CR2_OIS0 << SHIFT_TAB_OISx[iChannel])) >> SHIFT_TAB_OISx[iChannel]);
}

/**
 * @brief  Enable compare register (PWMAx_CCRx) preload for the output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_EnablePreload(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    SET_BIT(*pReg, (PWMA_CCMR1_OC_OC0PE << SHIFT_TAB_OCxx[iChannel]));
}

/**
 * @brief  Disable compare register (PWMAx_CCRx) preload for the output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_DisablePreload(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    CLEAR_BIT(*pReg, (PWMA_CCMR1_OC_OC0PE << SHIFT_TAB_OCxx[iChannel]));
}

/**
 * @brief  Indicates whether compare register (PWMAx_CCRx) preload is enabled for the output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_OC_IsEnabledPreload(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (READ_BIT(*pReg, (PWMA_CCMR1_OC_OC0PE << SHIFT_TAB_OCxx[iChannel])) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable clearing the output channel on an external event.
 * @note   This function can only be used in Output compare and PWM modes. It does not work in Forced mode.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_EnableClear(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    SET_BIT(*pReg, (PWMA_CCMR1_OC_OC0CE << SHIFT_TAB_OCxx[iChannel]));
}

/**
 * @brief  Disable clearing the output channel on an external event.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_DisableClear(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    CLEAR_BIT(*pReg, (PWMA_CCMR1_OC_OC0CE << SHIFT_TAB_OCxx[iChannel]));
}

/**
 * @brief  Indicates whether clearing the output channel on an external event is enabled for the output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_OC_IsEnabledClear(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (READ_BIT(*pReg, PWMA_CCMR1_OC_OC0CE << SHIFT_TAB_OCxx[iChannel]) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Set the dead-time delay (delay inserted between the rising edge of the OCxREF signal 
 *         and the rising edge if the OCx and OCxN signals).
 * @param  PWMAx Timer instance
 * @param  DeadTime between Min_Data=0 and Max_Data=255
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetDeadTime(PWMA_TypeDef *PWMAx, uint8_t DeadTime)
{
    MODIFY_REG(PWMAx->BDTR, PWMA_BDTR_DTG, DeadTime);
}

/**
 * @brief  Get the dead-time delay (delay inserted between the rising edge of the OCxREF signal 
 *         and the rising edge if the OCx and OCxN signals).
 * @param  PWMAx Timer instance
 * @retval Dead-time delay between Min_Data=0 and Max_Data=255
 */
__STATIC_INLINE uint8_t PWMA_OC_GetDeadTime(PWMA_TypeDef *PWMAx)
{
    return (uint8_t)READ_BIT(PWMAx->BDTR, PWMA_BDTR_DTG);
}

/**
 * @brief  Set compare value for output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  CompareValue between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetCompare(PWMA_TypeDef *PWMAx, uint32_t Channel, uint16_t CompareValue)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCR0) + ((iChannel >> 1) << 2));
    WRITE_REG(*pReg, CompareValue);
}

/**
 * @brief  Set compare value for output channel 0 (PWMAx_CCR0).
 * @param  PWMAx Timer instance
 * @param  CompareValue between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetCompareCH0(PWMA_TypeDef *PWMAx, uint16_t CompareValue)
{
    WRITE_REG(PWMAx->CCR0, CompareValue);
}

/**
 * @brief  Set compare value for output channel 1 (PWMAx_CCR1).
 * @param  PWMAx Timer instance
 * @param  CompareValue between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetCompareCH1(PWMA_TypeDef *PWMAx, uint16_t CompareValue)
{
    WRITE_REG(PWMAx->CCR1, CompareValue);
}

/**
 * @brief  Set compare value for output channel 2 (PWMAx_CCR2).
 * @param  PWMAx Timer instance
 * @param  CompareValue between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetCompareCH2(PWMA_TypeDef *PWMAx, uint16_t CompareValue)
{
    WRITE_REG(PWMAx->CCR2, CompareValue);
}

/**
 * @brief  Set compare value for output channel 3 (PWMAx_CCR3).
 * @param  PWMAx Timer instance
 * @param  CompareValue between Min_Data=0 and Max_Data=65535
 * @retval None
 */
__STATIC_INLINE void PWMA_OC_SetCompareCH3(PWMA_TypeDef *PWMAx, uint16_t CompareValue)
{
    WRITE_REG(PWMAx->CCR3, CompareValue);
}

/**
 * @brief  Get compare value for output channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval CompareValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_OC_GetCompare(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCR0) + ((iChannel >> 1) << 2));

    return (uint16_t)READ_REG(*pReg);
}

/**
 * @brief  Get compare value (PWMAx_CCR0) set for  output channel 0.
 * @param  PWMAx Timer instance
 * @retval CompareValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_OC_GetCompareCH0(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR0));
}

/**
 * @brief  Get compare value (PWMAx_CCR1) set for  output channel 1.
 * @param  PWMAx Timer instance
 * @retval CompareValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_OC_GetCompareCH1(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR1));
}

/**
 * @brief  Get compare value (PWMAx_CCR2) set for  output channel 2.
 * @param  PWMAx Timer instance
 * @retval CompareValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_OC_GetCompareCH2(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR2));
}

/**
 * @brief  Get compare value (PWMAx_CCR3) set for  output channel 3.
 * @param  PWMAx Timer instance
 * @retval CompareValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_OC_GetCompareCH3(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR3));
}

/**
 * @brief  Configure input channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  PWMA_IC_InitStruct The pointer to the @ref PWMA_IC_InitStruct_TypeDef structure.
 *                            The next parameters will be configured:
 *                            - ICActiveInput
 *                            - ICPrescaler
 *                            - ICFilter
 *                            - ICPolarity
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_Config(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    if ((iChannel & 0x1U) == 0x0U)
    {
        /* ICx input */
        __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
        MODIFY_REG(*pReg,
                   ((PWMA_CCMR1_IC_IC0F | PWMA_CCMR1_IC_IC0PSC | PWMA_CCMR1_CC0S) << SHIFT_TAB_ICxx[iChannel]),
                   (((uint32_t)PWMA_IC_InitStruct->ICFilter |
                     (uint32_t)PWMA_IC_InitStruct->ICPrescaler |
                     (uint32_t)PWMA_IC_InitStruct->ICActiveInput) << SHIFT_TAB_ICxx[iChannel]));

        MODIFY_REG(PWMAx->CCER,
                   (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel]),
                   ((uint32_t)PWMA_IC_InitStruct->ICPolarity << SHIFT_TAB_CCxP[iChannel]));
    }
}

/**
 * @brief  Set the active input.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  ActiveInput This parameter can be one of PWMA_IC_ActiveInput_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_SetActiveInput(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_ActiveInput_TypeDef ActiveInput)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    MODIFY_REG(*pReg,
               (PWMA_CCMR1_CC0S << SHIFT_TAB_ICxx[iChannel]),
               ((uint32_t)ActiveInput << SHIFT_TAB_ICxx[iChannel]));
}

/**
 * @brief  Get the active input.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval Returned value can be one of PWMA_IC_ActiveInput_TypeDef values
 */
__STATIC_INLINE PWMA_IC_ActiveInput_TypeDef PWMA_IC_GetActiveInput(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (PWMA_IC_ActiveInput_TypeDef)(READ_BIT(*pReg, (PWMA_CCMR1_CC0S << SHIFT_TAB_ICxx[iChannel])) >> SHIFT_TAB_ICxx[iChannel]);
}

/**
 * @brief  Set the prescaler of input channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  ICPrescaler This parameter can be one of PWMA_IC_Prescaler_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_SetPrescaler(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_Prescaler_TypeDef ICPrescaler)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    MODIFY_REG(*pReg,
               (PWMA_CCMR1_IC_IC0PSC << SHIFT_TAB_ICxx[iChannel]),
               ((uint32_t)ICPrescaler << SHIFT_TAB_ICxx[iChannel]));
}

/**
 * @brief  Get the prescaler of input channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval Returned value can be one of PWMA_IC_Prescaler_TypeDef values
 */
__STATIC_INLINE PWMA_IC_Prescaler_TypeDef PWMA_IC_GetPrescaler(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (PWMA_IC_Prescaler_TypeDef)(READ_BIT(*pReg, (PWMA_CCMR1_IC_IC0PSC << SHIFT_TAB_ICxx[iChannel])) >> SHIFT_TAB_ICxx[iChannel]);
}

/**
 * @brief  Set the input filter duration.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  ICFilter This parameter can be one of PWMA_IC_Filter_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_SetFilter(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_Filter_TypeDef ICFilter)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);
    MODIFY_REG(*pReg,
               (PWMA_CCMR1_IC_IC0F << SHIFT_TAB_ICxx[iChannel]),
               ((uint32_t)ICFilter << SHIFT_TAB_ICxx[iChannel]));
}

/**
 * @brief  Get the input filter duration.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval Returned value can be one of PWMA_IC_Filter_TypeDef values
 */
__STATIC_INLINE PWMA_IC_Filter_TypeDef PWMA_IC_GetFilter(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCMR1) + OFFSET_TAB_CCMRx[iChannel]);

    return (PWMA_IC_Filter_TypeDef)(READ_BIT(*pReg, (PWMA_CCMR1_IC_IC0F << SHIFT_TAB_ICxx[iChannel])) >> SHIFT_TAB_ICxx[iChannel]);
}

/**
 * @brief  Set the input channel polarity.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @param  ICPolarity This parameter can be one of PWMA_IC_Polarity_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_SetPolarity(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_Polarity_TypeDef ICPolarity)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    MODIFY_REG(PWMAx->CCER,
               (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel]),
               ((uint32_t)ICPolarity << SHIFT_TAB_CCxP[iChannel]));
}

/**
 * @brief  Get the input channel polarity.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval Returned value can be one of PWMA_IC_Polarity_TypeDef values
 */
__STATIC_INLINE PWMA_IC_Polarity_TypeDef PWMA_IC_GetPolarity(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);

    return (PWMA_IC_Polarity_TypeDef)(READ_BIT(PWMAx->CCER, (PWMA_CCER_CC0P << SHIFT_TAB_CCxP[iChannel])) >> SHIFT_TAB_CCxP[iChannel]);
}

/**
 * @brief  Connect the PWMAx_CH0, CH1 and CH2 pins to the TI0 input (XOR combination).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_EnableXORCombination(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->CR2, PWMA_CR2_TI0S);
}

/**
 * @brief  Disconnect the PWMAx_CH0, CH1 and CH2 pins to the TI0 input (XOR combination).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_IC_DisableXORCombination(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->CR2, PWMA_CR2_TI0S);
}

/**
 * @brief  Indicates whether the PWMAx_CH0, CH1 and CH2 pins are connectected to the TI0 input.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IC_IsEnabledXORCombination(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->CR2, PWMA_CR2_TI0S) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Get captured value for input channel.
 * @param  PWMAx Timer instance
 * @param  Channel This parameter can be one of PWMA_CHx values.
 * @retval CapturedValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_IC_GetCapture(PWMA_TypeDef *PWMAx, uint32_t Channel)
{
    uint8_t iChannel = PWMA_GET_CHANNEL_INDEX(Channel);
    __IO uint32_t *pReg = (__IO uint32_t*)((uint32_t)(&PWMAx->CCR0) + ((iChannel >> 1) << 2));

    return (uint16_t)READ_REG(*pReg);
}

/**
 * @brief  Get captured value for input channel 0.
 * @param  PWMAx Timer instance
 * @retval CapturedValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_IC_GetCaptureCH0(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR0));
}

/**
 * @brief  Get captured value for input channel 1.
 * @param  PWMAx Timer instance
 * @retval CapturedValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_IC_GetCaptureCH1(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR1));
}

/**
 * @brief  Get captured value for input channel 2.
 * @param  PWMAx Timer instance
 * @retval CapturedValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_IC_GetCaptureCH2(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR2));
}

/**
 * @brief  Get captured value for input channel 3.
 * @param  PWMAx Timer instance
 * @retval CapturedValue (between Min_Data=0 and Max_Data=65535)
 */
__STATIC_INLINE uint16_t PWMA_IC_GetCaptureCH3(PWMA_TypeDef *PWMAx)
{
    return (uint16_t)(READ_REG(PWMAx->CCR3));
}

/**
 * @brief  Enable external clock mode 2.
 * @note When external clock mode 2 is enabled the counter is clocked by any active edge on the ETRF signal.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableExternalClock(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->SMCR, PWMA_SMCR_ECE);
}

/**
 * @brief  Disable external clock mode 2.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE void PWMA_DisableExternalClock(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SMCR, PWMA_SMCR_ECE);
}

/**
 * @brief  Indicate whether external clock mode 2 is enabled.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledExternalClock(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SMCR, PWMA_SMCR_ECE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Set the clock source of the counter.
 * @note   When selected clock source is external clock mode 1, the timer input
 *         the external clock is applied is selected by calling the @ref PWMA_SetTriggerInput()
 *         function. This timer input must be configured by calling
 *         the @ref PWMA_IC_Config() function.
 * @param  PWMAx Timer instance
 * @param  ClockSource This parameter can be one of PWMA_ClockSource_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_SetClockSource(PWMA_TypeDef *PWMAx, PWMA_ClockSource_TypeDef ClockSource)
{
    MODIFY_REG(PWMAx->SMCR, (PWMA_SMCR_SMS | PWMA_SMCR_ECE), (uint32_t)ClockSource);
}

/**
 * @brief  Set the trigger output (TRGO) used for timer synchronization.
 * @param  PWMAx Timer instance
 * @param  TriggerOutput This parameter can be one of PWMA_TRGO_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_SetTriggerOutput(PWMA_TypeDef *PWMAx, PWMA_TRGO_TypeDef TriggerOutput)
{
    MODIFY_REG(PWMAx->CR2, PWMA_CR2_MMS, (uint32_t)TriggerOutput);
}

/**
 * @brief  Get the trigger output (TRGO) used for timer synchronization.
 * @param  PWMAx Timer instance
 * @retval The trigger output. Can be one of PWMA_TRGO_TypeDef values.
 */
__STATIC_INLINE PWMA_TRGO_TypeDef PWMA_GetTriggerOutput(PWMA_TypeDef *PWMAx)
{
    return (PWMA_TRGO_TypeDef)READ_BIT(PWMAx->CR2, PWMA_CR2_MMS);
}

/**
 * @brief  Set the synchronization mode of a slave timer.
 * @param  PWMAx Timer instance
 * @param  SlaveMode This parameter can be one of PWMA_SlaveMode_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_SetSlaveMode(PWMA_TypeDef *PWMAx, PWMA_SlaveMode_TypeDef SlaveMode)
{
    MODIFY_REG(PWMAx->SMCR, PWMA_SMCR_SMS, (uint32_t)SlaveMode);
}

/**
 * @brief  Set the trigger input to be used to synchronize the counter.
 * @param  PWMAx Timer instance
 * @param  TriggerInput This parameter can be one of PWMA_TS_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_SetTriggerInput(PWMA_TypeDef *PWMAx, PWMA_TS_TypeDef TriggerInput)
{
    MODIFY_REG(PWMAx->SMCR, PWMA_SMCR_TS, (uint32_t)TriggerInput);
}

/**
 * @brief  Get the trigger input to be used to synchronize the counter.
 * @param  PWMAx Timer instance
 * @retval The trigger input. Can be one of PWMA_TS_TypeDef values.
 */
__STATIC_INLINE PWMA_TS_TypeDef PWMA_GetTriggerInput(PWMA_TypeDef *PWMAx)
{
    return (PWMA_TS_TypeDef)READ_BIT(PWMAx->SMCR, PWMA_SMCR_TS);
}

/**
 * @brief  Configure the external trigger (ETR) input.
 * @param  PWMAx Timer instance
 * @param  PWMA_ETR_InitStruct The pointer to the @ref PWMA_ETR_InitStruct_TypeDef structure.
 *                            The next parameters will be configured:
 *                            - ETRPolarity
 *                            - ETRPrescaler
 *                            - ETRFilter
 * @retval None
 */
__STATIC_INLINE void PWMA_ETR_Config(PWMA_TypeDef *PWMAx, PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct)
{
    MODIFY_REG(PWMAx->SMCR,
               (PWMA_SMCR_ETP | PWMA_SMCR_ETPS | PWMA_SMCR_ETF),
               ((uint32_t)PWMA_ETR_InitStruct->ETRPolarity |
                (uint32_t)PWMA_ETR_InitStruct->ETRPrescaler |
                (uint32_t)PWMA_ETR_InitStruct->ETRFilter));
}

/**
 * @brief  Set the prescaler of the external trigger signal.
 * @param  PWMAx Timer instance
 * @param  ETRPrescaler This parameter can be one of PWMA_ETR_Prescaler_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_ETR_SetPrescaler(PWMA_TypeDef *PWMAx, PWMA_ETR_Prescaler_TypeDef ETRPrescaler)
{
    MODIFY_REG(PWMAx->SMCR, PWMA_SMCR_ETPS, (uint32_t)ETRPrescaler);
}

/**
 * @brief  Get the prescaler of the external trigger signal.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_ETR_Prescaler_TypeDef values
 */
__STATIC_INLINE PWMA_ETR_Prescaler_TypeDef PWMA_ETR_GetPrescaler(PWMA_TypeDef *PWMAx)
{
    return (PWMA_ETR_Prescaler_TypeDef)READ_BIT(PWMAx->SMCR, PWMA_SMCR_ETPS);
}

/**
 * @brief  Set the external trigger signal filter duration.
 * @param  PWMAx Timer instance
 * @param  ETRFilter This parameter can be one of PWMA_ETR_Filter_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_ETR_SetFilter(PWMA_TypeDef *PWMAx, PWMA_ETR_Filter_TypeDef ETRFilter)
{
    MODIFY_REG(PWMAx->SMCR, PWMA_SMCR_ETF, (uint32_t)ETRFilter);
}

/**
 * @brief  Get the external trigger signal filter duration.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_ETR_Filter_TypeDef values
 */
__STATIC_INLINE PWMA_ETR_Filter_TypeDef PWMA_ETR_GetFilter(PWMA_TypeDef *PWMAx)
{
    return (PWMA_ETR_Filter_TypeDef)READ_BIT(PWMAx->SMCR, PWMA_SMCR_ETF);
}

/**
 * @brief  Set the external trigger signal polarity.
 * @param  PWMAx Timer instance
 * @param  ETRPolarity This parameter can be one of PWMA_ETR_Polarity_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_ETR_SetPolarity(PWMA_TypeDef *PWMAx, PWMA_ETR_Polarity_TypeDef ETRPolarity)
{
    MODIFY_REG(PWMAx->SMCR, PWMA_SMCR_ETP, (uint32_t)ETRPolarity);
}

/**
 * @brief  Get the external trigger signal polarity.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_ETR_Polarity_TypeDef values
 */
__STATIC_INLINE PWMA_ETR_Polarity_TypeDef PWMA_ETR_GetPolarity(PWMA_TypeDef *PWMAx)
{
    return (PWMA_ETR_Polarity_TypeDef)READ_BIT(PWMAx->SMCR, PWMA_SMCR_ETP);
}

/**
 * @brief  Enable the break function.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableBreak(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->BDTR, PWMA_BDTR_BKE);
}

/**
 * @brief  Disable the break function.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableBreak(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->BDTR, PWMA_BDTR_BKE);
}

/**
 * @brief  Indicate whether break function is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledBreak(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->BDTR, PWMA_BDTR_BKE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Configure the break input (BRK) and dead time.
 * @param  PWMAx Timer instance
 * @param  PWMA_BDTR_InitStruct The pointer to the @ref PWMA_BDTR_InitStruct_TypeDef structure.
 *                            The next parameters will be configured:
 *                            - BreakPolarity
 * @retval None
 */
__STATIC_INLINE void PWMA_BDTR_Config(PWMA_TypeDef *PWMAx, PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct)
{
    MODIFY_REG(PWMAx->BDTR, PWMA_BDTR_BKP, (uint32_t)PWMA_BDTR_InitStruct->BreakPolarity);
}

/**
 * @brief  Set break input (BRK) polarity.
 * @param  PWMAx Timer instance
 * @param  BreakPolarity This parameter can be one of PWMA_BRK_Polarity_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_SetBreakPolarity(PWMA_TypeDef *PWMAx, PWMA_BRK_Polarity_TypeDef BreakPolarity)
{
    MODIFY_REG(PWMAx->BDTR, PWMA_BDTR_BKP, (uint32_t)BreakPolarity);
}

/**
 * @brief  Get break input (BRK) polarity.
 * @param  PWMAx Timer instance
 * @retval Returned value can be one of PWMA_BRK_Polarity_TypeDef values
 */
__STATIC_INLINE PWMA_BRK_Polarity_TypeDef PWMA_GetBreakPolarity(PWMA_TypeDef *PWMAx)
{
    return (PWMA_BRK_Polarity_TypeDef)READ_BIT(PWMAx->BDTR, PWMA_BDTR_BKP);
}

/**
 * @brief  Set the outputs off state (enabled v.s. disabled) in Run mode.
 * @param  PWMAx Timer instance
 * @param  OffStateRun This parameter can be one of PWMA_OSSR_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_SetOffStates(PWMA_TypeDef *PWMAx, PWMA_OSSR_TypeDef OffStateRun)
{
    MODIFY_REG(PWMAx->BDTR, PWMA_BDTR_OSSR, (uint32_t)OffStateRun);
}

/**
 * @brief  Get the outputs off state (enabled v.s. disabled) in Run mode.
 * @param  PWMAx Timer instance
 * @retval The outputs off state. This parameter can be one of PWMA_OSSR_TypeDef values.
 */
__STATIC_INLINE PWMA_OSSR_TypeDef PWMA_GetOffStates(PWMA_TypeDef *PWMAx)
{
    return (PWMA_OSSR_TypeDef)READ_BIT(PWMAx->BDTR, PWMA_BDTR_OSSR);
}

/**
 * @brief  Enable automatic output (MOE can be set by software or automatically when a break input is active).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableAutomaticOutput(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->BDTR, PWMA_BDTR_AOE);
}

/**
 * @brief Disable automatic output (MOE can be set only by software).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableAutomaticOutput(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->BDTR, PWMA_BDTR_AOE);
}

/**
 * @brief  Indicate whether automatic output is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledAutomaticOutput(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->BDTR, PWMA_BDTR_AOE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable the outputs (set the MOE bit in PWMAx_BDTR register).
 * @note   The MOE bit in PWMAx_BDTR register allows to enable/disable the outputs by
 *         software and is reset in the case of break event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableAllOutputs(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->BDTR, PWMA_BDTR_MOE);
}

/**
 * @brief  Disable the outputs (reset the MOE bit in PWMAx_BDTR register).
 * @note   The MOE bit in PWMAx_BDTR register allows to enable/disable the outputs by
 *         software and is reset in the case of break event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableAllOutputs(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->BDTR, PWMA_BDTR_MOE);
}

/**
 * @brief  Indicates whether outputs are enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledAllOutputs(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->BDTR, PWMA_BDTR_MOE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Configures the timer DMA burst.
 * @param  PWMAx Timer instance
 * @param  DMABurstBaseAddress This parameter can be one of PWMA_DMABurst_BaseAddress_TypeDef values
 * @param  DMABurstLength This parameter can be one of PWMA_DMABurst_Length_TypeDef values
 * @retval None
 */
__STATIC_INLINE void PWMA_DMA_Config(PWMA_TypeDef *PWMAx,
                                     PWMA_DMABurst_BaseAddress_TypeDef DMABurstBaseAddress,
                                     PWMA_DMABurst_Length_TypeDef DMABurstLength)
{
    MODIFY_REG(PWMAx->DCR,
               (PWMA_DCR_DBL | PWMA_DCR_DBA),
               ((uint32_t)DMABurstBaseAddress | 
                (uint32_t)DMABurstLength));
}

/**
 * @brief  Indicate whether a flag is set.
 * @param  PWMAx The PWMA instance.
 * @param  Flag The flag. Can be one of PWMA_Flag_TypeDef values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag(PWMA_TypeDef *PWMAx, PWMA_Flag_TypeDef Flag)
{
    return ((READ_BIT(PWMAx->SR, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief  Clear a flag.
 * @param  PWMAx The PWMA instance.
 * @param  Flag The flag. Can be one of PWMA_Flag_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag(PWMA_TypeDef *PWMAx, PWMA_Flag_TypeDef Flag)
{
    CLEAR_BIT(PWMAx->SR, (1UL << Flag));
}

/**
 * @brief  Indicate whether update interrupt flag (UIF) is set (update interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_UPDATE(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_UIF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the update interrupt flag (UIF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_UPDATE(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_UIF);
}

/**
 * @brief  Indicate whether Capture/Compare 0 interrupt flag (CC0IF) is set (Capture/Compare 0 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC0(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC0IF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 0 interrupt flag (CC0IF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC0(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC0IF);
}

/**
 * @brief  Indicate whether Capture/Compare 1 interrupt flag (CC1IF) is set (Capture/Compare 1 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC1(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC1IF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 1 interrupt flag (CC1IF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC1(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC1IF);
}

/**
 * @brief  Indicate whether Capture/Compare 2 interrupt flag (CC2IF) is set (Capture/Compare 2 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC2(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC2IF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 2 interrupt flag (CC2IF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC2(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC2IF);
}

/**
 * @brief  Indicate whether Capture/Compare 3 interrupt flag (CC3IF) is set (Capture/Compare 3 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC3(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC3IF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 3 interrupt flag (CC3IF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC3(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC3IF);
}

/**
 * @brief  Indicate whether commutation interrupt flag (COMIF) is set (commutation interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_COM(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_COMIF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the commutation interrupt flag (COMIF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_COM(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_COMIF);
}

/**
 * @brief  Indicate whether trigger interrupt flag (TIF) is set (trigger interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_TRIG(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_TIF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the trigger interrupt flag (TIF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_TRIG(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_TIF);
}

/**
 * @brief  Indicate whether break interrupt flag (BIF) is set (break interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_BRK(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_BIF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the break interrupt flag (BIF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_BRK(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_BIF);
}

/**
 * @brief  Indicate whether Capture/Compare 0 overcapture interrupt flag (CC0OF) is set (Capture/Compare 0 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC0OVR(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC0OF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 0 overcapture interrupt flag (CC0OF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC0OVR(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC0OF);
}

/**
 * @brief  Indicate whether Capture/Compare 1 overcapture interrupt flag (CC1OF) is set (Capture/Compare 1 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC1OVR(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC1OF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 1 overcapture interrupt flag (CC1OF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC1OVR(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC1OF);
}

/**
 * @brief  Indicate whether Capture/Compare 2 overcapture interrupt flag (CC2OF) is set (Capture/Compare 2 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC2OVR(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC2OF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 2 overcapture interrupt flag (CC2OF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC2OVR(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC2OF);
}

/**
 * @brief  Indicate whether Capture/Compare 3 overcapture interrupt flag (CC3OF) is set (Capture/Compare 3 interrupt is pending).
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsActiveFlag_CC3OVR(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->SR, PWMA_SR_CC3OF) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Clear the Capture/Compare 3 overcapture interrupt flag (CC3OF).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearFlag_CC3OVR(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->SR, PWMA_SR_CC3OF);
}

/**
 * @brief  Enable an interrupt.
 * @param  PWMAx Timer instance
 * @param  IT The interrupt source. Can be one of PWMA_IT_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT(PWMA_TypeDef *PWMAx, PWMA_IT_TypeDef IT)
{
    SET_BIT(PWMAx->DIER, (1UL << IT));
}

/**
 * @brief  Disable an interrupt.
 * @param  PWMAx Timer instance
 * @param  IT The interrupt source. Can be one of PWMA_IT_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT(PWMA_TypeDef *PWMAx, PWMA_IT_TypeDef IT)
{
    CLEAR_BIT(PWMAx->DIER, (1UL << IT));
}

/**
 * @brief  Indicates whether an interrupt is enabled.
 * @param  PWMAx Timer instance
 * @param  IT The interrupt source. Can be one of PWMA_IT_TypeDef values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT(PWMA_TypeDef *PWMAx, PWMA_IT_TypeDef IT)
{
    return ((READ_BIT(PWMAx->DIER, (1UL << IT)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief  Enable update interrupt (UIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_UPDATE(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_UIE);
}

/**
 * @brief  Disable update interrupt (UIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_UPDATE(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_UIE);
}

/**
 * @brief  Indicates whether the update interrupt (UIE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_UPDATE(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_UIE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 0 interrupt (CC0IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_CC0(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC0IE);
}

/**
 * @brief  Disable capture/compare 0 interrupt (CC0IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_CC0(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC0IE);
}

/**
 * @brief  Indicates whether the capture/compare 0 interrupt (CC0IE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_CC0(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC0IE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 1 interrupt (CC1IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_CC1(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC1IE);
}

/**
 * @brief  Disable capture/compare 1 interrupt (CC1IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_CC1(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC1IE);
}

/**
 * @brief  Indicates whether the capture/compare 1 interrupt (CC1IE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_CC1(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC1IE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 2 interrupt (CC2IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_CC2(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC2IE);
}

/**
 * @brief  Disable capture/compare 2 interrupt (CC2IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_CC2(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC2IE);
}

/**
 * @brief  Indicates whether the capture/compare 2 interrupt (CC2IE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_CC2(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC2IE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 3 interrupt (CC3IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_CC3(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC3IE);
}

/**
 * @brief  Disable capture/compare 3 interrupt (CC3IE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_CC3(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC3IE);
}

/**
 * @brief  Indicates whether the capture/compare 3 interrupt (CC3IE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_CC3(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC3IE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable commutation interrupt (COMIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_COM(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_COMIE);
}

/**
 * @brief  Disable commutation interrupt (COMIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_COM(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_COMIE);
}

/**
 * @brief  Indicates whether the commutation interrupt (COMIE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_COM(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_COMIE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable trigger interrupt (TIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_TRIG(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_TIE);
}

/**
 * @brief  Disable trigger interrupt (TIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_TRIG(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_TIE);
}

/**
 * @brief  Indicates whether the trigger interrupt (TIE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_TRIG(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_TIE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable break interrupt (BIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableIT_BRK(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_BIE);
}

/**
 * @brief  Disable break interrupt (BIE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableIT_BRK(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_BIE);
}

/**
 * @brief  Indicates whether the break interrupt (BIE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledIT_BRK(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_BIE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Interrupt clear (INTR_CLEAR).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_ClearIT(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_INTR_CLEAR);
}

/**
 * @brief  Enable a DMA request.
 * @param  PWMAx Timer instance
 * @param  DMAReq The DMA request source. Can be one of PWMA_DMAReq_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq(PWMA_TypeDef *PWMAx, PWMA_DMAReq_TypeDef DMAReq)
{
    SET_BIT(PWMAx->DIER, (1UL << DMAReq));
}

/**
 * @brief  Disable a DMA request.
 * @param  PWMAx Timer instance
 * @param  DMAReq The DMA request source. Can be one of PWMA_DMAReq_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq(PWMA_TypeDef *PWMAx, PWMA_DMAReq_TypeDef DMAReq)
{
    CLEAR_BIT(PWMAx->DIER, (1UL << DMAReq));
}

/**
 * @brief  Indicates whether a DMA request is enabled.
 * @param  PWMAx Timer instance
 * @param  DMAReq The DMA request source. Can be one of PWMA_DMAReq_TypeDef values.
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq(PWMA_TypeDef *PWMAx, PWMA_DMAReq_TypeDef DMAReq)
{
    return ((READ_BIT(PWMAx->DIER, (1UL << DMAReq)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief  Enable update DMA request (UDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_UPDATE(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_UDE);
}

/**
 * @brief  Disable update DMA request (UDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_UPDATE(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_UDE);
}

/**
 * @brief  Indicates whether the update DMA request (UDE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_UPDATE(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_UDE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 0 DMA request (CC0DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_CC0(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC0DE);
}

/**
 * @brief  Disable capture/compare 0 DMA request (CC0DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_CC0(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC0DE);
}

/**
 * @brief  Indicates whether the capture/compare 0 DMA request (CC0DE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_CC0(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC0DE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 1 DMA request (CC1DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_CC1(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC1DE);
}

/**
 * @brief  Disable capture/compare 1 DMA request (CC1DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_CC1(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC1DE);
}

/**
 * @brief  Indicates whether the capture/compare 1 DMA request (CC1DE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_CC1(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC1DE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 2 DMA request (CC2DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_CC2(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC2DE);
}

/**
 * @brief  Disable capture/compare 2 DMA request (CC2DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_CC2(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC2DE);
}

/**
 * @brief  Indicates whether the capture/compare 2 DMA request (CC2DE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_CC2(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC2DE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable capture/compare 3 DMA request (CC3DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_CC3(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_CC3DE);
}

/**
 * @brief  Disable capture/compare 3 DMA request (CC3DE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_CC3(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_CC3DE);
}

/**
 * @brief  Indicates whether the capture/compare 3 DMA request (CC3DE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_CC3(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_CC3DE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable commutation DMA request (COMDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_COM(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_COMDE);
}

/**
 * @brief  Disable commutation DMA request (COMDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_COM(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_COMDE);
}

/**
 * @brief  Indicates whether the commutation DMA request (COMDE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_COM(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_COMDE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Enable trigger interrupt (TDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_EnableDMAReq_TRIG(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->DIER, PWMA_DIER_TDE);
}

/**
 * @brief  Disable trigger interrupt (TDE).
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_DisableDMAReq_TRIG(PWMA_TypeDef *PWMAx)
{
    CLEAR_BIT(PWMAx->DIER, PWMA_DIER_TDE);
}

/**
 * @brief  Indicates whether the trigger interrupt (TDE) is enabled.
 * @param  PWMAx Timer instance
 * @retval State of bit (1 or 0).
 */
__STATIC_INLINE uint32_t PWMA_IsEnabledDMAReq_TRIG(PWMA_TypeDef *PWMAx)
{
    return (READ_BIT(PWMAx->DIER, PWMA_DIER_TDE) != 0x0UL) ? 1UL : 0UL;
}

/**
 * @brief  Generate an event.
 * @param  PWMAx Timer instance
 * @param  Event The event. Can be one of PWMA_Event_TypeDef values.
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent(PWMA_TypeDef *PWMAx, PWMA_Event_TypeDef Event)
{
    SET_BIT(PWMAx->EGR, (1UL << Event));
}

/**
 * @brief  Generate Update event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_UPDATE(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_UG);
}

/**
 * @brief  Generate Capture/Compare 0 event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_CC0(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_CC0G);
}

/**
 * @brief  Generate Capture/Compare 1 event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_CC1(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_CC1G);
}

/**
 * @brief  Generate Capture/Compare 2 event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_CC2(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_CC2G);
}

/**
 * @brief  Generate Capture/Compare 3 event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_CC3(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_CC3G);
}

/**
 * @brief  Generate Commutation event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_COM(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_COMG);
}

/**
 * @brief  Generate Trigger event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_TRIG(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_TG);
}

/**
 * @brief  Generate Break event.
 * @param  PWMAx Timer instance
 * @retval None
 */
__STATIC_INLINE void PWMA_GenerateEvent_BRK(PWMA_TypeDef *PWMAx)
{
    SET_BIT(PWMAx->EGR, PWMA_EGR_BG);
}

ErrorStatus PWMA_DeInit(PWMA_TypeDef *PWMAx);
ErrorStatus PWMA_Init(PWMA_TypeDef *PWMAx, PWMA_InitStruct_TypeDef *PWMA_InitStruct);
void PWMA_StructInit(PWMA_InitStruct_TypeDef *PWMA_InitStruct);

ErrorStatus PWMA_OC_Init(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);
void PWMA_OC_StructInit(PWMA_OC_InitStruct_TypeDef *PWMA_OC_InitStruct);

ErrorStatus PWMA_IC_Init(PWMA_TypeDef *PWMAx, uint32_t Channel, PWMA_IC_InitStruct_TypeDef *PWMA_IC_InitStruct);
void PWMA_IC_StructInit(PWMA_IC_InitStruct_TypeDef *PWMA_ICInitStruct);

ErrorStatus PWMA_ETR_Init(PWMA_TypeDef *PWMAx, PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct);
void PWMA_ETR_StructInit(PWMA_ETR_InitStruct_TypeDef *PWMA_ETR_InitStruct);

ErrorStatus PWMA_BDTR_Init(PWMA_TypeDef *PWMAx, PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct);
void PWMA_BDTR_StructInit(PWMA_BDTR_InitStruct_TypeDef *PWMA_BDTR_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_PWMA_H */

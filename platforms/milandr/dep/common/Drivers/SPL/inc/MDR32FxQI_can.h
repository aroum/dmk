/**
  ******************************************************************************
  * @file    MDR32FxQI_can.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/03/2024
  * @brief   This file contains all the functions prototypes for the CAN
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
#ifndef __MDR32FxQI_CAN_H
#define __MDR32FxQI_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxIQ Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)

/** @addtogroup CAN
  * @{
  */

/** @defgroup CAN_Exported_Types CAN Exported Types
  * @{
  */

/**
  * @brief CAN Propagation time
  */
typedef enum
{
    CAN_PSEG_Mul_1TQ = (((uint32_t)0x0) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 1 x TQ */
    CAN_PSEG_Mul_2TQ = (((uint32_t)0x1) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 2 x TQ */
    CAN_PSEG_Mul_3TQ = (((uint32_t)0x2) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 3 x TQ */
    CAN_PSEG_Mul_4TQ = (((uint32_t)0x3) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 4 x TQ */
    CAN_PSEG_Mul_5TQ = (((uint32_t)0x4) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 5 x TQ */
    CAN_PSEG_Mul_6TQ = (((uint32_t)0x5) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 6 x TQ */
    CAN_PSEG_Mul_7TQ = (((uint32_t)0x6) << CAN_BITTMNG_PSEG_Pos), /*!< Propagation time = 7 x TQ */
    CAN_PSEG_Mul_8TQ = (((uint32_t)0x7) << CAN_BITTMNG_PSEG_Pos)  /*!< Propagation time = 8 x TQ */
} CAN_Propagation_Time;

#define IS_CAN_PSEG_VALUE(VALUE)  (((VALUE) == CAN_PSEG_Mul_1TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_2TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_3TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_4TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_5TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_6TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_7TQ) || \
                                   ((VALUE) == CAN_PSEG_Mul_8TQ))

/**
  * @brief CAN Phase Segment 1 time
  */
typedef enum
{
    CAN_SEG1_Mul_1TQ = (((uint32_t)0x0) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 1 x TQ */
    CAN_SEG1_Mul_2TQ = (((uint32_t)0x1) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 2 x TQ */
    CAN_SEG1_Mul_3TQ = (((uint32_t)0x2) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 3 x TQ */
    CAN_SEG1_Mul_4TQ = (((uint32_t)0x3) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 4 x TQ */
    CAN_SEG1_Mul_5TQ = (((uint32_t)0x4) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 5 x TQ */
    CAN_SEG1_Mul_6TQ = (((uint32_t)0x5) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 6 x TQ */
    CAN_SEG1_Mul_7TQ = (((uint32_t)0x6) << CAN_BITTMNG_SEG1_Pos), /*!< Phase Segment 1 time = 7 x TQ */
    CAN_SEG1_Mul_8TQ = (((uint32_t)0x7) << CAN_BITTMNG_SEG1_Pos)  /*!< Phase Segment 1 time = 8 x TQ */
} CAN_Phase_Seg1_Time;

#define IS_CAN_SEG1_VALUE(VALUE)  (((VALUE) == CAN_SEG1_Mul_1TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_2TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_3TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_4TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_5TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_6TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_7TQ) || \
                                   ((VALUE) == CAN_SEG1_Mul_8TQ))

/**
  * @brief CAN Phase Segment 2 time
  */
typedef enum
{
    CAN_SEG2_Mul_1TQ = (((uint32_t)0x0) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 1 x TQ */
    CAN_SEG2_Mul_2TQ = (((uint32_t)0x1) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 2 x TQ */
    CAN_SEG2_Mul_3TQ = (((uint32_t)0x2) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 3 x TQ */
    CAN_SEG2_Mul_4TQ = (((uint32_t)0x3) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 4 x TQ */
    CAN_SEG2_Mul_5TQ = (((uint32_t)0x4) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 5 x TQ */
    CAN_SEG2_Mul_6TQ = (((uint32_t)0x5) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 6 x TQ */
    CAN_SEG2_Mul_7TQ = (((uint32_t)0x6) << CAN_BITTMNG_SEG2_Pos), /*!< Phase Segment 2 time = 7 x TQ */
    CAN_SEG2_Mul_8TQ = (((uint32_t)0x7) << CAN_BITTMNG_SEG2_Pos)  /*!< Phase Segment 2 time = 8 x TQ */
} CAN_Phase_Seg2_Time;

#define IS_CAN_SEG2_VALUE(VALUE)  (((VALUE) == CAN_SEG2_Mul_1TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_2TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_3TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_4TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_5TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_6TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_7TQ) || \
                                   ((VALUE) == CAN_SEG2_Mul_8TQ))

/**
  * @brief CAN Synchronizations jump width time
  */
typedef enum
{
    CAN_SJW_Mul_1TQ = (((uint32_t)0x0) << CAN_BITTMNG_SJW_Pos), /*!< Synchronization jump width time = 1 x TQ */
    CAN_SJW_Mul_2TQ = (((uint32_t)0x1) << CAN_BITTMNG_SJW_Pos), /*!< Synchronization jump width time = 2 x TQ */
    CAN_SJW_Mul_3TQ = (((uint32_t)0x2) << CAN_BITTMNG_SJW_Pos), /*!< Synchronization jump width time = 3 x TQ */
    CAN_SJW_Mul_4TQ = (((uint32_t)0x3) << CAN_BITTMNG_SJW_Pos)  /*!< Synchronization jump width time = 4 x TQ */
} CAN_SJW_Time;

#define IS_CAN_SJW_VALUE(VALUE)    (((VALUE) == CAN_SJW_Mul_1TQ) || \
                                    ((VALUE) == CAN_SJW_Mul_2TQ) || \
                                    ((VALUE) == CAN_SJW_Mul_3TQ) || \
                                    ((VALUE) == CAN_SJW_Mul_4TQ))

/**
  * @brief CAN Sampling mode
  */
typedef enum
{
    CAN_SB_1_SAMPLE = (((uint32_t)0x0) << CAN_BITTMNG_SB_Pos), /*!< One sample is enough */
    CAN_SB_3_SAMPLE = (((uint32_t)0x1) << CAN_BITTMNG_SB_Pos)  /*!< Three samples should be obtained */
} CAN_Sampling_Mode;

#define IS_CAN_SAMPLING_MODE(MODE) (((MODE) == CAN_SB_1_SAMPLE) || \
                                    ((MODE) == CAN_SB_3_SAMPLE))

/**
  * @brief CAN Identifier Type
  */
typedef enum
{
    CAN_ID_STD = ((uint8_t)0x0), /*!< Standard Id */
    CAN_ID_EXT = ((uint8_t)0x1)  /*!< Extended Id */
} CAN_Identifier_Type;

#define IS_CAN_IDTYPE(IDTYPE) (((IDTYPE) == CAN_ID_STD) || ((IDTYPE) == CAN_ID_EXT))

/**
  * @brief CAN Interrupts
  */
typedef enum
{
    CAN_IT_GLBINTEN     = (((uint32_t)0x1) <<  0), /*!< General flag that enables CAN module interrupts */
    CAN_IT_RXINTEN      = (((uint32_t)0x1) <<  1), /*!< Flag that enables interrupt on message reception */
    CAN_IT_TXINTEN      = (((uint32_t)0x1) <<  2), /*!< Flag that enables interrupt if transmission becomes available */
    CAN_IT_ERRINTEN     = (((uint32_t)0x1) <<  3), /*!< Flag that enables interrupt on error */
    CAN_IT_ERROVERINTEN = (((uint32_t)0x1) <<  4)  /*!< Flag that enables interrupt on TEC or REC exceeds ERROR_MAX value */
} CAN_IT_Def;

#define CAN_IT_MASK     (CAN_IT_GLBINTEN | CAN_IT_RXINTEN | CAN_IT_TXINTEN | \
                         CAN_IT_ERRINTEN | CAN_IT_ERROVERINTEN)

#define IS_CAN_ITConfig(IT)  (((IT) & (~CAN_IT_MASK)) == 0)

/**
  * @brief CAN Clock BRG
  */
typedef enum
{
    CAN_HCLKdiv1   = ((uint32_t)0x00),
    CAN_HCLKdiv2   = ((uint32_t)0x01),
    CAN_HCLKdiv4   = ((uint32_t)0x02),
    CAN_HCLKdiv8   = ((uint32_t)0x03),
    CAN_HCLKdiv16  = ((uint32_t)0x04),
    CAN_HCLKdiv32  = ((uint32_t)0x05),
    CAN_HCLKdiv64  = ((uint32_t)0x06),
    CAN_HCLKdiv128 = ((uint32_t)0x07)
} CAN_Clock_BRG;

#define IS_CAN_CLOCK_BRG(BRG)      ((BRG) <= 7)

/**
  * @brief CAN init structure definition
  */
typedef struct
{
    FunctionalState      CAN_ROP;            /*!< Enables or disables own packets reception.
                                                  This parameter can be a value of @ref FunctionalState */
    FunctionalState      CAN_SAP;            /*!< Enables or disables sending ACK on own packets.
                                                  This parameter can be a value of @ref FunctionalState */
    FunctionalState      CAN_STM;            /*!< Enables or disables Self Test mode.
                                                  This parameter can be a value of @ref FunctionalState */
    FunctionalState      CAN_ROM;            /*!< Enables or disables Read Only mode.
                                                  This parameter can be a value of @ref FunctionalState */
    CAN_Propagation_Time CAN_PSEG;           /*!< Specifies the Propagation time.
                                                  This parameter is one of @ref CAN_Propagation_Time values. */
    CAN_Phase_Seg1_Time  CAN_SEG1;           /*!< Specifies the Phase Segment 1 time.
                                                  This parameter is one of @ref CAN_Phase_Seg1_Time values. */
    CAN_Phase_Seg2_Time  CAN_SEG2;           /*!< Specifies the Phase Segment 2 time.
                                                  This parameter is one of @ref CAN_Phase_Seg2_Time values. */
    CAN_SJW_Time         CAN_SJW;            /*!< Specifies the Synchronizations jump width time.
                                                  This parameter is one of @ref CAN_SJW_Time values. */
    CAN_Sampling_Mode    CAN_SB;             /*!< Specifies the Sampling mode.
                                                  This parameter is one of @ref CAN_Sampling_Mode */
    uint16_t             CAN_BRP;            /*!< Specifies the System Clock Prescaler. Used to calculate Time Quanta
                                                  by the following algorithm:
                                                  CLK = PCLK/(BRP + 1)
                                                  TQ(microsec) = (BRP + 1)/CLK(MHz) */
    uint8_t              CAN_OVER_ERROR_MAX; /*!< Specifies the upper limit for TEC / REC error counters before
                                                  ERROR_OVER flag is set. */
} CAN_InitTypeDef;

/**
  * @brief CAN Buffer Data container definition
  */
typedef uint32_t CAN_DataTypeDef[2];

/**
  * @brief CAN Transmitted message structure definition
  */
typedef struct
{
    uint32_t ID;          /*!< Specifies the full (Standard or Extended) identifier.
                               To set Standard ID, the macro CAN_STDID_TO_EXTID should
                               be used. To get Standard ID use CAN_EXTID_TO_STDID macro.
                               @ref CAN_ID_conversion
                               This parameter can be a value between 0 to 0x1FFFFFFF. */
    uint8_t PRIOR_0;      /*!< Specifies sending priority.
                               This parameter can be a value of @ref FunctionalState. */
    uint8_t IDE;          /*!< Specifies the type of identifier for the message that will be transmitted.
                               This parameter can be a value of @ref CAN_Identifier_Type */
    uint8_t DLC;          /*!< Specifies the length of the frame that will be transmitted.
                               This parameter can be a value between 0 to 8 */
    CAN_DataTypeDef Data; /*!< Contains the data to be transmitted. */
} CAN_TxMsgTypeDef;

/**
  * @brief CAN Remote Transfer Request structure definition
  */
typedef struct
{
    uint32_t ID;     /*!< Specifies the full (Standard or Extended) identifier.
                          To set Standard ID, the macro CAN_STDID_TO_EXTID should
                          be used. To get Standard ID use CAN_EXTID_TO_STDID macro.
                          @ref CAN_ID_conversion
                          This parameter can be a value between 0 to 0x1FFFFFFF. */
    uint8_t PRIOR_0; /*!< Specifies sending priority.
                          This parameter can be a value of @ref FunctionalState. */
    uint8_t IDE;     /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_Identifier_Type */
} CAN_RTRMessageTypeDef;

/**
  * @brief CAN Remote Transfer Request Reply structure definition
  */
typedef struct
{
    uint32_t ID;          /*!< Specifies the full (Standard or Extended) identifier.
                               To set Standard ID, the macro CAN_STDID_TO_EXTID should
                               be used. To get Standard ID use CAN_EXTID_TO_STDID macro.
                               @ref CAN_ID_conversion
                               This parameter can be a value between 0 to 0x1FFFFFFF. */
    uint8_t IDE;          /*!< Specifies the type of identifier for the message that will be transmitted.
                               This parameter can be a value of @ref CAN_Identifier_Type */
    uint8_t DLC;          /*!< Specifies the length of the frame that will be transmitted.
                               This parameter can be a value between 0 to 8 */
    CAN_DataTypeDef Data; /*!< Contains the data to be transmitted. */
} CAN_RTRReplyTypeDef;

/**
  * @brief CAN Received message header structure definition
  */
typedef struct
{
    uint32_t ID;     /*!< Specifies the full (Standard or Extended) identifier.
                          To set Standard ID, the macro CAN_STDID_TO_EXTID should
                          be used. To get Standard ID use CAN_EXTID_TO_STDID macro.
                          @ref CAN_ID_conversion
                          This parameter can be a value between 0 to 0x1FFFFFFF. */
    uint8_t OVER_EN; /*!< Specifies if the received message could be rewritten.
                          This parameter can be a value of FunctionalState. */
    uint8_t IDE;     /*!< Specifies the type of identifier for the message that will be received.
                          This parameter can be a value of @ref CAN_Identifier_Type */
    uint8_t DLC;     /*!< Specifies the length of the frame that will be transmitted.
                          This parameter can be a value between 0 to 8 */
} CAN_RxMsgHeaderTypeDef;

/**
  * @brief  CAN Received message structure definition
  */
typedef struct
{
    CAN_RxMsgHeaderTypeDef Rx_Header;  /*!< Contains message header. */
    CAN_DataTypeDef Data;              /*!< Contains received data. */
} CAN_RxMsgTypeDef;

/**
  * @brief  CAN filter init structure definition
  */
typedef struct
{
    uint32_t Filter_ID; /*!< Specifies standard identifier for the filter.
                             This parameter can be a value between 0 to 0x1FFFFFFF. */
    uint32_t Mask_ID;   /*!< Specifies standard identifier for the mask.
                             This parameter can be a value between 0 to 0x1FFFFFFF. */
} CAN_FilterInitTypeDef;

/** @} */ /* End of group CAN_Exported_Types */


/** @defgroup CAN_Exported_Constants CAN Exported Constants
  * @{
  */

/** @defgroup CAN_Status_Flags CAN Status Flags
  * @{
  */

/*!< Flags */
#if 0
/* Note: this group is under #if 0 and is introduced for informational purpose
 * only. These flags are defined in the header file corresponding to SFR
 * level in CAN_STATUS group */
#define CAN_STATUS_RX_READY         (((uint32_t)0x1) <<  0) /*!< Flag indicating that there are messages received */
#define CAN_STATUS_TX_READY         (((uint32_t)0x1) <<  1) /*!< Flag indicating that there are buffers for transmitting */
#define CAN_STATUS_ERROR_OVER       (((uint32_t)0x1) <<  2) /*!< Flag indicating that TEC or REC exceeds ERROR_MAX value */
#define CAN_STATUS_BIT_ERR          (((uint32_t)0x1) <<  3) /*!< Transmitting frame bits error flag */
#define CAN_STATUS_BIT_STUFF_ERR    (((uint32_t)0x1) <<  4) /*!< Staff frame bits error flag */
#define CAN_STATUS_CRC_ERR          (((uint32_t)0x1) <<  5) /*!< Frame CRC error flag */
#define CAN_STATUS_FRAME_ERR        (((uint32_t)0x1) <<  6) /*!< Frame format error flag */
#define CAN_STATUS_ACK_ERR          (((uint32_t)0x1) <<  7) /*!< Reception acknowledge error flag */
#define CAN_STATUS_IDLOWER          (((uint32_t)0x1) <<  8) /*!< Arbitrage "loosing" flag */
#define CAN_STATUS_ERR_PASSIVE      (((uint32_t)0x1) <<  9) /*!< Flag indicating that CAN controller is in the ERROR PASSIVE state */
#define CAN_STATUS_ERR_BUSOFF       (((uint32_t)0x1) << 10) /*!< Flag indicating that CAN controller is in the BUS OFF state */
#define CAN_STATUS_RX_ERR_CNT8      (((uint32_t)0x1) << 11) /*!< REC (receiver error counter) flag. Set if REC > 255 */
#define CAN_STATUS_TX_ERR_CNT8      (((uint32_t)0x1) << 12) /*!< TEC (transmitter error counter) flag. Set if TEC > 255 */
#endif /* #if 0 */

#define IS_CAN_IT_ERROR_FLAG(FLAG)  (((FLAG) == CAN_STATUS_ERROR_OVER)    || \
                                     ((FLAG) == CAN_STATUS_BIT_ERR)       || \
                                     ((FLAG) == CAN_STATUS_BIT_STUFF_ERR) || \
                                     ((FLAG) == CAN_STATUS_CRC_ERR)       || \
                                     ((FLAG) == CAN_STATUS_FRAME_ERR)     || \
                                     ((FLAG) == CAN_STATUS_ACK_ERR))

#define IS_CAN_IT_RXTX_FLAG(FLAG)   (((FLAG) == CAN_STATUS_RX_READY)  || \
                                     ((FLAG) == CAN_STATUS_TX_READY))

/*!< CAN controller ERROR ACTIVE state mask */
#define CAN_STATUS_ERRACTIVE(STATUS)  !((STATUS) & (CAN_STATUS_ERR_PASSIVE | CAN_STATUS_ERR_BUSOFF))

/* Error counters */
#define CAN_STATUS_TXERRCNT(STATUS) ((uint32_t)((STATUS) & CAN_STATUS_TX_ERR_CNT_Msk) >> CAN_STATUS_TX_ERR_CNT_Pos)
#define CAN_STATUS_RXERRCNT(STATUS) ((uint32_t)((STATUS) & CAN_STATUS_RX_ERR_CNT_Msk) >> CAN_STATUS_RX_ERR_CNT_Pos)

/** @} */ /* End of group CAN_Status_Flags */

/** @defgroup CAN_remote_transmission_request CAN Remote Transmission Request
  * @{
  */

#define CAN_RTR_DATA                ((uint8_t)0x0)                       /*!< Data frame */
#define CAN_RTR_REMOTE              (((uint32_t)0x1) << CAN_DLC_RTR_Pos) /*!< Remote frame */

#define IS_CAN_RTR(RTR)             (((RTR) == CAN_RTR_DATA) || ((RTR) == CAN_RTR_REMOTE))

/** @} */ /* End of group CAN_remote_transmission_request */

/** @defgroup CAN_Message CAN Message-related macros
  * @{
  */

#define IS_CAN_ID(ID)               ((ID) <= ((uint32_t)0x1FFFFFFF))
#define IS_CAN_DLC(DLC)             ((DLC) <= ((uint8_t)0x08))
#define IS_CAN_BUFFER(NUMBER)       ((NUMBER) < ((uint32_t)0x20))
/** @} */ /* End of group CAN_Message */

/** @defgroup CAN_receive_request CAN Receive Request
  * @{
  */

#define CAN_DLC_DATA_LENGTH         ((uint32_t)0xF)  /*!< Mask for message data length field */

/** @} */ /* End of group CAN_receive_request */

/** @defgroup CAN_Constants CAN Constants
  * @{
  */

#define CAN_BUFFER_NUMBER           ((uint32_t)32)          /*!< Total buffer number */

/** @} */ /* End of group CAN_Constants */

/** @defgroup CAN_Buffer_Control_Bit_Masks CAN Buffer Control Bit Masks
  * @{
  */

#define CAN_STATUS_EN              (((uint32_t)0x1) <<  0) /*!< Buffer enable state.
                                                                0 - off, 1 - on */
#define CAN_STATUS_RX_TXn          (((uint32_t)0x1) <<  1) /*!< Buffer operation mode.
                                                                0 - transmission, 1 - reception */
#define CAN_STATUS_OVER_EN         (((uint32_t)0x1) <<  2) /*!< Is received message overwriting allowed.
                                                                0 - not allowed, 1 - allowed */
#define CAN_STATUS_RTR_EN          (((uint32_t)0x1) <<  3) /*!< RTR answer mode
                                                                0 - don't answer, 1 - answer when RTR is received into buffer */
#define CAN_STATUS_PRIOR_0         (((uint32_t)0x1) <<  4) /*!< Sending priority
                                                                0 - with priority, 1 - without priority */
#define CAN_STATUS_TX_REQ          (((uint32_t)0x1) <<  5) /*!< Message sending request
                                                                0 - no request or already sent, 1 - request pending */
/* Flags */
#define CAN_STATUS_RX_FULL         (((uint32_t)0x1) <<  6) /*!< Reception readiness flag */
#define CAN_STATUS_OVER_WR         (((uint32_t)0x1) <<  7) /*!< Flag indicating that received message was overwritten */

/** @} */ /* End of group CAN_Buffer_Control_Bit_Masks */

/** @defgroup CAN_Buffer_Interrupts CAN Buffer Interrupts
  * @{
  */

#define CAN_BUFFER_0               (((uint32_t)0x1) <<  0)
#define CAN_BUFFER_1               (((uint32_t)0x1) <<  1)
#define CAN_BUFFER_2               (((uint32_t)0x1) <<  2)
#define CAN_BUFFER_3               (((uint32_t)0x1) <<  3)
#define CAN_BUFFER_4               (((uint32_t)0x1) <<  4)
#define CAN_BUFFER_5               (((uint32_t)0x1) <<  5)
#define CAN_BUFFER_6               (((uint32_t)0x1) <<  6)
#define CAN_BUFFER_7               (((uint32_t)0x1) <<  7)
#define CAN_BUFFER_8               (((uint32_t)0x1) <<  8)
#define CAN_BUFFER_9               (((uint32_t)0x1) <<  9)
#define CAN_BUFFER_10              (((uint32_t)0x1) << 10)
#define CAN_BUFFER_11              (((uint32_t)0x1) << 11)
#define CAN_BUFFER_12              (((uint32_t)0x1) << 12)
#define CAN_BUFFER_13              (((uint32_t)0x1) << 13)
#define CAN_BUFFER_14              (((uint32_t)0x1) << 14)
#define CAN_BUFFER_15              (((uint32_t)0x1) << 15)
#define CAN_BUFFER_16              (((uint32_t)0x1) << 16)
#define CAN_BUFFER_17              (((uint32_t)0x1) << 17)
#define CAN_BUFFER_18              (((uint32_t)0x1) << 18)
#define CAN_BUFFER_19              (((uint32_t)0x1) << 19)
#define CAN_BUFFER_20              (((uint32_t)0x1) << 20)
#define CAN_BUFFER_21              (((uint32_t)0x1) << 21)
#define CAN_BUFFER_22              (((uint32_t)0x1) << 22)
#define CAN_BUFFER_23              (((uint32_t)0x1) << 23)
#define CAN_BUFFER_24              (((uint32_t)0x1) << 24)
#define CAN_BUFFER_25              (((uint32_t)0x1) << 25)
#define CAN_BUFFER_26              (((uint32_t)0x1) << 26)
#define CAN_BUFFER_27              (((uint32_t)0x1) << 27)
#define CAN_BUFFER_28              (((uint32_t)0x1) << 28)
#define CAN_BUFFER_29              (((uint32_t)0x1) << 29)
#define CAN_BUFFER_30              (((uint32_t)0x1) << 30)
#define CAN_BUFFER_31              (((uint32_t)0x1) << 31)

/** @} */ /* End of group CAN_Buffer_Interrupts */

/** CANx_BUF_xx_DLC Register masks to determine packet format */
#define CAN_BUF_DLC_STD   ((uint32_t)0x00000A00)    /* Standard format */
#define CAN_BUF_DLC_EXT   ((uint32_t)0x00001A00)    /* Extended format */

/** @} */ /* End of group CAN_Exported_Constants */


/** @defgroup CAN_Exported_Macros CAN Exported Macros
  * @{
  */

/** @defgroup CAN_ID_conversion CAN Standard to/from Extended ID conversion
  * @{
  */

#define CAN_STDID_TO_EXTID(STDID)  ((STDID) << CAN_ID_SID_Pos)
#define CAN_EXTID_TO_STDID(EXTID)  ((EXTID) >> CAN_ID_SID_Pos)

/** @} */ /* End of group CAN_ID_conversion */

/** @} */ /* End of group CAN_Exported_Macros */


/** @defgroup CAN_Exported_Functions CAN Exported Functions
  * @{
  */

void CAN_DeInit(MDR_CAN_TypeDef* CANx);
void CAN_Init(MDR_CAN_TypeDef* CANx, const CAN_InitTypeDef* CAN_InitStruct);
void CAN_StructInit(CAN_InitTypeDef* CAN_InitStruct);
void CAN_FilterInit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_FilterInitTypeDef* CAN_FilterInitStruct);
void CAN_ITConfig(MDR_CAN_TypeDef* CANx, uint32_t CAN_IT, FunctionalState NewState);
uint32_t CAN_GetDisabledBuffer(MDR_CAN_TypeDef* CANx);
uint32_t CAN_GetEmptyTransferBuffer(MDR_CAN_TypeDef* CANx);
void CAN_Transmit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_TxMsgTypeDef* TxMessage);
void CAN_SendRTR(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RTRMessageTypeDef* RTRMessage);
void CAN_RTRReply(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RTRReplyTypeDef* RTRReplyMessage);
void CAN_Receive(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, FunctionalState OverWrite);
void CAN_GetReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_DataTypeDef RxBuffer);
void CAN_GetRawReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RxMsgTypeDef* RxMessage);
void CAN_BufferRelease(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber);
void CAN_Cmd(MDR_CAN_TypeDef* CANx, FunctionalState NewState);
uint32_t CAN_GetStatus(MDR_CAN_TypeDef* CANx);
uint32_t CAN_GetBufferStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber);
ITStatus CAN_GetITState(MDR_CAN_TypeDef* CANx, CAN_IT_Def CAN_IT);
void CAN_ITClearErrorPendingBit(MDR_CAN_TypeDef* CANx, uint32_t Status_Flag);
void CAN_RxITConfig(MDR_CAN_TypeDef* CANx, uint32_t Buffer_IT, FunctionalState NewState);
ITStatus CAN_GetRxITStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber);
uint32_t CAN_GetRx(MDR_CAN_TypeDef* CANx);
void CAN_TxITConfig(MDR_CAN_TypeDef* CANx, uint32_t Buffer_IT, FunctionalState NewState);
ITStatus CAN_GetTxITStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber);
void CAN_ITClearRxTxPendingBit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, uint32_t Status_Flag);

void CAN_BRGInit(MDR_CAN_TypeDef* CANx, CAN_Clock_BRG CAN_BRG);


/** @} */ /* End of group CAN_Exported_Functions */

/** @} */ /* End of group CAN */

#endif /* #if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_CAN_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_can.h */



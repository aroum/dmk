/**
 * *****************************************************************************
 *  @file       bmcu_canfd.h
 *  @author     Baikal electronics SDK team
 *  @brief      Controller Area Network Flexible Data-Rate (CAN-FD)
 *              module driver header file
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

#ifndef __BMCU_CANFD_H
#define __BMCU_CANFD_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Tx buffers */
#define CANFD_TXBUF_0                   0x00000001UL   /*!< Tx buffer 0  */
#define CANFD_TXBUF_1                   0x00000002UL   /*!< Tx buffer 1  */
#define CANFD_TXBUF_2                   0x00000004UL   /*!< Tx buffer 2  */
#define CANFD_TXBUF_3                   0x00000008UL   /*!< Tx buffer 3  */
#define CANFD_TXBUF_4                   0x00000010UL   /*!< Tx buffer 4  */
#define CANFD_TXBUF_5                   0x00000020UL   /*!< Tx buffer 5  */
#define CANFD_TXBUF_6                   0x00000040UL   /*!< Tx buffer 6  */
#define CANFD_TXBUF_7                   0x00000080UL   /*!< Tx buffer 7  */
#define CANFD_TXBUF_8                   0x00000100UL   /*!< Tx buffer 8  */
#define CANFD_TXBUF_9                   0x00000200UL   /*!< Tx buffer 9  */
#define CANFD_TXBUF_10                  0x00000400UL   /*!< Tx buffer 10 */
#define CANFD_TXBUF_11                  0x00000800UL   /*!< Tx buffer 11 */
#define CANFD_TXBUF_12                  0x00001000UL   /*!< Tx buffer 12 */
#define CANFD_TXBUF_13                  0x00002000UL   /*!< Tx buffer 13 */
#define CANFD_TXBUF_14                  0x00004000UL   /*!< Tx buffer 14 */
#define CANFD_TXBUF_15                  0x00008000UL   /*!< Tx buffer 15 */
#define CANFD_TXBUF_16                  0x00010000UL   /*!< Tx buffer 16 */
#define CANFD_TXBUF_17                  0x00020000UL   /*!< Tx buffer 17 */
#define CANFD_TXBUF_18                  0x00040000UL   /*!< Tx buffer 18 */
#define CANFD_TXBUF_19                  0x00080000UL   /*!< Tx buffer 19 */
#define CANFD_TXBUF_20                  0x00100000UL   /*!< Tx buffer 20 */
#define CANFD_TXBUF_21                  0x00200000UL   /*!< Tx buffer 21 */
#define CANFD_TXBUF_22                  0x00400000UL   /*!< Tx buffer 22 */
#define CANFD_TXBUF_23                  0x00800000UL   /*!< Tx buffer 23 */
#define CANFD_TXBUF_24                  0x01000000UL   /*!< Tx buffer 24 */
#define CANFD_TXBUF_25                  0x02000000UL   /*!< Tx buffer 25 */
#define CANFD_TXBUF_26                  0x04000000UL   /*!< Tx buffer 26 */
#define CANFD_TXBUF_27                  0x08000000UL   /*!< Tx buffer 27 */
#define CANFD_TXBUF_28                  0x10000000UL   /*!< Tx buffer 28 */
#define CANFD_TXBUF_29                  0x20000000UL   /*!< Tx buffer 29 */
#define CANFD_TXBUF_30                  0x40000000UL   /*!< Tx buffer 30 */
#define CANFD_TXBUF_31                  0x80000000UL   /*!< Tx buffer 31 */

#define CANFD_TXBUF_ALL                 (CANFD_TXBUF_0  | CANFD_TXBUF_1  | CANFD_TXBUF_2  | \
                                         CANFD_TXBUF_3  | CANFD_TXBUF_4  | CANFD_TXBUF_5  | \
                                         CANFD_TXBUF_6  | CANFD_TXBUF_7  | CANFD_TXBUF_8  | \
                                         CANFD_TXBUF_9  | CANFD_TXBUF_10 | CANFD_TXBUF_11 | \
                                         CANFD_TXBUF_12 | CANFD_TXBUF_13 | CANFD_TXBUF_14 | \
                                         CANFD_TXBUF_15 | CANFD_TXBUF_16 | CANFD_TXBUF_17 | \
                                         CANFD_TXBUF_18 | CANFD_TXBUF_19 | CANFD_TXBUF_20 | \
                                         CANFD_TXBUF_21 | CANFD_TXBUF_22 | CANFD_TXBUF_23 | \
                                         CANFD_TXBUF_24 | CANFD_TXBUF_25 | CANFD_TXBUF_26 | \
                                         CANFD_TXBUF_27 | CANFD_TXBUF_28 | CANFD_TXBUF_29 | \
                                         CANFD_TXBUF_30 | CANFD_TXBUF_31)   /*!< All Tx buffers */

#define CANFD_NUM_OF_TX_BUF             ((uint8_t)32U)      /*!< The number of Tx buffers */
#define CANFD_TX_NO_BUFFER              ((uint8_t)0xFFU)    /*!< No empty buffer for message transmission */

/** @brief Transmission status */
typedef enum {
    CANFD_TX_STATUS_OK = 0,     /*!< Transmission succeeded */
    CANFD_TX_STATUS_PENDING     /*!< Transmission pending */
} CANFD_TxStatus_TypeDef;

/** @brief Rx FIFO */
typedef enum {
    CANFD_RX_FIFO0 = 0,     /*!< Rx FIFO-0 */
    CANFD_RX_FIFO1          /*!< Rx FIFO-1 */
} CANFD_RxFIFO_TypeDef;

/** @brief Operating mode */
typedef enum {
    CANFD_OPMODE_CONFIG = 0,    /*!< Configuration mode */
    CANFD_OPMODE_NORMAL,        /*!< Normal mode */
    CANFD_OPMODE_LOOPBACK,      /*!< Loopback mode */
    CANFD_OPMODE_SLEEP,         /*!< Sleep mode */
    CANFD_OPMODE_SNOOP,         /*!< Snoop mode */
    CANFD_OPMODE_PEE,           /*!< Protocol exception mode */
    CANFD_OPMODE_BR             /*!< Bus-off recovery mode */
} CANFD_OpMode_TypeDef;

/** @brief Errors */
typedef enum {
    CANFD_ERROR_CRCER   = 0,    /*!< CRC error */
    CANFD_ERROR_FMER    = 1,    /*!< Form error */
    CANFD_ERROR_STER    = 2,    /*!< Stuff error */
    CANFD_ERROR_BERR    = 3,    /*!< Bit error */
    CANFD_ERROR_ACKER   = 4,    /*!< ACK error */
    CANFD_ERROR_F_CRCER = 8,    /*!< CRC error in data phase */
    CANFD_ERROR_F_FMER  = 9,    /*!< Form error in data phase */
    CANFD_ERROR_F_STER  = 10,   /*!< Stuff error in data phase */
    CANFD_ERROR_F_BERR  = 11    /*!< Bit error in data phase */
} CANFD_Error_TypeDef;

/** @brief Flags */
typedef enum {
    CANFD_FLAG_CONFIG = 0,      /*!< Configuration mode */
    CANFD_FLAG_LBACK  = 1,      /*!< Loopback mode */
    CANFD_FLAG_SLEEP  = 2,      /*!< Sleep mode */
    CANFD_FLAG_NORMAL = 3,      /*!< Normal mode */
    CANFD_FLAG_BIDLE  = 4,      /*!< Bus idle */
    CANFD_FLAG_BBSY   = 5,      /*!< Bus busy */
    CANFD_FLAG_ERRWRN = 6,      /*!< Error warning */
    CANFD_FLAG_PEE    = 9,      /*!< PEE mode */
    CANFD_FLAG_BSFR   = 10,     /*!< Bus-off recovery mode */
    CANFD_FLAG_SNOOP  = 12      /*!< Snoop mode */
} CANFD_Flag_TypeDef;

/** @brief Interrupts */
typedef enum {
    CANFD_IT_ARBLOST    = 0,    /*!< Arbitration lost interrupt */
    CANFD_IT_TXOK       = 1,    /*!< Transmission successful interrupt */
    CANFD_IT_PEE        = 2,    /*!< Protocol exception event interrupt */
    CANFD_IT_BSFRD      = 3,    /*!< Bus-off recovery done interrupt */
    CANFD_IT_RXOK       = 4,    /*!< New message received interrupt */
    CANFD_IT_TSCNT_OFLW = 5,    /*!< Timestamp counter overflow interrupt */
    CANFD_IT_RXFOFLW    = 6,    /*!< Rx FIFO-0 overflow interrupt */
    CANFD_IT_ERROR      = 8,    /*!< Error interrupt */
    CANFD_IT_BSOFF      = 9,    /*!< Bus-off interrupt */
    CANFD_IT_SLP        = 10,   /*!< Sleep interrupt */
    CANFD_IT_WKUP       = 11,   /*!< Wake-up interrupt */
    CANFD_IT_RXFWMFLL   = 12,   /*!< Rx FIFO-0 watermark full interrupt */
    CANFD_IT_TXRRS      = 13,   /*!< Tx buffer ready request served interrupt */
    CANFD_IT_TXCRS      = 14,   /*!< Tx cancellation request served interrupt */
    CANFD_IT_RXFOFLW_1  = 15,   /*!< Rx FIFO-1 overflow interrupt */
    CANFD_IT_RXFWMFLL_1 = 16,   /*!< Rx FIFO-1 watermark full interrupt */
    CANFD_IT_RXMNF      = 17,   /*!< Rx match not finished interrupt */
    CANFD_IT_TXEOFLW    = 30,   /*!< Tx event FIFO overflow interrupt */
    CANFD_IT_TXEWMFLL   = 31    /*!< Tx event FIFO watermark full interrupt */
} CANFD_IT_TypeDef;

/** @brief Identifier Extension */
typedef enum {
    CANFD_IDE_STD = 0,      /*!< Standard identifier */
    CANFD_IDE_EXT           /*!< Extended identifier */
} CANFD_IDE_TypeDef;

/** @brief Extended Data Length */
typedef enum {
    CANFD_EDL_CAN = 0,      /*!< CAN format frame */
    CANFD_EDL_CANFD         /*!< CAN FD format frame */
} CANFD_EDL_TypeDef;

/** @brief Remote Transmission Request */
typedef enum {
    CANFD_RTR_DATA = 0,     /*!< Data frame */
    CANFD_RTR_REMOTE        /*!< Remote frame */
} CANFD_RTR_TypeDef;

/** @brief Bit Rate Switch */
typedef enum {
    CANFD_BRS_DISABLE = 0,  /*!< Bit rate is not switched inside a CAN FD frame */
    CANFD_BRS_ENABLE        /*!< Bit rate is switched inside a CAN FD frame */
} CANFD_BRS_TypeDef;

/** @brief Data Length Code */
typedef enum {
    CANFD_DLC_0BYTES  = 0x0,    /*!< 0 bytes */
    CANFD_DLC_1BYTE   = 0x1,    /*!< 1 byte */
    CANFD_DLC_2BYTES  = 0x2,    /*!< 2 bytes */
    CANFD_DLC_3BYTES  = 0x3,    /*!< 3 bytes */
    CANFD_DLC_4BYTES  = 0x4,    /*!< 4 bytes */
    CANFD_DLC_5BYTES  = 0x5,    /*!< 5 bytes */
    CANFD_DLC_6BYTES  = 0x6,    /*!< 6 bytes */
    CANFD_DLC_7BYTES  = 0x7,    /*!< 7 bytes */
    CANFD_DLC_8BYTES  = 0x8,    /*!< 8 bytes */
    CANFD_DLC_12BYTES = 0x9,    /*!< 12 bytes */
    CANFD_DLC_16BYTES = 0xA,    /*!< 16 bytes */
    CANFD_DLC_20BYTES = 0xB,    /*!< 20 bytes */
    CANFD_DLC_24BYTES = 0xC,    /*!< 24 bytes */
    CANFD_DLC_32BYTES = 0xD,    /*!< 32 bytes */
    CANFD_DLC_48BYTES = 0xE,    /*!< 48 bytes */
    CANFD_DLC_64BYTES = 0xF     /*!< 64 bytes */
} CANFD_DLC_TypeDef;

/** @brief Event FIFO Control */
typedef enum {
    CANFD_EFC_DISABLE = 0,      /*!< Don't store TX events */
    CANFD_EFC_ENABLE            /*!< Store TX events */
} CANFD_EFC_TypeDef;

/** @brief Event Type */
typedef enum {
    CANFD_ET_FORCED      = 1,   /*!< Transmitted in spite of cancellation request or DAR mode transmissions */
    CANFD_ET_TRANSMITTED = 3    /*!< Transmitted */
} CANFD_ET_TypeDef;

/** @brief Error State Indicator */
typedef enum {
    CANFD_ESI_ACTIVE  = 0,      /*!< The frame was sent by the error active transmitter */
    CANFD_ESI_PASSIVE           /*!< The frame was sent by the error passive transmitter */
} CANFD_ESI_TypeDef;

/** @brief CAN Init Structure definition */
typedef struct
{
    CANFD_OpMode_TypeDef Mode;  /*!< Operating mode */
    uint16_t AP_Prescaler;      /*!< Arbitration phase baudrate prescaler */
    uint8_t AP_SJW;             /*!< Arbitration phase Synchronization Jump Width */
    uint16_t AP_TS1;            /*!< Arbitration phase Time Segment 1 */
    uint8_t AP_TS2;             /*!< Arbitration phase Time Segment 2 */
    uint16_t DP_Prescaler;      /*!< Data phase baudrate prescaler */
    uint8_t DP_SJW;             /*!< Data phase Synchronization Jump Width */
    uint8_t DP_TS1;             /*!< Data phase Time Segment 1 */
    uint8_t DP_TS2;             /*!< Data phase Time Segment 2 */
    FunctionalState DP_TDC;     /*!< Data phase Transmitter Delay Compensation enable */
    uint8_t DP_TDCOFF;          /*!< Data phase Transmitter Delay Compensation Offset */
    FunctionalState BRSD;       /*!< Bit Rate Switch disable override */
    FunctionalState DAR;        /*!< Disable auto retransmission */
    FunctionalState DPEE;       /*!< Disable Protocol Exception Event detection */
    FunctionalState ABR;        /*!< Auto bus-off recovery request */
} CANFD_InitStruct_TypeDef;

/** @brief CAN Filter Init Structure definition */
typedef struct
{
    uint8_t Index;              /*!< Filter index */
    FunctionalState Ctrl;       /*!< Filter enable */
    union
    {
        uint32_t StdId;         /*!< Standard frame ID */
        uint32_t ExtId;         /*!< Extended frame ID */
    };
    union
    {
        uint32_t StdIdMask;     /*!< Standard frame ID mask */
        uint32_t ExtIdMask;     /*!< Extended frame ID mask */
    };
    CANFD_IDE_TypeDef IDE;      /*!< Identifier Extension */
    FunctionalState CtrlIDE;    /*!< Identifier Extension enable 
                                     ENABLE: ID mask is applicable to Standard or Extended frame
                                     DISABLE: ID mask is applicable to both Standard and Extended frames */
    CANFD_RTR_TypeDef RTR;      /*!< Remote Transmission Request (RTR bit for Extended frames) */
    FunctionalState CtrlRTR;    /*!< Remote Transmission Request enable */
    CANFD_RTR_TypeDef SRR;      /*!< Substitute Remote Transmission Request (RTR bit for Standard frames) */
    FunctionalState CtrlSRR;    /*!< Substitute Remote Transmission Request enable */
} CANFD_FilterInitStruct_TypeDef;

/** @brief CAN Tx message structure */
typedef struct
{
    union
    {
        uint32_t StdId;     /*!< Standard frame ID */
        uint32_t ExtId;     /*!< Extended frame ID */
    };
    CANFD_IDE_TypeDef IDE;  /*!< Identifier Extension */
    CANFD_EDL_TypeDef EDL;  /*!< Extended Data Length */
    CANFD_RTR_TypeDef RTR;  /*!< Remote Transmission Request */
    CANFD_BRS_TypeDef BRS;  /*!< Bit Rate Switch */
    CANFD_DLC_TypeDef DLC;  /*!< Data Length Code */
    uint8_t Data[64];       /*!< The data to be transmitted */
    CANFD_EFC_TypeDef EFC;  /*!< Event FIFO Control */
    uint8_t MM;             /*!< Message Marker */
} CANFD_TxMsg_TypeDef;

/** @brief CAN Tx event structure */
typedef struct
{
    union
    {
        uint32_t StdId;     /*!< Standard frame ID */
        uint32_t ExtId;     /*!< Extended frame ID */
    };
    CANFD_IDE_TypeDef IDE;  /*!< Identifier Extension */
    CANFD_EDL_TypeDef EDL;  /*!< Extended Data Length */
    CANFD_RTR_TypeDef RTR;  /*!< Remote Transmission Request */
    CANFD_BRS_TypeDef BRS;  /*!< Bit Rate Switch */
    CANFD_DLC_TypeDef DLC;  /*!< Data Length Code */
    CANFD_ET_TypeDef ET;    /*!< Event Type */
    uint8_t MM;             /*!< Message Marker */
    uint16_t TS;            /*!< Timestamp */
} CANFD_TxEvt_TypeDef;

/** @brief CAN Rx message structure */
typedef struct
{
    union
    {
        uint32_t StdId;     /*!< Standard frame ID */
        uint32_t ExtId;     /*!< Extended frame ID */
    };
    CANFD_IDE_TypeDef IDE;  /*!< Identifier Extension */
    CANFD_EDL_TypeDef EDL;  /*!< Extended Data Length */
    CANFD_RTR_TypeDef RTR;  /*!< Remote Transmission Request */
    CANFD_BRS_TypeDef BRS;  /*!< Bit Rate Switch */
    CANFD_DLC_TypeDef DLC;  /*!< Data Length Code */
    CANFD_ESI_TypeDef ESI;  /*!< Error State Indicator */
    uint8_t Data[64];       /*!< The data received */
    uint8_t MFI;            /*!< Matched Filter Index */
    uint16_t TS;            /*!< Timestamp */
} CANFD_RxMsg_TypeDef;

/**
 * @brief Writes a value in CAN register.
 * @param INSTANCE The CAN instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define CANFD_WriteReg(INSTANCE, REG, VALUE)    WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in CAN register.
 * @param INSTANCE The CAN instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define CANFD_ReadReg(INSTANCE, REG)            READ_REG(INSTANCE->REG)

/**
 * @brief Enables the selected CAN instance.
 * @param CANFDx The CAN instance.
 */
__STATIC_INLINE void CANFD_Enable(CANFD_TypeDef *CANFDx)
{
    SET_BIT(CANFDx->SRR, CANFD_SRR_CEN);
}

/**
 * @brief Disables the selected CAN instance.
 * @param CANFDx The CAN instance.
 */
__STATIC_INLINE void CANFD_Disable(CANFD_TypeDef *CANFDx)
{
    CLEAR_BIT(CANFDx->SRR, CANFD_SRR_CEN);
}

/**
 * @brief Checks if the selected CAN instance is enabled.
 * @param CANFDx The CAN instance.
 * @retval 1 if CAN is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CANFD_IsEnabled(CANFD_TypeDef *CANFDx)
{
    return ((READ_BIT(CANFDx->SRR, CANFD_SRR_CEN) == CANFD_SRR_CEN) ? 1UL : 0UL);
}

/**
 * @brief Returns the receive error count.
 * @param CANFDx The CAN instance.
 * @returns The receive error count.
 */
__STATIC_INLINE uint8_t CANFD_GetReceiveErrorCounter(CANFD_TypeDef *CANFDx)
{
    return (uint8_t)(READ_BIT(CANFDx->ECR, CANFD_ECR_REC) >> CANFD_ECR_REC_Pos);
}

/**
 * @brief Returns the transmit error count.
 * @param CANFDx The CAN instance.
 * @returns The transmit error count.
 */
__STATIC_INLINE uint8_t CANFD_GetTransmitErrorCounter(CANFD_TypeDef *CANFDx)
{
    return (uint8_t)(READ_BIT(CANFDx->ECR, CANFD_ECR_TEC) >> CANFD_ECR_TEC_Pos);
}

/**
 * @brief Returns the timestamp counter value.
 * @param CANFDx The CAN instance.
 * @returns The timestamp counter value.
 */
__STATIC_INLINE uint16_t CANFD_GetTimestampCounter(CANFD_TypeDef *CANFDx)
{
    return (uint16_t)(READ_BIT(CANFDx->TSR, CANFD_TSR_TIMESTAMP_CNT) >> CANFD_TSR_TIMESTAMP_CNT_Pos);
}

/**
 * @brief Clears the timestamp counter.
 * @param CANFDx The CAN instance.
 */
__STATIC_INLINE void CANFD_ClearTimestampCounter(CANFD_TypeDef *CANFDx)
{
    SET_BIT(CANFDx->TSR, CANFD_TSR_CTS);
}

/**
 * @brief Checks if error is indicated.
 * @param CANFDx The CAN instance.
 * @param Error The error. Can be one of CANFD_Error_TypeDef values.
 * @retval The state of error (1 or 0).
 */
__STATIC_INLINE uint32_t CANFD_IsActiveError(CANFD_TypeDef *CANFDx, CANFD_Error_TypeDef Error)
{
    return ((READ_BIT(CANFDx->ESR, (1UL << Error)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if flag is set.
 * @param CANFDx The CAN instance.
 * @param Flag The flag. Can be one of CANFD_Flag_TypeDef values.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t CANFD_IsActiveFlag(CANFD_TypeDef *CANFDx, CANFD_Flag_TypeDef Flag)
{
    return ((READ_BIT(CANFDx->SR, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears error.
 * @param CANFDx The CAN instance.
 * @param Error The error. Can be one of CANFD_Error_TypeDef values.
 */
__STATIC_INLINE void CANFD_ClearError(CANFD_TypeDef *CANFDx, CANFD_Error_TypeDef Error)
{
    SET_BIT(CANFDx->ESR, (1UL << Error));
}

/**
 * @brief Enables CAN interrupt.
 * @param CANFDx The CAN instance.
 * @param IT The interrupt source. Can be one of CANFD_IT_TypeDef values.
 */
__STATIC_INLINE void CANFD_EnableIT(CANFD_TypeDef *CANFDx, CANFD_IT_TypeDef IT)
{
    SET_BIT(CANFDx->IER, (1UL << IT));
}

/**
 * @brief Disables CAN interrupt.
 * @param CANFDx The CAN instance.
 * @param IT The interrupt source. Can be one of CANFD_IT_TypeDef values.
 */
__STATIC_INLINE void CANFD_DisableIT(CANFD_TypeDef *CANFDx, CANFD_IT_TypeDef IT)
{
    CLEAR_BIT(CANFDx->IER, (1UL << IT));
}

/**
 * @brief Checks if CAN interrupt is enabled.
 * @param CANFDx The CAN instance.
 * @param IT The interrupt source. Can be one of CANFD_IT_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CANFD_IsEnabledIT(CANFD_TypeDef *CANFDx, CANFD_IT_TypeDef IT)
{
    return ((READ_BIT(CANFDx->IER, (1UL << IT)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears CAN interrupt.
 * @param CANFDx The CAN instance.
 * @param IT The interrupt source. Can be one of CANFD_IT_TypeDef values.
 */
__STATIC_INLINE void CANFD_ClearIT(CANFD_TypeDef *CANFDx, CANFD_IT_TypeDef IT)
{
    SET_BIT(CANFDx->ICR, (1UL << IT));
}

/**
 * @brief Enables Tx buffer ready request served interrupt.
 * @note The function only selects Tx buffers the interrupt will be
 *       generated for. To enable the interrupt set ETXRRS bit in IER
 *       register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 */
__STATIC_INLINE void CANFD_EnableIT_TRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    SET_BIT(CANFDx->IETRS, TxBufferMask);
}

/**
 * @brief Disables Tx buffer ready request served interrupt.
 * @note The function only selects Tx buffers the interrupt will be
 *       generated for. To disable the interrupt clear ETXRRS bit in IER
 *       register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 */
__STATIC_INLINE void CANFD_DisableIT_TRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    CLEAR_BIT(CANFDx->IETRS, TxBufferMask);
}

/**
 * @brief Checks if Tx buffer ready request served interrupt is enabled.
 * @note The function only checks if interrupt is generated for the selected
 *       Tx buffer. The interrupt is controlled by setting of ETXRRS bit
 *       in IER register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 * @retval 1 if all the masked interrupts are enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CANFD_IsEnabledIT_TRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    return ((READ_BIT(CANFDx->IETRS, TxBufferMask) == TxBufferMask) ? 1UL : 0UL);
}

/**
 * @brief Enables Tx buffer cancellation request served interrupt.
 * @note The function only selects Tx buffers the interrupt will be
 *       generated for. To enable the interrupt set ETXCRS bit in IER
 *       register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 */
__STATIC_INLINE void CANFD_EnableIT_CRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    SET_BIT(CANFDx->IETCS, TxBufferMask);
}

/**
 * @brief Disables Tx buffer cancellation request served interrupt.
 * @note The function only selects Tx buffers the interrupt will be
 *       generated for. To disable the interrupt clear ETXCRS bit in IER
 *       register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 */
__STATIC_INLINE void CANFD_DisableIT_CRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    CLEAR_BIT(CANFDx->IETCS, TxBufferMask);
}

/**
 * @brief Checks if Tx buffer cancellation request served interrupt is enabled.
 * @note The function only checks if interrupt is generated for the selected
 *       Tx buffer. The interrupt is controlled by setting of ETXCRS bit
 *       in IER register.
 * @param CANFDx The CAN instance.
 * @param TxBufferMask A bitwise OR combination of CANFD_TXBUF_n.
 * @retval 1 if all the masked interrupts are enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CANFD_IsEnabledIT_CRS(CANFD_TypeDef *CANFDx, uint32_t TxBufferMask)
{
    return ((READ_BIT(CANFDx->IETCS, TxBufferMask) == TxBufferMask) ? 1UL : 0UL);
}

ErrorStatus CANFD_DeInit(CANFD_TypeDef *CANFDx);
ErrorStatus CANFD_Init(CANFD_TypeDef *CANFDx, CANFD_InitStruct_TypeDef *CANFD_InitStruct);
void CANFD_StructInit(CANFD_InitStruct_TypeDef *CANFD_InitStruct);
void CANFD_FilterInit(CANFD_TypeDef *CANFDx, CANFD_FilterInitStruct_TypeDef *CAN_FilterInitStruct);
CANFD_OpMode_TypeDef CANFD_GetMode(CANFD_TypeDef *CANFDx);
void CANFD_RequestMode(CANFD_TypeDef *CANFDx, CANFD_OpMode_TypeDef Mode);
CANFD_DLC_TypeDef CANFD_LENtoDLC(uint8_t Length);
uint8_t CANFD_DLCtoLEN(CANFD_DLC_TypeDef DLC);
uint8_t CANFD_Transmit(CANFD_TypeDef *CANFDx, CANFD_TxMsg_TypeDef *TxMessage);
CANFD_TxStatus_TypeDef CANFD_TransmitStatus(CANFD_TypeDef *CANFDx, uint8_t TransmitBuffer);
void CANFD_CancelTransmit(CANFD_TypeDef *CANFDx, uint8_t TransmitBuffer);
uint8_t CANFD_TXE_FIFO_GetFillLevel(CANFD_TypeDef *CANFDx);
uint8_t CANFD_TXE_FIFO_GetReadIndex(CANFD_TypeDef *CANFDx);
void CANFD_TXE_FIFO_IncReadIndex(CANFD_TypeDef *CANFDx);
void CANFD_TXE_FIFO_SetWatermark(CANFD_TypeDef *CANFDx, uint8_t Level);
uint8_t CANFD_TXE_FIFO_GetWatermark(CANFD_TypeDef *CANFDx);
void CANFD_GetEvent(CANFD_TypeDef *CANFDx, CANFD_TxEvt_TypeDef *TxEvent);
uint8_t CANFD_RX_FIFO_GetFillLevel(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber);
uint8_t CANFD_RX_FIFO_GetReadIndex(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber);
void CANFD_RX_FIFO_IncReadIndex(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber);
void CANFD_RX_FIFO_SetWatermark(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber, uint8_t Level);
uint8_t CANFD_RX_FIFO_GetWatermark(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber);
void CANFD_SetFilterPartition(CANFD_TypeDef *CANFDx, uint8_t Index);
uint8_t CANFD_GetFilterPartition(CANFD_TypeDef *CANFDx);
void CANFD_Receive(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber, CANFD_RxMsg_TypeDef *RxMessage);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_CANFD_H */

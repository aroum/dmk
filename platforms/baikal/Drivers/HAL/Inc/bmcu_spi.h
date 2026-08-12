/**
 * *****************************************************************************
 *  @file       bmcu_spi.h
 *  @author     Baikal electronics SDK team
 *  @brief      Serial Peripheral Interface (SPI) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - SPI related constants, type definitions and enumerations
 *      - Functions for SPI control and status check
 *      - Functions for SPI interrupts control and status check
 *      - SPI module export functions prototype
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMCU_SPI_H
#define __BMCU_SPI_H

/* Includes ------------------------------------------------------------------*/
#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
 * @typedef
 * @brief   Serial Clock Phase modes enumeration
 *          The serial clock phase selects the relationship of the serial
 *          clock with the slave select signal.
 *          When SCPH = 0, data are captured on the first edge of the
 *          serial clock. When SCPH = 1, the serial clock starts toggling
 *          one cycle after the slave select line is activated, and data are
 *          captured on the second edge of the serial clock.
 * @note    SPI_PHASE_MIDDLE: serial clock toggles in middle of first data bit
 *          SPI_PHASE_START:  serial clock toggles at start of first data bit
 * @see     SPI_SetClockPhase (), SPI_GetClockPhase()
 */
typedef enum {
    SPI_PHASE_MIDDLE = SPI_CTRLR0_SCPH_MIDDLE,
    SPI_PHASE_START  = SPI_CTRLR0_SCPH_START
} SPI_Phase_TypeDef;

/**
 * @typedef
 * @brief   Serial Clock Polarity modes enumeration
 *          Used to select the polarity of the inactive serial clock, which
 *          is held inactive when the SPI is not actively
 *          transferring data on the serial bus.
 * @note    SPI_POLARITY_LOW:  inactive state of serial clock is low
 *          SPI_POLARITY_HIGH: inactive state of serial clock is high
 * @see     SPI_SetClockPolarity(), SPI_GetClockPolarity()
 */
typedef enum {
    SPI_POLARITY_LOW  = SPI_CTRLR0_SCPOL_LOW,
    SPI_POLARITY_HIGH = SPI_CTRLR0_SCPOL_HIGH
} SPI_Polarity_TypeDef;

/**
 * @typedef
 * @brief   Transfer Modes enumeration
 *          Selects the mode of transfer for serial communication. This
 *          field does not affect the transfer duplicity. Only indicates
 *          whether the receive or transmit data are valid.
 *          In transmit-only mode, data received from the external
 *          device is not valid and is not stored in the receive FIFO
 *          memory; it is overwritten on the next transfer.
 *          In receive-only mode, transmitted data are not valid. After
 *          the first write to the transmit FIFO, the same word is
 *          retransmitted for the duration of the transfer.
 *          In transmit-and-receive mode, both transmit and receive
 *          data are valid. The transfer continues until the transmit FIFO
 *          is empty. Data received from the external device are stored
 *          into the receive FIFO memory, where it can be accessed by
 *          the host processor.
 *          In eeprom-read mode, receive data is not valid while control
 *          data is being transmitted. When all control data is sent to the
 *          EEPROM, receive data becomes valid and transmit data
 *          becomes invalid. All data in the transmit FIFO is considered
 *          control data in this mode.
 * @note    SPI_XFER_TXRX: transmit & Receive
 *          SPI_XFER_TX: transmit only mode
 *          SPI_XFER_RX: receive only mode
 *          SPI_XFER_EEPROM: EEPROM Read mode
 * @see     SPI_SetTransferMode(), SPI_GetTransferMode()
 */
typedef enum {
    SPI_XFER_TXRX   = SPI_CTRLR0_TMOD_TX_RX,
    SPI_XFER_TX     = SPI_CTRLR0_TMOD_TX,
    SPI_XFER_RX     = SPI_CTRLR0_TMOD_RX,
    SPI_XFER_EEPROM = SPI_CTRLR0_TMOD_EEPROM_READ
} SPI_XferMode_TypeDef;


/**
 * @typedef
 * @brief   Data Frame Size enumeration
 *          Used to select the data frame size in 32-bit transfer mode.
 *          When the data frame size is programmed to be less than 32
 *          bits, the receive data are automatically right-justified by the
 *          receive logic, with the upper bits of the receive FIFO zero-
 *          padded. You are responsible for making sure that transmit
 *          data is right-justified before writing into the transmit FIFO.
 *          The transmit logic ignores the upper unused bits when
 *          transmitting the data.
 * @see     SPI_SetDataFrameSize(), SPI_GetFrameSize()
 */
typedef enum {
    SPI_DATA_FRAME_SIZE_4  = SPI_CTRLR0_DFS_32_04_BIT, /*!< 4-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_5  = SPI_CTRLR0_DFS_32_05_BIT, /*!< 5-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_6  = SPI_CTRLR0_DFS_32_06_BIT, /*!< 6-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_7  = SPI_CTRLR0_DFS_32_07_BIT, /*!< 7-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_8  = SPI_CTRLR0_DFS_32_08_BIT, /*!< 8-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_9  = SPI_CTRLR0_DFS_32_09_BIT, /*!< 9-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_10 = SPI_CTRLR0_DFS_32_10_BIT, /*!< 10-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_11 = SPI_CTRLR0_DFS_32_11_BIT, /*!< 11-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_12 = SPI_CTRLR0_DFS_32_12_BIT, /*!< 12-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_13 = SPI_CTRLR0_DFS_32_13_BIT, /*!< 13-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_14 = SPI_CTRLR0_DFS_32_14_BIT, /*!< 14-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_15 = SPI_CTRLR0_DFS_32_15_BIT, /*!< 15-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_16 = SPI_CTRLR0_DFS_32_16_BIT, /*!< 16-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_17 = SPI_CTRLR0_DFS_32_17_BIT, /*!< 17-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_18 = SPI_CTRLR0_DFS_32_18_BIT, /*!< 18-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_19 = SPI_CTRLR0_DFS_32_19_BIT, /*!< 19-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_20 = SPI_CTRLR0_DFS_32_20_BIT, /*!< 20-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_21 = SPI_CTRLR0_DFS_32_21_BIT, /*!< 21-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_22 = SPI_CTRLR0_DFS_32_22_BIT, /*!< 22-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_23 = SPI_CTRLR0_DFS_32_23_BIT, /*!< 23-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_24 = SPI_CTRLR0_DFS_32_24_BIT, /*!< 24-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_25 = SPI_CTRLR0_DFS_32_25_BIT, /*!< 25-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_26 = SPI_CTRLR0_DFS_32_26_BIT, /*!< 26-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_27 = SPI_CTRLR0_DFS_32_27_BIT, /*!< 27-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_28 = SPI_CTRLR0_DFS_32_28_BIT, /*!< 28-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_29 = SPI_CTRLR0_DFS_32_29_BIT, /*!< 29-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_30 = SPI_CTRLR0_DFS_32_30_BIT, /*!< 30-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_31 = SPI_CTRLR0_DFS_32_31_BIT, /*!< 31-bit serial data transfer */
    SPI_DATA_FRAME_SIZE_32 = SPI_CTRLR0_DFS_32_32_BIT  /*!< 32-bit serial data transfer */
} SPI_DataFrameSize_TypeDef;


/**
 * @typedef
 * @brief   Slave Select Enable Flag modes enumeration
 *          When a value is set (1), slave select line from
 *          the SPI is activated when a serial transfer begins. It
 *          should be noted that setting or clearing the value
 *          have no effect on the slave select outputs until
 *          a transfer is started.
 * @note    SPI_SSEL_NOT_SELECTED : No slave selected
 *          SPI_SSEL_SELECTED:      Slave selected
 * @see     SPI_SetSlaveSelect()
 */
typedef enum {
    SPI_SSEL_NOT_SELECTED = SPI_SER_SER_NOT_SELECTED,
    SPI_SSEL_SELECTED     = SPI_SER_SER_SELECTED
} SPI_SlaveSel_TypeDef;


/**
 * @typedef
 * @brief   SPI status flags enumeratioin
 * @details Flags in the read-only status register used to indicate the current
 *          transfer status, FIFO status, and any transmission/reception errors.
 *          Status flags  may be read at any time.
 *          SPI_FLAG_BUSY: transfer in progress
 *                         set: serial transfer is in progress (actively
 *                         transferring data);
 *                         cleared: the SPI module is idle or disabled
 *          SPI_FLAG_TFNF: Transmit FIFO Not Full
 *                         set: transmit FIFO contains >= 1 empty locations
 *                         cleared: the Tx FIFO is full
 *          SPI_FLAG_TFE:  Transmit FIFO Empty
 *                         set: the transmit FIFO is completely empty
 *                         cleared: the transmit FIFO contains one or more
 *                         valid entries
 *          SPI_FLAG_RFNE: Receive FIFO Not Empty
 *                         set: Rx FIFO is not empty
 *                         cleared: Rx FIFO is empty
 *          SPI_FLAG_RFF:  Receive FIFO Full
 *                         set: Rx FIFO is completely full
 *          SPI_FLAG_TXE:  Transmission Error
 *                         Set if the transmit FIFO is empty when a transfer
 *                         is started. This flag can be set only for the slave
 *                         SPI instance. Data from the previous transmission is
 *                         resent on the txd line. This bit is cleared when read
 *                         set: transmission error
 *                         cleared: no Error
 *                         cleared: Rx FIFO contains >= 1 empty location
 *          SPI_FLAG_DCOL: Data Collision Error
 *                         If input slave select signal is asserted by other
 *                         master, when the SPI is in the middle
 *                         the transfer. This informs the processor that the
 *                         last data transfer was halted before completion.
 *                         set: Tx Data Collision error
 *                         clear: no error
 * @see     SPI_IsActiveFlag();
 */
typedef enum {
    SPI_FLAG_BUSY = 0,     /*!< Transfer in progress */
    SPI_FLAG_TFNF = 1,     /*!< Transmit FIFO Not Full */
    SPI_FLAG_TFE  = 2,     /*!< Transmit FIFO Empty */
    SPI_FLAG_RFNE = 3,     /*!< Receive FIFO Not Empty */
    SPI_FLAG_RFF  = 4,     /*!< Receive FIFO Full */
    SPI_FLAG_TXE  = 5,     /*!< Transmission Error */
    SPI_FLAG_DCOL = 6      /*!< Data Collision Error */
} SPI_Flag_TypeDef;


/**
 * @brief   Interrupts types enumeration
 * @details The SPI supports combined interrupt request. Each of individual
 *          interrupt can be masked. The combined interrupt request is the ORed
 *          result of all other SPI interrupts after masking.
 *          SPI_IT_TXE: Transmit FIFO Empty
 *                      Set when the transmit FIFO is equal to or below its
 *                      threshold value and requires service to prevent an
 *                      under-run. The threshold value, set through a
 *                      software-programmable register, determines the level of
 *                      transmit FIFO entries at which an interrupt is
 *                      generated. This interrupt is cleared by hardware when
 *                      data are written into the transmit FIFO buffer,
 *                      bringing it over the threshold level.
 *          SPI_IT_TXO: Transmit FIFO Overflow
 *                      Set when an bus access attempts to write into the
 *                      transmit FIFO after it has been completely filled.
 *                      When set, data written from the bus is discarded. This
 *                      interrupt remains set until you read the transmit FIFO
 *                      overflow interrupt clear register (TXOICR).
 *          SPI_IT_RXU: Receive FIFO Underflow
 *                      Set when an bus access attempts to read from the
 *                      receive FIFO when it is empty. When set, zeros are read
 *                      back from the receive FIFO. This interrupt remains set
 *                      until you read the receive FIFO underflow interrupt
 *                      clear register (RXUICR).
 *          SPI_IT_RXO: Receive FIFO Overflow
 *                      Set when the receive logic attempts to place data into
 *                      the receive FIFO after it has been completely filled.
 *                      When set, newly received data are discarded. This
 *                      interrupt remains set until you read the receive FIFO
 *                      overflow interrupt clear register (RXOICR).
 *          SPI_IT_RXF: Receive FIFO Full
 *                      Set when the receive FIFO is equal to or above its
 *                      threshold value plus 1 and requires service to prevent
 *                      an overflow. The threshold value, set through a
 *                      software-programmable register, determines the level of
 *                      receive FIFO entries at which an interrupt is
 *                      generated. This interrupt is cleared by hardware when
 *                      data are read from the receive FIFO buffer, bringing it
 *                      below the threshold level.
 *          SPI_IT_MST: Multi-Master Contention
 *                      Present on for SPI master instances
 *                      The interrupt is set when another serial master on the
 *                      serial bus selects the SPI master as a serial-slave
 *                      device and is actively transferring data. This informs
 *                      the processor of possible contention on the serial bus.
 *                      This interrupt remains set until you read the
 *                      multi-master interrupt clear register (MSTICR).
 * @see     SPI_EnableIT(), SPI_DisableIT(), SPI_IsEnabledIT(),
 *          SPI_ClearIT(), SPI_IsActiveIT(), SPI_IsActiveITRaw
 */
typedef enum {
    SPI_IT_TXE = 0,        /*!< Transmit FIFO Empty */
    SPI_IT_TXO = 1,        /*!< Transmit FIFO Overflow  */
    SPI_IT_RXU = 2,        /*!< Receive FIFO Underflow */
    SPI_IT_RXO = 3,        /*!< Receive FIFO Overflow */
    SPI_IT_RXF = 4,        /*!< Receive FIFO Full */
    SPI_IT_MST = 5         /*!< Multi-Master Contention */
} SPI_IT_TypeDef;


/**
 * @typedef
 * @brief   SPI initialization structure
 *          Used for initial SPI parameters setup
 * @see     SPI_Init(), SPI_StructInit()
 */
typedef struct {
    uint16_t                  ClkDivider;
    SPI_Polarity_TypeDef      Polarity;
    SPI_Phase_TypeDef         Phase;
    SPI_XferMode_TypeDef      XferMode;
    SPI_DataFrameSize_TypeDef DataFrameSize;
    SPI_SlaveSel_TypeDef      SlaveSel;
    FunctionalState           SlaveSelToggle;
    FunctionalState           EndianConv;
    FunctionalState           ShiftRegLoop;
    uint8_t                   FIFOTxThreshold;
    uint8_t                   FIFORxThreshold;
    uint8_t                   RxSampleDelay;
} SPI_InitStruct_TypeDef;

/* Exported constants --------------------------------------------------------*/
/*!< Max value of data frame size */
#define SPI_XFER_SIZE_MAX           (32U)
/*!< Only even values of SCKDV allowed */
#define SPI_SCKDV_MAX               (0xFFFEU)
/*!< Maximum number of SPI clock cycles that can be used to delay
the sampling of the Rx input */
#define SPI_RX_DELAY_DEPTH_MAX      (4U)
/*!< Receive FIFO buffer depth */
#define SPI_FIFO_DEPTH_RX           (8U)
/*!< Transmit FIFO buffer depth */
#define SPI_FIFO_DEPTH_TX           (64U)
/*!< Receive FIFO threshold max value */
#define SPI_FIFO_TFT_MAX_RX         (SPI_FIFO_DEPTH_RX - 1)
/*!< Transmit FIFO threshold max value */
#define SPI_FIFO_TFT_MAX_TX         (SPI_FIFO_DEPTH_TX - 1)
/*!< DMA Receive Data Level max value */
#define SPI_DMARDL_MAX              (SPI_FIFO_DEPTH_RX - 1)
/*!< DMA Transmit Data Level max value */
#define SPI_DMATDL_MAX              (SPI_FIFO_DEPTH_TX - 1)
/*!< ClkDivider default value */
#define SPI_CLKDIVIDER_DEFAULT      (SPI_SCKDV_MAX / 10)
/*!< Tx FIFO Threshold level default value */
#define SPI_FIFOTXTHRESHOLD_DEFAULT (SPI_FIFO_TFT_MAX_TX / 2)
/*!< Rx FIFO Threshold level default value */
#define SPI_FIFORXTHRESHOLD_DEFAULT (SPI_FIFO_TFT_MAX_RX / 2)
/*!< Rx sample delay default value */
#define SPI_RX_DELAY_DEPTH_DEFAULT   (1U)
/*!< DeInit max attempts value (see SPI_DeInit()) */
#define SPI_DEINIT_RETRY_COUNT (1000U)

/* Exported macro ------------------------------------------------------------*/
/**
  * @brief  Write a value in SPI register
  * @param  __INSTANCE__ SPI Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define SPI_WriteReg(__INSTANCE__, __REG__, __VALUE__) \
           WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in SPI register
  * @param  __INSTANCE__ SPI Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define SPI_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Enable SPI peripheral block
 * @details Enables SPI operations. When SPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the SPI control registers
 *          when enabled.
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_Disable()
 */
__STATIC_INLINE void
SPI_Enable (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->SPIENR, SPI_SPIENR_SPI_EN_Msk, SPI_SPIENR_SPI_EN_ENABLED);
}


/**
 * @brief   Disable SPI peripheral block
 * @param   SPIx The SPI instance
 * @details Disable all SPI operations. When SPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the SPI control registers
 *          when enabled.
 * @retval  None
 * @see     SPI_Enable()
 */
__STATIC_INLINE void
SPI_Disable (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->SPIENR, SPI_SPIENR_SPI_EN_Msk, SPI_SPIENR_SPI_EN_DISABLED);
}


/**
 * @brief   Check if the requested SPI instance is enabled
 * @details When SPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the SPI control registers
 *          when enabled.
 * @param   SPIx The SPI instance
 * @retval  1 if SPI instance is enabled, 0 otherwise
 * @see     SPI_Enable(), SPI_Disable()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabled (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->SPIENR, SPI_SPIENR_SPI_EN_Msk) ==
                                (SPI_SPIENR_SPI_EN_ENABLED)) ? ENABLE : DISABLE;
}


/**
 * @brief   Configure the transfer direction
 * @param   SPIx The SPI instance
 * @param   XferMode transfer mode (direction)
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetTransferMode()
 */
__STATIC_INLINE void
SPI_SetTransferMode (SPI_TypeDef *SPIx,
                           SPI_XferMode_TypeDef XferMode) {
    MODIFY_REG(SPIx->CTRLR0, SPI_CTRLR0_TMOD_Msk, XferMode);
}


/**
 * @brief   Get the configured transfer direction
 * @param   SPIx The SPI instance
 * @retval  configured transfer mode (direction)
 * @see     SPI_SetTransferMode()
 */
__STATIC_INLINE SPI_XferMode_TypeDef
SPI_GetTransferMode (SPI_TypeDef *SPIx) {
    return (SPI_XferMode_TypeDef)(READ_BIT(SPIx->CTRLR0,
                                                         SPI_CTRLR0_TMOD_Msk));
}


/**
 * @brief   Configure the data frame size
 * @param   SPIx The SPI instance
 * @param   DataFrameSize data frame size
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetDataFrameSize()
 */
__STATIC_INLINE void
SPI_SetDataFrameSize (SPI_TypeDef *SPIx, SPI_DataFrameSize_TypeDef DataFrameSize) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_DFS_32_Msk, DataFrameSize);
}


/**
 * @brief   Get the configured data frame size
 * @param   SPIx The SPI instance
 * @retval  Data frame size value
 * @see     SPI_SetDataFrameSize()
 */
__STATIC_INLINE SPI_DataFrameSize_TypeDef
SPI_GetDataFrameSize (SPI_TypeDef *SPIx) {
    return (SPI_DataFrameSize_TypeDef)(READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_DFS_32_Msk));
}


/**
 * @brief   Enable the endianness for data register reads
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_DisableEndianConversion(), SPI_IsEnabledEndianConversion
 */
__STATIC_INLINE void
SPI_EnableEndianConversion (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SECONV_Msk, SPI_CTRLR0_SECONV_ENABLED);
}


/**
 * @brief   Disable the endianness for data register reads
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_EnableEndianConversion(), SPI_IsEnabledEndianConversion
 */
__STATIC_INLINE void
SPI_DisableEndianConversion (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SECONV_Msk, SPI_CTRLR0_SECONV_DISABLED);
}


/**
 * @brief   Check if the endianness for data register reads is enabled
 * @param   SPIx The SPI instance
 * @retval  1 if the endianness for data register reads is enabled,
 *          0 otherwise
 * @see     SPI_EnableEndianConversion(), SPI_DisableEndianConversion()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledEndianConversion (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SECONV_Msk) ==
                                (SPI_CTRLR0_SECONV_ENABLED)) ? ENABLE : DISABLE;
}

/**
 * @brief   Set divider for output serial clock.
 * @param   SPIx The SPI instance
 * @param   ClkDivider Output clock divider value (2..SPI_SCKDV_MAX)
 *          The LSB is always set to 0 and is unafected by a write operation.
 * @note    Serial output clock is disable in case of ClkDivider == 0.
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetClkDivider()
 */

__STATIC_INLINE void
SPI_SetClkDivider (SPI_TypeDef *SPIx, uint16_t ClkDivider) {
    WRITE_REG(SPIx->BAUDR , ClkDivider);
}


/**
 * @brief   Get the clock divider value
 * @param   SPIx The SPI instance
 * @retval  Clock divider value
 * @see     SPI_SetClkDivider()
 */
__STATIC_INLINE uint16_t
SPI_GetClkDivider (SPI_TypeDef *SPIx) {
    return (uint16_t)(READ_REG(SPIx->BAUDR));
}


/**
 * @brief   Configure the clock polarity value
 * @param   SPIx The SPI instance
 * @param   Polarity the new clock polarity value
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetClockPolarity()
 */
__STATIC_INLINE void
SPI_SetClockPolarity (SPI_TypeDef *SPIx, SPI_Polarity_TypeDef Polarity) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SCPOL_Msk, Polarity);
}


/**
 * @brief   Get the clock polarity value
 * @param   SPIx The SPI instance
 * @retval  Clock polarity value
 * @see     SPI_SetClockPolarity()
 */
__STATIC_INLINE SPI_Polarity_TypeDef
SPI_GetClockPolarity (SPI_TypeDef *SPIx) {
    return (SPI_Polarity_TypeDef)(READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SCPOL_Msk));
}


/**
 * @brief   Configure the clock phase value
 * @param   SPIx The SPI instance
 * @param   ClockPhase the new clock phase value
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetClockPhase()
 */
__STATIC_INLINE void
SPI_SetClockPhase (SPI_TypeDef *SPIx, SPI_Phase_TypeDef ClockPhase) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SCPH_Msk, ClockPhase);
}


/**
 * @brief   Get the clock phase value
 * @param   SPIx The SPI instance
 * @retval  Clock phase value
 * @see     SPI_SetClockPhase()
 */
__STATIC_INLINE SPI_Phase_TypeDef
SPI_GetClockPhase (SPI_TypeDef *SPIx) {
    return (SPI_Phase_TypeDef)(READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SCPH_Msk));
}


/**
 * @brief   Enable slave select toggling in SPI mode
  * @details When operating with clock phase set to SPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_DisableSlaveSelToggle(), SPI_IsEnabledSlaveSelToggle
 */
__STATIC_INLINE void
SPI_EnableSlaveSelToggle (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SSTE_Msk, SPI_CTRLR0_SSTE_ENABLED);
}


/**
 * @brief   Disable slave select toggling in SPI mode
  * @details When operating with clock phase set to SPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_EnableSlaveSelToggle(), SPI_IsEnabledSlaveSelToggle
 */
__STATIC_INLINE void
SPI_DisableSlaveSelToggle (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0 , SPI_CTRLR0_SSTE_Msk, SPI_CTRLR0_SSTE_DISABLED);
}


/**
 * @brief   Check if slave select toggling in SPI mode is enabled
  * @details When operating with clock phase set to SPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   SPIx The SPI instance
 * @retval  1 if slave select togglingis enabled, 0 otherwise
 * @see     SPI_EnableSlaveSelToggle(), SPI_DisableSlaveSelToggle()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledSlaveSelToggle (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SSTE_Msk) ==
                                  (SPI_CTRLR0_SSTE_ENABLED)) ? ENABLE : DISABLE;
}


/**
 * @brief   Enable slave OE (output enable)
 * @details Relevant only for SPI Slave instances.
 *          Enable the setting of the OE output from the SPI serial slave.
 *          When OE is disabled, the OE output can never be active. When the OE
 *          output controls the tri-state buffer on the Tx output from the
 *          slave, a high impedance state is always present on the slave
 *          txd output when disabled.
 *          This is useful when the master transmits in broadcast mode
 *          (master transmits data to all slave devices). Only one slave
 *          may respond with data on the master rxd line. OE is
 *          enabled after reset and must be disabled by software (when
 *          broadcast mode is used), if you do not want this device to
 *          respond with data.
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_DisableOE(), SPI_IsEnabledOE()
 * */
__STATIC_INLINE void
SPI_EnableOE (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0, SPI_CTRLR0_SLV_OE_Msk, SPI_CTRLR0_SLV_OE_ENABLED);
}


/**
 * @brief   Disable slave OE (output enable)
 * @details Relevant only for SPI Slave instances.
 *          Disable the setting of the OE output from the SPI serial slave.
 *          When OE is disabled, the OE output can never be active. When the OE
 *          output controls the tri-state buffer on the Tx output from the
 *          slave, a high impedance state is always present on the slave
 *          txd output when disabled.
 *          This is useful when the master transmits in broadcast mode
 *          (master transmits data to all slave devices). Only one slave
 *          may respond with data on the master rxd line. OE is
 *          enabled after reset and must be disabled by software (when
 *          broadcast mode is used), if you do not want this device to
 *          respond with data.
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_EnableOE(), SPI_IsEnabledOE()
 * */
__STATIC_INLINE void
SPI_DisableOE (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0, SPI_CTRLR0_SLV_OE_Msk, SPI_CTRLR0_SLV_OE_DISABLED);
}


/**
 * @brief   Check if slave OE (output enable) is enabled
 * @details Relevant only for SPI Slave instances.
 *          Disable the setting of the OE output from the SPI serial slave.
 *          When OE is disabled, the OE output can never be active. When the OE
 *          output controls the tri-state buffer on the Tx output from the
 *          slave, a high impedance state is always present on the slave
 *          txd output when disabled.
 *          This is useful when the master transmits in broadcast mode
 *          (master transmits data to all slave devices). Only one slave
 *          may respond with data on the master rxd line. OE is
 *          enabled after reset and must be disabled by software (when
 *          broadcast mode is used), if you do not want this device to
 *          respond with data.
 * @param   SPIx The SPI instance
 * @retval  1 if slave OE is enabled, 0 otherwise
 * @see     SPI_EnableOE(), SPI_DisableOE()
 * */
__STATIC_INLINE FunctionalState
SPI_IsEnabledOE (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SLV_OE_Msk) ==
                                (SPI_CTRLR0_SLV_OE_ENABLED)) ? ENABLE : DISABLE;
}


/**
 * @brief   Enable internal Tx-Rx shift registers loop
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_DisableShiftRegLoop(), SPI_IsEnabledShiftRegLoop()
 */
__STATIC_INLINE void
SPI_EnableShiftRegLoop (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0, SPI_CTRLR0_SRL_Msk, SPI_CTRLR0_SRL_TESTING);
}


/**
 * @brief   Disable internal Tx-Rx shift registers loop
 * @param   SPIx The SPI instance
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_EnableShiftRegLoop(), SPI_IsEnabledShiftRegLoop()
 */
__STATIC_INLINE void
SPI_DisableShiftRegLoop (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->CTRLR0, SPI_CTRLR0_SRL_Msk, SPI_CTRLR0_SRL_NORMAL);
}


/**
 * @brief   Check if internal Tx-Rx shift registers loop is enabled
 * @param   SPIx The SPI instance
 * @retval  1 if internal Tx-Rx shift registers loop is enabled, 0 otherwise
 * @see     SPI_EnableShiftRegLoop(), SPI_DisableShiftRegLoop()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledShiftRegLoop (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->CTRLR0, SPI_CTRLR0_SRL_Msk) ==
                                (SPI_CTRLR0_SRL_TESTING)) ? ENABLE : DISABLE;
}


/**
 * @brief   Configure number of data frames to be continuously received
 * @note    The SPI continues to receive serial data until the number of data
 *          frames received is equal to this register value plus 1, which
 *          enables you to receive up to 64KB of data in a
 *          continuous transfer.
 * @param   SPIx The SPI instance
 * @param   NumOfDataFrames number of data frames to be continuously received
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetNumOfDataFrames()
 */
__STATIC_INLINE void
SPI_SetNumOfDataFrames (SPI_TypeDef *SPIx, SPI_DataFrameSize_TypeDef NumOfDataFrames) {
    WRITE_REG(SPIx->CTRLR1, NumOfDataFrames);
}


/**
 * @brief   Get the number of data frames to be continuously received
 * @param   SPIx The SPI instance
 * @retval  number of data frames to be continuously received
 * @see     SPI_SetNumOfDataFrames()
 */
__STATIC_INLINE SPI_DataFrameSize_TypeDef
SPI_GetNumOfDataFrames (SPI_TypeDef *SPIx) {
    return (uint16_t)(READ_REG(SPIx->CTRLR1));
}


/**
 * @brief   Configure slave select line
 * @param   SPIx The SPI instance
 * @param   SlaveSel slave select line
 * @retval  None
 * @see     SPI_GetSlaveSelect()
 */
__STATIC_INLINE void
SPI_SetSlaveSelect (SPI_TypeDef *SPIx, SPI_SlaveSel_TypeDef SlaveSel) {
    MODIFY_REG(SPIx->SER, SPI_SER_SER_Msk, SlaveSel);
}


/**
 * @brief   Get the slave select line
 * @param   SPIx The SPI instance
 * @retval  slave select line
 * @see     SPI_SetSlaveSelect()
 */
__STATIC_INLINE SPI_SlaveSel_TypeDef
SPI_GetSlaveSelect (SPI_TypeDef *SPIx) {
    return (SPI_SlaveSel_TypeDef)(READ_REG(SPIx->SER));
}


/**
 * @brief   Configure the transmit FIFO threshold level
 * @param   SPIx The SPI instance
 * @param   FIFOTxThreshold transmit FIFO threshold level
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetFIFOTxThreshold()
 */
__STATIC_INLINE void
SPI_SetFIFOTxThreshold (SPI_TypeDef *SPIx, uint8_t FIFOTxThreshold) {
    WRITE_REG(SPIx->TXFTLR, FIFOTxThreshold);
}


/**
 * @brief   Get the transmit FIFO threshold level
 * @param   SPIx The SPI instance
 * @retval  Transmit FIFO threshold level
 * @see     SPI_SetFIFOTxThreshold()
 */
__STATIC_INLINE uint8_t
SPI_GetFIFOTxThreshold (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->TXFTLR));
}


/**
 * @brief   Configure the receive FIFO threshold level
 * @param   SPIx The SPI instance
 * @param   FIFORxThreshold receive FIFO threshold level
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetFIFORxThreshold()
 */
__STATIC_INLINE void
SPI_SetFIFORxThreshold (SPI_TypeDef *SPIx, uint8_t FIFORxThreshold) {
    WRITE_REG(SPIx->RXFTLR, FIFORxThreshold);
}


/**
 * @brief   Get the receive FIFO threshold level
 * @param   SPIx The SPI instance
 * @retval  Receive FIFO threshold level
 * @see     SPI_SetFIFORxThreshold()
 */
__STATIC_INLINE uint8_t
SPI_GetFIFORxThreshold (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->RXFTLR));
}


/**
 * @brief   Get the transmit FIFO level
 * @param   SPIx The SPI instance
 * @retval  Transmit FIFO level value
 */
__STATIC_INLINE uint8_t
SPI_GetFIFOTxLvl (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->TXFLR));
}


/**
 * @brief   Get the receive FIFO level
 * @param   SPIx The SPI instance
 * @retval  Receive FIFO level value
 */
__STATIC_INLINE uint8_t
SPI_GetFIFORxLvl (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->RXFLR));
}


/**
 * @brief   Configure RX Sample Delay value
 * @details Control the number of SPI clock cycles that are delayed (from the
 *          default sample time) before the actual sample of the
 *          input signal occurs.  This additional logic can help to increase
 *          the maximum achievable frequency on the bus.
 * @param   SPIx The SPI instance
 * @param   RxSampleDelay RX Sample Delay value (0..SPI_RX_DELAY_DEPTH_MAX)
 * @note    It is incorrect to execute this function when the SPI is enabled
 * @retval  None
 * @see     SPI_GetRxSampleDelay()
 */
__STATIC_INLINE void
SPI_SetRxSampleDelay (SPI_TypeDef *SPIx, uint8_t RxSampleDelay) {
    WRITE_REG(SPIx->RX_SAMPLE_DLY, RxSampleDelay);
}


/**
 * @brief   Get the RX Sample Delay value
 * @param   SPIx The SPI instance
 * @retval  RX Sample Delay value
 * @see     SPI_SetRxSampleDelay()
 */
__STATIC_INLINE uint8_t
SPI_GetRxSampleDelay (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->RX_SAMPLE_DLY));
}


/**
 * @brief   Check if specified SPI status flag is set
 * @param   SPIx The SPI instance
 * @param   Flag Requested flag. Can be one of SPI_Flag_TypeDef values.
 * @retval  1 if the flag was set; 0 if the flag was cleared
 */
__STATIC_INLINE FunctionalState
SPI_IsActiveFlag(SPI_TypeDef *SPIx, SPI_Flag_TypeDef Flag) {
    return (READ_BIT(SPIx->SR, (1UL << Flag)) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable specified interrupt
 * @param   SPIx The SPI instance
 * @param   IT The nterrupt source. Can be one of the SPI_IT_TypeDef values
 * @retval  None
 * @see     SPI_DisableIT(), SPI_IsEnabledIT(),
 *          SPI_ClearIT(), SPI_IsActiveIT(), SPI_IsActiveITRaw
 */
__STATIC_INLINE void
SPI_EnableIT (SPI_TypeDef *SPIx, SPI_IT_TypeDef IT) {
    SET_BIT(SPIx->IMR, (1UL << IT));
}


/**
 * @brief   Disable specified interrupt
 * @param   SPIx The SPI instance
 * @param   IT The nterrupt source. Can be one of the SPI_IT_TypeDef values
 * @retval  None
 * @see     SPI_EnableIT(), SPI_IsEnabledIT(),
 *          SPI_ClearIT(), SPI_IsActiveIT(), SPI_IsActiveITRaw
 */
__STATIC_INLINE void
SPI_DisableIT (SPI_TypeDef *SPIx, SPI_IT_TypeDef IT) {
    CLEAR_BIT(SPIx->IMR, (1UL << IT));
}


/**
 * @brief   Checks if specified interrupt is enabled
 * @param   SPIx The SPI instance
 * @param   IT The interrupt source. Can be one of SPI_IT_TypeDef values
 * @retval  1 if the interrupt is enabled, otherwise 0
 * @see     SPI_EnableIT(), SPI_DisableIT(),
 *          SPI_ClearIT(), SPI_IsActiveIT(), SPI_IsActiveITRaw
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledIT (SPI_TypeDef *SPIx, SPI_IT_TypeDef IT) {
    return (READ_BIT(SPIx->IMR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Clear interrupt
 * @note    Clears all SPI interrupt flags
 *          It is possible to clear SPI_IT_TXO, SPI_IT_RXU, SPI_IT_RXO and
 *          SPI_IT_MST separately be reading TXOICR, RXUICR, RXOICR and MSTICR
 *          registers respectively
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_EnableIT(), SPI_DisableIT(), SPI_IsEnabledIT(),
 *          SPI_IsActiveIT(), SPI_IsActiveITRaw
 */
__STATIC_INLINE void
SPI_ClearIT (SPI_TypeDef *SPIx) {
    (void) READ_BIT(SPIx->ICR, SPI_ICR_ICR_Msk);
}


/**
 * @brief   Check if the specified SPI interrupt is active
 * @param   SPIx The SPI instance
 * @param   IT The interrupt source. Can be one of SPI_IT_TypeDef values
 * @retval  1 if interrupt is active, otherwise 0
 * @see     SPI_EnableIT(), SPI_DisableIT(), SPI_IsEnabledIT(),
 *          SPI_ClearIT(), SPI_IsActiveITRaw
 */
__STATIC_INLINE FunctionalState
SPI_IsActiveIT (SPI_TypeDef *SPIx, SPI_IT_TypeDef IT) {
    return (READ_BIT(SPIx->ISR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Check if the specified SPI interrupt is active (raw status)
 * @param   SPIx The SPI instance
 * @param   IT The interrupt source. Can be one of SPI_IT_TypeDef values
 * @retval  1 if interrupt is active, otherwise 0
 * @see     SPI_EnableIT(), SPI_DisableIT(), SPI_IsEnabledIT(),
 *          SPI_ClearIT(), SPI_IsActiveIT()
 */
__STATIC_INLINE FunctionalState
SPI_IsActiveITRaw (SPI_TypeDef *SPIx, SPI_IT_TypeDef IT) {
    return (READ_BIT(SPIx->RISR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable the DMA Transmit operation
 * @note    Enables the transmit FIFO DMA channel
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_DisableDMATx(), SPI_IsEnabledDMATx
 */
__STATIC_INLINE void
SPI_EnableDMATx (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->DMACR, SPI_DMACR_TDMAE_Msk, SPI_DMACR_TDMAE_ENABLE);
}


/**
 * @brief   Disable the DMA Transmit operation
 * @note    Disables the transmit FIFO DMA channel
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_EnableDMATx(), SPI_IsEnabledDMATx
 */
__STATIC_INLINE void
SPI_DisableDMATx (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->DMACR, SPI_DMACR_TDMAE_Msk, SPI_DMACR_TDMAE_DISABLE);
}


/**
 * @brief   Checks if DMA Receiving is enabled
 * @param   SPIx The SPI instance
 * @retval  1 if DMA Rx is enabled, otherwise 0
 * @see     SPI_EnableDMATx(), SPI_DisableDMATx()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledDMATx (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->DMACR, SPI_DMACR_TDMAE_Msk) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable the DMA Receive operation
 * @note    Enables the receive FIFO DMA channel
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_DisableDMARx(), SPI_IsEnabledDMARx()
 */
__STATIC_INLINE void
SPI_EnableDMARx (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->DMACR, SPI_DMACR_RDMAE_Msk, SPI_DMACR_RDMAE_ENABLE);
}


/**
 * @brief   Disable the DMA Receive operation
 * @note    Disables the receive FIFO DMA channel
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_EnableDMARx(), SPI_IsEnabledDMARx()
 */
__STATIC_INLINE void
SPI_DisableDMARx (SPI_TypeDef *SPIx) {
    MODIFY_REG(SPIx->DMACR, SPI_DMACR_RDMAE_Msk, SPI_DMACR_RDMAE_DISABLE);
}


/**
 * @brief   Checks if DMA Receiving is enabled
 * @param   SPIx The SPI instance
 * @retval  1 if DMA Rx is enabled, otherwise 0
 * @see     SPI_EnableDMARx(), SPI_DisableDMARx()
 */
__STATIC_INLINE FunctionalState
SPI_IsEnabledDMARx (SPI_TypeDef *SPIx) {
    return (READ_BIT(SPIx->DMACR, SPI_DMACR_RDMAE_Msk) ? ENABLE : DISABLE);
}


/**
 * @brief   Set DMA Transmit data level
 * @param   SPIx The SPI instance
 * @param   DataLvl The level at which a DMA request is made by the transmit
 *          logic. It is equal to the watermark level; that is, the DMA Tx
 *          request signal is generated when the number of valid data entries in
 *          the transmit FIFO is equal to or below this value, and SPI DMA Tx
 *          is enabled (SPI->DMACR:TDMAE = 1).
 *          DMA Tx request signal is asserted when DataLvl or less data entries
 *          are present in the transmit FIFO
 *          Range of acceptable values: 0..SPI_DMATDL_MAX
 * @see     SPI_GetDMATxDataLvl()
 */
__STATIC_INLINE void
SPI_SetDMATxDataLvl (SPI_TypeDef *SPIx, uint32_t DataLvl) {
    WRITE_REG(SPIx->DMATDLR, DataLvl);
}


/**
 * @brief   Get DMA Transmit data level
 * @param   SPIx The SPI instance
 * @retval  The level at which a DMA request is made by the transmit
 *          logic. It is equal to the watermark level; that is, the DMA Tx
 *          request signal is generated when the number of valid data entries in
 *          the transmit FIFO is equal to or below this value, and SPI DMA Tx
 *          is enabled (SPI->DMACR:TDMAE = 1).
 *          DMA Tx request signal is asserted when DataLvl or less data entries
 *          are present in the transmit FIFO
 * @see     SPI_SetDMATxDataLvl()
 */
__STATIC_INLINE uint32_t
SPI_GetDMATxDataLvl (SPI_TypeDef *SPIx) {
    return (uint32_t)(READ_REG(SPIx->DMATDLR));
}


/**
 * @brief   Set DMA Receive data level
 * @param   SPIx The SPI instance
 * @param   DataLvl The level at which a DMA request is made by the receive
 *          logic. The watermark level = DataLvl+1; that is, DMA Rx request is
 *          generated when the number of valid data entries in the receive FIFO
 *          is equal to or above this DataLvl value + 1, and SPI DMA Rx is
 *          enabled (SPIx->DMACR:RDMAE=1).
 *          DMA Rx request is asserted when DataLvl or more valid data entries
 *          are present in the receive FIFO
 *          Range of acceptable values: 0..SPI_DMARDL_MAX
 * @see     SPI_GetDMARxDataLvl()
 */
__STATIC_INLINE void
SPI_SetDMARxDataLvl (SPI_TypeDef *SPIx, uint32_t DataLvl) {
    WRITE_REG(SPIx->DMARDLR, DataLvl);
}


/**
 * @brief   Get DMA Receive data level
 * @param   SPIx The SPI instance
 * @retval  The level at which a DMA request is made by the receive
 *          logic. The watermark level = DataLvl+1; that is, DMA Rx request is
 *          generated when the number of valid data entries in the receive FIFO
 *          is equal to or above this DataLvl value + 1, and SPI DMA Rx is
 *          enabled (SPIx->DMACR:RDMAE=1).
 *          DMA Rx request is asserted when DataLvl or more valid data entries
 *          are present in the receive FIFO
 * @see     SPI_SetDMARxDataLvl()
 */
__STATIC_INLINE uint32_t
SPI_GetDMARxDataLvl (SPI_TypeDef *SPIx) {
    return (uint32_t)(READ_REG(SPIx->DMARDLR));
}


/**
 * @brief   Receive 8 bit data from Data Register (Rx FIFO)
 * @param   SPIx The SPI instance
 * @retval  8-bit eceived data
 * @see     SPI_ReceiveData16(), SPI_ReceiveData32()
 */
__STATIC_INLINE uint8_t
SPI_ReceiveData8 (SPI_TypeDef *SPIx) {
    return (uint8_t)(READ_REG(SPIx->DR[0]));
}


/**
 * @brief   Receive 16 bit data from Data Register (Rx FIFO)
 * @param   SPIx The SPI instance
 * @retval  16-bit received data
 * @see     SPI_ReceiveData8(), SPI_ReceiveData32()
 */
__STATIC_INLINE uint16_t
SPI_ReceiveData16 (SPI_TypeDef *SPIx) {
    return (uint16_t)(READ_REG(SPIx->DR[0]));
}


/**
 * @brief   Receive 32 bit data from Data Register (Rx FIFO)
 * @param   SPIx The SPI instance
 * @retval  32-bit received data
 * @see     SPI_ReceiveData8(), SPI_ReceiveData16()
 */
__STATIC_INLINE uint32_t
SPI_ReceiveData32 (SPI_TypeDef *SPIx) {
    return READ_REG(SPIx->DR[0]);
}


/**
 * @brief   Transmit 8-bit datao to Data Register (Tx FIFO)
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_TransmitData16(), SPI_TransmitData32()
 */
__STATIC_INLINE void
SPI_TransmitData8 (SPI_TypeDef *SPIx, uint8_t TxData) {
    *((__IO uint8_t *)&SPIx->DR[0]) = TxData;
}


/**
 * @brief   Transmit 16-bit datao to Data Register (Tx FIFO)
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_TransmitData8(), SPI_TransmitData32()
 */
__STATIC_INLINE void
SPI_TransmitData16 (SPI_TypeDef *SPIx, uint16_t TxData) {
    __IO uint16_t* ptr = (__IO uint16_t *)&SPIx->DR[0];
    *ptr = TxData;
}


/**
 * @brief   Transmit 32-bit datao to Data Register (Tx FIFO)
 * @param   SPIx The SPI instance
 * @retval  None
 * @see     SPI_TransmitData8(), SPI_TransmitData16()
 */
__STATIC_INLINE void
SPI_TransmitData32 (SPI_TypeDef *SPIx, uint32_t TxData) {
    WRITE_REG(SPIx->DR[0], TxData);
}


/* Initialization and de-initialization functions prototypes -----------------*/

/**
 * @brief   Set each field of SPI_InitStruct_TypeDef structure to the default value
 * @param   SPI_InitStruct The pointer to the SPI_InitStruct_TypeDef structure
 * @retval  None
 */
void
SPI_StructInit(SPI_InitStruct_TypeDef *SPI_InitStruct);


/**
 * @brief   Initialize SPI registers according to the specified parameters
 *          of SPI_InitStruct structure
 * @param   SPIx The SPI instance
 * @param   SPI_InitStruct Pointer to struct with SPI configuration parameters
 * @retval  An ErrorStatus type value:
 *              - SUCCESS: SPI was initialized according to SPI_InitStruct
 *                         settings
 *              - ERROR:   SPI is enabled; reconfiguration is not possible
 * @note    SPI configuratioin is possible only when SPI is disabled
 */
ErrorStatus
SPI_Init (SPI_TypeDef *SPIx, SPI_InitStruct_TypeDef *SPI_InitStruct);


/**
 * @brief Denitialize SPI peripheral block
 * @param SPIx           SPI instance
 * @retval An ErrorStatus type value:
 *          - SUCCESS: SPI was deinitialized successfully
 *          - ERROR:   Not applicable
 */
ErrorStatus
SPI_DeInit (SPI_TypeDef *SPIx);

#ifdef __cplusplus
}
#endif

#endif // __BMCU_SPI_H

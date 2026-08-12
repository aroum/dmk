/**
 * *****************************************************************************
 *  @file       bmcu_qspi.h
 *  @author     Baikal electronics SDK team
 *  @brief      Quad SPI (QSPI) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - QSPI related constants, type definitions and enumerations
 *      - Macros for QSPI control and status check
 *      - Macros for QSPI interrupts control and status check
 *      - QSPI module export functions prototype
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMCU_QSPI_H
#define __BMCU_QSPI_H

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
 * @note    QSPI_PHASE_MIDDLE: serial clock toggles in middle of first data bit
 *          QSPI_PHASE_START:  serial clock toggles at start of first data bit
 * @see     QSPI_SetClockPhase (), QSPI_GetClockPhase()
 */
typedef enum {
    QSPI_PHASE_MIDDLE = QSPI_CTRLR0_SCPH_MIDDLE,
    QSPI_PHASE_START  = QSPI_CTRLR0_SCPH_START
} QSPI_Phase_TypeDef;

/**
 * @typedef
 * @brief   Serial Clock Polarity modes enumeration
 *          Used to select the polarity of the inactive serial clock, which
 *          is held inactive when the QSPI is not actively
 *          transferring data on the serial bus.
 * @note    QSPI_POLARITY_LOW:  inactive state of serial clock is low
 *          QSPI_POLARITY_HIGH: inactive state of serial clock is high
 * @see     QSPI_SetClockPolarity(), QSPI_GetClockPolarity()
 */
typedef enum {
    QSPI_POLARITY_LOW  = QSPI_CTRLR0_SCPOL_LOW,
    QSPI_POLARITY_HIGH = QSPI_CTRLR0_SCPOL_HIGH
} QSPI_Polarity_TypeDef;

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
 * @note    QSPI_XFER_TXRX:   transmit & Receive
 *          QSPI_XFER_TX:     transmit only mode
 *          QSPI_XFER_RX:     receive only mode
 *          QSPI_XFER_EEPROM: EEPROM Read mode
 * @see     QSPI_SetTransferMode(), QSPI_GetTransferMode()
 */
typedef enum{
    QSPI_XFER_TXRX   = QSPI_CTRLR0_TMOD_TX_RX,
    QSPI_XFER_TX     = QSPI_CTRLR0_TMOD_TX,
    QSPI_XFER_RX     = QSPI_CTRLR0_TMOD_RX,
    QSPI_XFER_EEPROM = QSPI_CTRLR0_TMOD_EEPROM_READ
} QSPI_XferMode_TypeDef;


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
 * @see     QSPI_SetDataFrameSize(), QSPI_GetFrameSize()
 */
typedef enum {
    QSPI_DATA_FRAME_SIZE_4  = QSPI_CTRLR0_DFS_32_04_BIT, /*!< 4-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_5  = QSPI_CTRLR0_DFS_32_05_BIT, /*!< 5-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_6  = QSPI_CTRLR0_DFS_32_06_BIT, /*!< 6-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_7  = QSPI_CTRLR0_DFS_32_07_BIT, /*!< 7-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_8  = QSPI_CTRLR0_DFS_32_08_BIT, /*!< 8-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_9  = QSPI_CTRLR0_DFS_32_09_BIT, /*!< 9-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_10 = QSPI_CTRLR0_DFS_32_10_BIT, /*!< 10-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_11 = QSPI_CTRLR0_DFS_32_11_BIT, /*!< 11-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_12 = QSPI_CTRLR0_DFS_32_12_BIT, /*!< 12-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_13 = QSPI_CTRLR0_DFS_32_13_BIT, /*!< 13-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_14 = QSPI_CTRLR0_DFS_32_14_BIT, /*!< 14-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_15 = QSPI_CTRLR0_DFS_32_15_BIT, /*!< 15-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_16 = QSPI_CTRLR0_DFS_32_16_BIT, /*!< 16-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_17 = QSPI_CTRLR0_DFS_32_17_BIT, /*!< 17-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_18 = QSPI_CTRLR0_DFS_32_18_BIT, /*!< 18-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_19 = QSPI_CTRLR0_DFS_32_19_BIT, /*!< 19-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_20 = QSPI_CTRLR0_DFS_32_20_BIT, /*!< 20-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_21 = QSPI_CTRLR0_DFS_32_21_BIT, /*!< 21-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_22 = QSPI_CTRLR0_DFS_32_22_BIT, /*!< 22-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_23 = QSPI_CTRLR0_DFS_32_23_BIT, /*!< 23-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_24 = QSPI_CTRLR0_DFS_32_24_BIT, /*!< 24-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_25 = QSPI_CTRLR0_DFS_32_25_BIT, /*!< 25-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_26 = QSPI_CTRLR0_DFS_32_26_BIT, /*!< 26-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_27 = QSPI_CTRLR0_DFS_32_27_BIT, /*!< 27-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_28 = QSPI_CTRLR0_DFS_32_28_BIT, /*!< 28-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_29 = QSPI_CTRLR0_DFS_32_29_BIT, /*!< 29-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_30 = QSPI_CTRLR0_DFS_32_30_BIT, /*!< 30-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_31 = QSPI_CTRLR0_DFS_32_31_BIT, /*!< 31-bit serial data transfer */
    QSPI_DATA_FRAME_SIZE_32 = QSPI_CTRLR0_DFS_32_32_BIT  /*!< 32-bit serial data transfer */
} QSPI_DataFrameSize_TypeDef;


/**
 * @typedef
 * @brief   Slave Select Enable Flag modes enumeration
 *          Each value corresponds to a slave select line
 *          (ss_x_n) from the QSPI. When a value is set (1),
 *          the corresponding slave select line from
 *          the QSPI is activated when a serial transfer begins. It
 *          should be noted that setting or clearing values
 *          have no effect on the corresponding slave select outputs until
 *          a transfer is started. Before beginning a transfer, you should
 *          enable the value that corresponds to the slave
 *          device with which the QSPI wants to communicate.
 * @note    QSPI_SSEL_NOT_SELECTED : No slave selected
 *          QSPI_SSEL_0:             1-st slave selected
 *          QSPI_SSEL_1:             2-nd slave selected
 *          QSPI_SSEL_2:             3-rd slave selected
 * @see     QSPI_SetSlaveSelect()
 */
typedef enum {
    QSPI_SSEL_NOT_SELECTED = QSPI_SER_SER_NOT_SELECTED,
    QSPI_SSEL_0            = QSPI_SER_SER_0,
    QSPI_SSEL_1            = QSPI_SER_SER_1,
    QSPI_SSEL_2            = QSPI_SER_SER_2,
} QSPI_SlaveSel_TypeDef;


/**
 * @typedef
 * @brief   Frame Format enumeration
 *          Selects data frame format for Transmitting/Receiving the
 *          data.
 * @note    QSPI_FRAMEFORMAT_STD:  standard SPI Frame Format
 *          QSPI_FRAMEFORMAT_DUAL: dual SPI Frame Format
 *          QSPI_FRAMEFORMAT_QUAD: quad SPI Frame Format
 * @see     QSPI_SetFrameFormat(), QSPI_GetFrameFormat()
 */
typedef enum {
    QSPI_FRAMEFORMAT_STD  = QSPI_CTRLR0_SPI_FRF_STD,
    QSPI_FRAMEFORMAT_DUAL = QSPI_CTRLR0_SPI_FRF_DUAL,
    QSPI_FRAMEFORMAT_QUAD = QSPI_CTRLR0_SPI_FRF_QUAD
} QSPI_FrameFormat_TypeDef;


/**
 * @typedef
 * @brief   Address and instruction transfer format modes enumeration
 *          Selects whether QSPI will transmit
 *          instruction/address either in Standard SPI mode or the SPI
 *          mode selected in CTRLR0.SPI_FRF field.
 *          QSPI_XFER_STD:  instruction and Address will be sent
 *                          in Standard SPI Mode
 *          QSPI_XFER_INST: instruction will be sent
 *                          in Standard SPI Mode and address will be sent
 *                          in the mode specified by CTRLR0.SPI_FRF
 *          QSPI_XFER_BOTH: both Instruction and address will be sent
 *                          in the mode specified by SPI_FRF
 * @see     QSPI_SetAddrInstXferMode(), QSPI_GetAddrInstXferMode()
 */
typedef enum {
    QSPI_XFER_STD  = QSPI_SPI_CTRLR0_TRANS_TYPE_STD,
    QSPI_XFER_INST = QSPI_SPI_CTRLR0_TRANS_TYPE_INST,
    QSPI_XFER_BOTH = QSPI_SPI_CTRLR0_TRANS_TYPE_BOTH
} QSPI_AddrInstXferMode_TypeDef;

/**
 * @typedef
 * @brief   Address Length enumeration
 *          This field defines Length of Address to be transmitted. Only
 *          after this much bits are programmed in to the FIFO the
 *          transfer can begin.
 * @see     QSPI_SetAddrLen(), QSPI_GetAddrLen()
 */
typedef enum {
    QSPI_ADDRLEN_0  = QSPI_SPI_CTRLR0_ADDR_L_0,   /*!< 0-bit Addr. width */
    QSPI_ADDRLEN_4  = QSPI_SPI_CTRLR0_ADDR_L_4,   /*!< 4-bit Addr. width */
    QSPI_ADDRLEN_8  = QSPI_SPI_CTRLR0_ADDR_L_8,   /*!< 8-bit Addr. width */
    QSPI_ADDRLEN_12 = QSPI_SPI_CTRLR0_ADDR_L_12,  /*!< 12-bit Addr. width */
    QSPI_ADDRLEN_16 = QSPI_SPI_CTRLR0_ADDR_L_16,  /*!< 16-bit Addr. width */
    QSPI_ADDRLEN_20 = QSPI_SPI_CTRLR0_ADDR_L_20,  /*!< 20-bit Addr. width */
    QSPI_ADDRLEN_24 = QSPI_SPI_CTRLR0_ADDR_L_24,  /*!< 24-bit Addr. width */
    QSPI_ADDRLEN_28 = QSPI_SPI_CTRLR0_ADDR_L_28,  /*!< 28-bit Addr. width */
    QSPI_ADDRLEN_32 = QSPI_SPI_CTRLR0_ADDR_L_32,  /*!< 32-bit Addr. width */
    QSPI_ADDRLEN_36 = QSPI_SPI_CTRLR0_ADDR_L_36,  /*!< 36-bit Addr. width */
    QSPI_ADDRLEN_40 = QSPI_SPI_CTRLR0_ADDR_L_40,  /*!< 40-bit Addr. width */
    QSPI_ADDRLEN_44 = QSPI_SPI_CTRLR0_ADDR_L_44,  /*!< 44-bit Addr. width */
    QSPI_ADDRLEN_48 = QSPI_SPI_CTRLR0_ADDR_L_48,  /*!< 48-bit Addr. width */
    QSPI_ADDRLEN_52 = QSPI_SPI_CTRLR0_ADDR_L_52,  /*!< 52-bit Addr. width */
    QSPI_ADDRLEN_56 = QSPI_SPI_CTRLR0_ADDR_L_56,  /*!< 56-bit Addr. width */
    QSPI_ADDRLEN_60 = QSPI_SPI_CTRLR0_ADDR_L_60,  /*!< 60-bit Addr. width */
} QSPI_AddrLen_TypeDef;

/**
 * @typedef
 * @brief   Instruction Length enumeration
 *          Dual/Quad mode instruction length in bits
 * @note    QSPI_INSTLEN_0:  0-bit (No Instruction)
            QSPI_INSTLEN_4:  4-bit Instruction
            QSPI_INSTLEN_8:  8-bit Instruction
            QSPI_INSTLEN_16: 16-bit Instruction
 * @see     QSPI_SetInstLen(), QSPI_GetInstLen()
 */
typedef enum {
    QSPI_INSTLEN_0  = QSPI_SPI_CTRLR0_INST_L_0,
    QSPI_INSTLEN_4  = QSPI_SPI_CTRLR0_INST_L_4,
    QSPI_INSTLEN_8  = QSPI_SPI_CTRLR0_INST_L_8,
    QSPI_INSTLEN_16 = QSPI_SPI_CTRLR0_INST_L_16
} QSPI_InstLen_TypeDef;

/**
 * @typedef
 * @brief   Wait cycles enuberation
 *          Number of wait cycles in Dual/Quad mode between
 *          control frames transmit and data reception. This value is
 *          specified as number of SPI clock cycles.
 * @see     QSPI_SetWaitCycles(), QSPI_GetWaitCycles()
 */
typedef enum {
    QSPI_WAITCYCLES_0  = QSPI_SPI_CTRLR0_WAIT_CYCLES_0,
    QSPI_WAITCYCLES_1  = QSPI_SPI_CTRLR0_WAIT_CYCLES_1,
    QSPI_WAITCYCLES_2  = QSPI_SPI_CTRLR0_WAIT_CYCLES_2,
    QSPI_WAITCYCLES_3  = QSPI_SPI_CTRLR0_WAIT_CYCLES_3,
    QSPI_WAITCYCLES_4  = QSPI_SPI_CTRLR0_WAIT_CYCLES_4,
    QSPI_WAITCYCLES_5  = QSPI_SPI_CTRLR0_WAIT_CYCLES_5,
    QSPI_WAITCYCLES_6  = QSPI_SPI_CTRLR0_WAIT_CYCLES_6,
    QSPI_WAITCYCLES_7  = QSPI_SPI_CTRLR0_WAIT_CYCLES_7,
    QSPI_WAITCYCLES_8  = QSPI_SPI_CTRLR0_WAIT_CYCLES_8,
    QSPI_WAITCYCLES_9  = QSPI_SPI_CTRLR0_WAIT_CYCLES_9,
    QSPI_WAITCYCLES_10 = QSPI_SPI_CTRLR0_WAIT_CYCLES_10,
    QSPI_WAITCYCLES_11 = QSPI_SPI_CTRLR0_WAIT_CYCLES_11,
    QSPI_WAITCYCLES_12 = QSPI_SPI_CTRLR0_WAIT_CYCLES_12,
    QSPI_WAITCYCLES_13 = QSPI_SPI_CTRLR0_WAIT_CYCLES_13,
    QSPI_WAITCYCLES_14 = QSPI_SPI_CTRLR0_WAIT_CYCLES_14,
    QSPI_WAITCYCLES_15 = QSPI_SPI_CTRLR0_WAIT_CYCLES_15,
    QSPI_WAITCYCLES_16 = QSPI_SPI_CTRLR0_WAIT_CYCLES_16,
    QSPI_WAITCYCLES_17 = QSPI_SPI_CTRLR0_WAIT_CYCLES_17,
    QSPI_WAITCYCLES_18 = QSPI_SPI_CTRLR0_WAIT_CYCLES_18,
    QSPI_WAITCYCLES_19 = QSPI_SPI_CTRLR0_WAIT_CYCLES_19,
    QSPI_WAITCYCLES_20 = QSPI_SPI_CTRLR0_WAIT_CYCLES_20,
    QSPI_WAITCYCLES_21 = QSPI_SPI_CTRLR0_WAIT_CYCLES_21,
    QSPI_WAITCYCLES_22 = QSPI_SPI_CTRLR0_WAIT_CYCLES_22,
    QSPI_WAITCYCLES_23 = QSPI_SPI_CTRLR0_WAIT_CYCLES_23,
    QSPI_WAITCYCLES_24 = QSPI_SPI_CTRLR0_WAIT_CYCLES_24,
    QSPI_WAITCYCLES_25 = QSPI_SPI_CTRLR0_WAIT_CYCLES_25,
    QSPI_WAITCYCLES_26 = QSPI_SPI_CTRLR0_WAIT_CYCLES_26,
    QSPI_WAITCYCLES_27 = QSPI_SPI_CTRLR0_WAIT_CYCLES_27,
    QSPI_WAITCYCLES_28 = QSPI_SPI_CTRLR0_WAIT_CYCLES_28,
    QSPI_WAITCYCLES_29 = QSPI_SPI_CTRLR0_WAIT_CYCLES_29,
    QSPI_WAITCYCLES_30 = QSPI_SPI_CTRLR0_WAIT_CYCLES_30,
    QSPI_WAITCYCLES_31 = QSPI_SPI_CTRLR0_WAIT_CYCLES_31,
} QSPI_WaitCycles_TypeDef;


/**
 * @typedef
 * @brief   QSPI status flags enumeratioin
 * @details Flags in the read-only status register used to indicate the current
 *          transfer status, FIFO status, and any transmission/reception errors.
 *          Status flags  may be read at any time.
 *          QSPI_FLAG_BUSY: transfer in progress
 *                          set: serial transfer is in progress (actively
 *                          transferring data);
 *                          cleared: the QSPI module is idle or disabled
 *          QSPI_FLAG_TFNF: Transmit FIFO Not Full
 *                          set: transmit FIFO contains >= 1 empty locations
 *                          cleared: the Tx FIFO is full
 *          QSPI_FLAG_TFE:  Transmit FIFO Empty
 *                          set: the transmit FIFO is completely empty
 *                          cleared: the transmit FIFO contains one or more
 *                          valid entries
 *          QSPI_FLAG_RFNE: Receive FIFO Not Empty
 *                          set: Rx FIFO is not empty
 *                          cleared: Rx FIFO is empty
 *          QSPI_FLAG_RFF:  Receive FIFO Full
 *                          set: Rx FIFO is completely full
 *                          cleared: Rx FIFO contains >= 1 empty location
 *          QSPI_FLAG_DCOL: Data Collision Error
 *                          If input slave select signal is asserted by other
 *                          master, when the QSPI is in the middle 
 *                          the transfer. This informs the processor that the
 *                          last data transfer was halted before completion.
 *                          set: Tx Data Collision error
 *                          clear: no error
 * @see     QSPI_IsActiveFlag();
 */
typedef enum {
    QSPI_FLAG_BUSY = 0,     /*!< Transfer in progress */
    QSPI_FLAG_TFNF = 1,     /*!< Transmit FIFO Not Full */
    QSPI_FLAG_TFE  = 2,     /*!< Transmit FIFO Empty */
    QSPI_FLAG_RFNE = 3,     /*!< Receive FIFO Not Empty */
    QSPI_FLAG_RFF  = 4,     /*!< Receive FIFO Full */
    QSPI_FLAG_DCOL = 6      /*!< Data Collision Error */
} QSPI_Flag_TypeDef;


/**
 * @brief   Interrupts types enumeration
 * @details The QSPI supports combined interrupt request. Each of individual
 *          interrupt can be masked. The combined interrupt request is the ORed
 *          result of all other QSPI interrupts after masking.
 *          QSPI_IT_TXE: Transmit FIFO Empty
 *                       Set when the transmit FIFO is equal to or below its
 *                       threshold value and requires service to prevent an
 *                       under-run. The threshold value, set through a
 *                       software-programmable register, determines the level of
 *                       transmit FIFO entries at which an interrupt is
 *                       generated. This interrupt is cleared by hardware when
 *                       data are written into the transmit FIFO buffer,
 *                       bringing it over the threshold level.
 *          QSPI_IT_TXO: Transmit FIFO Overflow
 *                       Set when an bus access attempts to write into the
 *                       transmit FIFO after it has been completely filled.
 *                       When set, data written from the bus is discarded. This
 *                       interrupt remains set until you read the transmit FIFO
 *                       overflow interrupt clear register (TXOICR).
 *          QSPI_IT_RXU: Receive FIFO Underflow
 *                       Set when an bus access attempts to read from the
 *                       receive FIFO when it is empty. When set, zeros are read
 *                       back from the receive FIFO. This interrupt remains set
 *                       until you read the receive FIFO underflow interrupt
 *                       clear register (RXUICR).
 *          QSPI_IT_RXO: Receive FIFO Overflow
 *                       Set when the receive logic attempts to place data into
 *                       the receive FIFO after it has been completely filled.
 *                       When set, newly received data are discarded. This
 *                       interrupt remains set until you read the receive FIFO
 *                       overflow interrupt clear register (RXOICR).
 *          QSPI_IT_RXF: Receive FIFO Full
 *                       Set when the receive FIFO is equal to or above its
 *                       threshold value plus 1 and requires service to prevent
 *                       an overflow. The threshold value, set through a
 *                       software-programmable register, determines the level of
 *                       receive FIFO entries at which an interrupt is
 *                       generated. This interrupt is cleared by hardware when
 *                       data are read from the receive FIFO buffer, bringing it
 *                       below the threshold level.
 *          QSPI_IT_MST: Multi-Master Contention
 *                       The interrupt is set when another serial master on the
 *                       serial bus selects the QSPI master as a serial-slave
 *                       device and is actively transferring data. This informs
 *                       the processor of possible contention on the serial bus.
 *                       This interrupt remains set until you read the
 *                       multi-master interrupt clear register (MSTICR).
 * @see     QSPI_EnableIT(), QSPI_DisableIT(), QSPI_IsEnabledIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveIT(), QSPI_IsActiveITRaw
 */
typedef enum {
    QSPI_IT_TXE = 0,        /*!< Transmit FIFO Empty */
    QSPI_IT_TXO = 1,        /*!< Transmit FIFO Overflow  */
    QSPI_IT_RXU = 2,        /*!< Receive FIFO Underflow */
    QSPI_IT_RXO = 3,        /*!< Receive FIFO Overflow */
    QSPI_IT_RXF = 4,        /*!< Receive FIFO Full */
    QSPI_IT_MST = 5         /*!< Multi-Master Contention */
} QSPI_IT_TypeDef;


/**
 * @typedef
 * @brief   QSPI initialization structure
 *          Used for initial QSPI parameters setup
 * @see     QSPI_Init(), QSPI_StructInit()
 */
typedef struct {
    uint16_t                      ClkDivider;
    QSPI_Polarity_TypeDef         Polarity;
    QSPI_Phase_TypeDef            Phase;
    QSPI_XferMode_TypeDef         XferMode;
    QSPI_DataFrameSize_TypeDef    DataFrameSize;
    QSPI_SlaveSel_TypeDef         SlaveSel;
    QSPI_FrameFormat_TypeDef      FrameFormat;
    QSPI_AddrInstXferMode_TypeDef AddrInstXferMode;
    QSPI_AddrLen_TypeDef          AddrLen;
    QSPI_InstLen_TypeDef          InstLen;
    QSPI_WaitCycles_TypeDef       WaitCycles;
    FunctionalState               SlaveSelToggle;
    FunctionalState               EndianConv;
    FunctionalState               ShiftRegLoop;
    FunctionalState               DDR;
    FunctionalState               InstDDR;
    uint8_t                       FIFOTxThreshold;
    uint8_t                       FIFORxThreshold;
    uint8_t                       RxSampleDelay;
} QSPI_InitStruct_TypeDef;

/* Exported constants --------------------------------------------------------*/
/*!< Max value of data frame size */
#define QSPI_XFER_SIZE_MAX            (32U)
/*!< Only even values of SCKDV allowed */
#define QSPI_SCKDV_MAX                (0xFFFEU)
/*!< Maximum number of QSPI clock cycles that can be used to delay
the sampling of the Rx input */
#define QSPI_RX_DELAY_DEPTH_MAX       (8U)
/*!< FIFO depth. Equal for Transmit and Receive */
#define QSPI_FIFO_DEPTH               (8U)
/*!< FIFO threshold max value. Equal for Transmit and Receive */
#define QSPI_FIFO_TFT_MAX             (QSPI_FIFO_DEPTH - 1)
/*!< DMA Transmit Data Level max value */
#define QSPI_DMATDL_MAX               (QSPI_FIFO_DEPTH - 1)
/*!< DMA Receive Data Level max value */
#define QSPI_DMARDL_MAX               (QSPI_FIFO_DEPTH - 1)
/*!< ClkDivider default value */
#define QSPI_CLKDIVIDER_DEFAULT       (QSPI_SCKDV_MAX / 10)
/*!< Tx FIFO Threshold level default value */
#define QSPI_FIFOTXTHRESHOLD_DEFAULT  (QSPI_FIFO_TFT_MAX / 2)
/*!< Rx FIFO Threshold level default value */
#define QSPI_FIFORXTHRESHOLD_DEFAULT  (QSPI_FIFO_TFT_MAX / 2)
/*!< Rx sample delay default value */
#define QSPI_RX_DELAY_DEPTH_DEFAULT   (1U)
/*!< DeInit max attempts value (see QSPI_DeInit()) */
#define QSPI_DEINIT_RETRY_COUNT (1000U)

/* Exported macro ------------------------------------------------------------*/
/**
  * @brief  Write a value in QSPI register
  * @param  __INSTANCE__ QSPI Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define QSPI_WriteReg(__INSTANCE__, __REG__, __VALUE__) \
        WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in QSPI register
  * @param  __INSTANCE__ QSPI Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define QSPI_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Enable QSPI peripheral block
 * @details Enables QSPI operations. When QSPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the QSPI control registers
 *          when enabled.
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_Disable()
 */
__STATIC_INLINE void
QSPI_Enable (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->QSPIENR, QSPI_QSPIENR_QSPI_EN_Msk, QSPI_QSPIENR_QSPI_EN_ENABLED);
}


/**
 * @brief   Disable QSPI peripheral block
 * @details Disable all QSPI operations. When QSPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the QSPI control registers
 *          when enabled.
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_Enable()
 */
__STATIC_INLINE void
QSPI_Disable (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->QSPIENR, QSPI_QSPIENR_QSPI_EN_Msk, QSPI_QSPIENR_QSPI_EN_DISABLED);
}


/**
 * @brief   Check if th requested QSPI is enabled
 * @details When QSPI is disabled, all serial
 *          transfers are halted immediately. Transmit and receive FIFO buffers
 *          are cleared when the device is disabled.
 *          It is impossible to program some of the QSPI control registers
 *          when enabled.
 * @param   QSPIx The QSPI instance
 * @retval  1 if QSPI instance is enabled, 0 otherwise
 * @see     QSPI_Enable(), QSPI_Disable()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabled (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->QSPIENR, QSPI_QSPIENR_QSPI_EN_Msk) ==
                             (QSPI_QSPIENR_QSPI_EN_ENABLED)) ? ENABLE : DISABLE;
}


/**
 * @brief   Configure the transfer direction
 * @param   QSPIx The QSPI instance
 * @param   XferMode transfer mode (direction)
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetTransferMode()
 */
__STATIC_INLINE void
QSPI_SetTransferMode (QSPI_TypeDef *QSPIx,
                           QSPI_XferMode_TypeDef XferMode) {
    MODIFY_REG(QSPIx->CTRLR0, QSPI_CTRLR0_TMOD_Msk, XferMode);
}


/**
 * @brief   Get the configured transfer direction
 * @param   QSPIx The QSPI instance
 * @retval  configured transfer mode (direction)
 * @see     QSPI_SetTransferMode()
 */
__STATIC_INLINE QSPI_XferMode_TypeDef
QSPI_GetTransferMode (QSPI_TypeDef *QSPIx) {
    return (QSPI_XferMode_TypeDef)(READ_BIT(QSPIx->CTRLR0,
                                                         QSPI_CTRLR0_TMOD_Msk));
}


/**
 * @brief   Configure the data frame size
 * @param   QSPIx The QSPI instance
 * @param   DataFrameSize data frame size
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetFrameSize()
 */
__STATIC_INLINE void
QSPI_SetDataFrameSize (QSPI_TypeDef *QSPIx, QSPI_DataFrameSize_TypeDef DataFrameSize) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_DFS_32_Msk, DataFrameSize);
}


/**
 * @brief   Get the configured data frame size
 * @param   QSPIx The QSPI instance
 * @retval  Data frame size value
 * @see     QSPI_SetDataFrameSize()
 */
__STATIC_INLINE QSPI_DataFrameSize_TypeDef
QSPI_GetFrameSize (QSPI_TypeDef *QSPIx) {
    return (QSPI_DataFrameSize_TypeDef)(READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_DFS_32_Msk));
}


/**
 * @brief   Enable the endianness for XIP and data register reads
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_DisableEndianConversion(), QSPI_IsEnabledEndianConversion
 */
__STATIC_INLINE void
QSPI_EnableEndianConversion (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SECONV_Msk, QSPI_CTRLR0_SECONV_ENABLED);
}


/**
 * @brief   Disable the endianness for XIP and data register reads
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_EnableEndianConversion(), QSPI_IsEnabledEndianConversion
 */
__STATIC_INLINE void
QSPI_DisableEndianConversion (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SECONV_Msk, QSPI_CTRLR0_SECONV_DISABLED);
}


/**
 * @brief   Check if the endianness for XIP and data register reads is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if the endianness for XIP and data register reads is enabled,
 *          0 otherwise
 * @see     QSPI_EnableEndianConversion(), QSPI_DisableEndianConversion()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledEndianConversion (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_SECONV_Msk) ==
                    (QSPI_CTRLR0_SECONV_ENABLED)) ? ENABLE : DISABLE;
}

/**
 * @brief   Set divider for output serial clock.
 * @param   QSPIx The QSPI instance
 * @param   ClkDivider Output clock divider value (2..QSPI_SCKDV_MAX)
 *          The LSB is always set to 0 and is unafected by a write operation.
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @note    Serial output clock is disable in case of ClkDivider == 0.
 * @retval  none
 * @see     QSPI_GetClkDivider()
 */

__STATIC_INLINE void
QSPI_SetClkDivider (QSPI_TypeDef *QSPIx, uint16_t ClkDivider) {
    WRITE_REG(QSPIx->BAUDR, ClkDivider);
}


/**
 * @brief   Get the clock divider value
 * @param   QSPIx The QSPI instance
 * @retval  Clock divider value
 * @see     QSPI_SetClkDivider()
 */
__STATIC_INLINE uint16_t
QSPI_GetClkDivider (QSPI_TypeDef *QSPIx) {
    return (uint16_t)(READ_REG(QSPIx->BAUDR));
}


/**
 * @brief   Configure the clock polarity value
 * @param   QSPIx The QSPI instance
 * @param   ClockPolarity the new clock polarity value
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetClockPolarity()
 */
__STATIC_INLINE void
QSPI_SetClockPolarity (QSPI_TypeDef *QSPIx, QSPI_Polarity_TypeDef ClockPolarity) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SCPOL_Msk, ClockPolarity);
}


/**
 * @brief   Get the clock polarity value
 * @param   QSPIx The QSPI instance
 * @retval  Clock polarity value
 * @see     QSPI_SetClockPolarity()
 */
__STATIC_INLINE QSPI_Polarity_TypeDef
QSPI_GetClockPolarity (QSPI_TypeDef *QSPIx) {
    return (QSPI_Polarity_TypeDef)(READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_SCPOL_Msk));
}


/**
 * @brief   Configure the clock phase value
 * @param   QSPIx The QSPI instance
 * @param   ClockPhase
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetClockPhase()
 */
__STATIC_INLINE void
QSPI_SetClockPhase (QSPI_TypeDef *QSPIx, QSPI_Phase_TypeDef ClockPhase) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SCPH_Msk, ClockPhase);
}


/**
 * @brief   Get the clock phase value
 * @param   QSPIx The QSPI instance
 * @retval  Clock phase value
 * @see     QSPI_SetClockPhase()
 */
__STATIC_INLINE QSPI_Phase_TypeDef
QSPI_GetClockPhase (QSPI_TypeDef *QSPIx) {
    return (QSPI_Phase_TypeDef)(READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_SCPH_Msk));
}


/**
 * @brief   Configure the data frame format (standard, dual or quad mode)
 * @param   QSPIx The QSPI instance
 * @param   FrameFormat
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetFrameFormat()
 */
__STATIC_INLINE void
QSPI_SetFrameFormat (QSPI_TypeDef *QSPIx,
                        QSPI_FrameFormat_TypeDef FrameFormat) {
    MODIFY_REG(QSPIx->CTRLR0, QSPI_CTRLR0_SPI_FRF_Msk, FrameFormat);
}


/**
 * @brief   Get the data frame format (standard, dual or quad mode)
 * @param   QSPIx The QSPI instance
 * @retval  Data frame format (standard, dual or quad mode)
 * @see     QSPI_SetFrameFormat()
 */
__STATIC_INLINE QSPI_FrameFormat_TypeDef
QSPI_GetFrameFormat (QSPI_TypeDef *QSPIx) {
    return (QSPI_FrameFormat_TypeDef)(READ_BIT(QSPIx->CTRLR0,
                                            QSPI_CTRLR0_SPI_FRF_Msk));
}


/**
 * @brief   Enable slave select toggling
 * @details When operating with clock phase set to QSPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_DisableSlaveSelToggle(), QSPI_IsEnabledSlaveSelToggle
 */
__STATIC_INLINE void
QSPI_EnableSlaveSelToggle (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SSTE_Msk, QSPI_CTRLR0_SSTE_ENABLED);
}


/**
 * @brief   Disable slave select toggling
 * @details When operating with clock phase set to QSPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_EnableSlaveSelToggle(), QSPI_IsEnabledSlaveSelToggle
 */
__STATIC_INLINE void
QSPI_DisableSlaveSelToggle (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0 , QSPI_CTRLR0_SSTE_Msk, QSPI_CTRLR0_SSTE_DISABLED);
}


/**
 * @brief   Check if slave select toggling is enabled
 * @details When operating with clock phase set to QSPI_PHASE_MIDDLE
 *          this slabe select toggling controls the behavior of the slave select
 *          line between data frames. If slave select toggling is enabled
 *          the ss_*_n line will toggle between consecutive data frames,
 *          with the serial clock (sclk) being held to its default value while
 *          ss_*_n is high; if slave select toggling is disabled the ss_*_n will
 *          stay low and sclk will run continuously for the duration of the
 *          transfer.
 * @param   QSPIx The QSPI instance
 * @retval  1 if slave select togglingis enabled, 0 otherwise
 * @see     QSPI_EnableSlaveSelToggle(), QSPI_DisableSlaveSelToggle()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledSlaveSelToggle (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_SSTE_Msk) ==
                    (QSPI_CTRLR0_SSTE_ENABLED)) ? ENABLE : DISABLE;
}


/**
 * @brief   Configure address and instruction transfer mode
 * @param   QSPIx The QSPI instance
 * @param   XferMode address and instruction transfer mode
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetAddrInstXferMode()
 */
__STATIC_INLINE void
QSPI_SetAddrInstXferMode (QSPI_TypeDef *QSPIx,
                         QSPI_AddrInstXferMode_TypeDef AddrInstXferMode) {
    MODIFY_REG(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_TRANS_TYPE_Msk, AddrInstXferMode);
}


/**
 * @brief   Get the address and instruction transfer mode
 * @param   QSPIx The QSPI instance
 * @retval  address and instruction transfer mode
 * @see     QSPI_SetAddrInstXferMode()
 */
__STATIC_INLINE QSPI_AddrInstXferMode_TypeDef
QSPI_GetAddrInstXferMode (QSPI_TypeDef *QSPIx) {
    return (QSPI_AddrInstXferMode_TypeDef)(READ_BIT(QSPIx->SPI_CTRLR0,
                                               QSPI_SPI_CTRLR0_TRANS_TYPE_Msk));
}


/**
 * @brief   Configure the length of address to be transmitted
 * @param   QSPIx The QSPI instance
 * @param   AddrLen length of address to be transmitted
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetAddrLen()
 */
__STATIC_INLINE void
QSPI_SetAddrLen (QSPI_TypeDef *QSPIx, QSPI_AddrLen_TypeDef AddrLen) {
    MODIFY_REG(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_ADDR_L_Msk, AddrLen);
}


/**
 * @brief   Get the length of address to be transmitted
 * @param   QSPIx The QSPI instance
 * @retval  Length of address to be transmitted
 * @see     QSPI_SetAddrLen()
 */
__STATIC_INLINE QSPI_AddrLen_TypeDef
QSPI_GetAddrLen (QSPI_TypeDef *QSPIx) {
    return (QSPI_AddrLen_TypeDef)(READ_BIT(QSPIx->SPI_CTRLR0,
                                                QSPI_SPI_CTRLR0_ADDR_L_Msk));
}


/**
 * @brief   Configure Dual/Quad mode instruction length
 * @param   QSPIx The QSPI instance
 * @param   InstLen instruction length value
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetInstLen()
 */
__STATIC_INLINE void
QSPI_SetInstLen (QSPI_TypeDef *QSPIx, QSPI_InstLen_TypeDef InstLen) {
    MODIFY_REG(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_INST_L_Msk, InstLen);
}


/**
 * @brief   Get the Dual/Quad mode instruction length
 * @param   QSPIx The QSPI instance
 * @retval  Dual/Quad mode instruction length
 * @see     QSPI_SetInstLen()
 */
__STATIC_INLINE QSPI_InstLen_TypeDef
QSPI_GetInstLen (QSPI_TypeDef *QSPIx) {
    return (QSPI_InstLen_TypeDef)(READ_BIT(QSPIx->SPI_CTRLR0,
                                                   QSPI_SPI_CTRLR0_INST_L_Msk));
}


/**
 * @brief   Configure the number of wait cycles in Dual/Quad mode
 * @param   QSPIx The QSPI instance
 * @param   WaitCycles Number of wait cycles value
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetWaitCycles()
 */
__STATIC_INLINE void
QSPI_SetWaitCycles (QSPI_TypeDef *QSPIx, QSPI_WaitCycles_TypeDef WaitCycles) {
    MODIFY_REG(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_WAIT_CYCLES_Msk, WaitCycles);
}


/**
 * @brief   Get the number of wait cycles in Dual/Quad mdoe
 * @param   QSPIx The QSPI instance
 * @retval  Number of wait cycles in Dual/Quad mode
 * @see     QSPI_SetWaitCycles()
 */
__STATIC_INLINE QSPI_WaitCycles_TypeDef
QSPI_GetWaitCycles (QSPI_TypeDef *QSPIx) {
    return (QSPI_WaitCycles_TypeDef)(READ_BIT(QSPIx->SPI_CTRLR0,
                                              QSPI_SPI_CTRLR0_WAIT_CYCLES_Msk));
}

/**
 * @brief   Enable Dual-data rate transfers in Dual/Quad frame formats of SPI
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_DisableDDR(), QSPI_IsEnabledDDR()
 */
__STATIC_INLINE void
QSPI_EnableDDR (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->SPI_CTRLR0 , QSPI_SPI_CTRLR0_SPI_DDR_EN_Msk,
                                             QSPI_SPI_CTRLR0_SPI_DDR_EN_ENABLE);
}


/**
 * @brief   Disable Dual-data rate transfers in Dual/Quad frame formats of SPI
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_EnableDDR(), QSPI_IsEnabledDDR()
 */
__STATIC_INLINE void
QSPI_DisableDDR (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->SPI_CTRLR0 , QSPI_SPI_CTRLR0_SPI_DDR_EN_Msk,
                                            QSPI_SPI_CTRLR0_SPI_DDR_EN_DISABLE);
}


/**
 * @brief   Check if Dual-data rate transfers in Dual/Quad frame formats
 *          of SPI is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if Dual-data rate transfers in Dual/Quad frame formats
 *          of SPI is enabled, 0 otherwise
 * @see     QSPI_EnableDDR(), QSPI_DisableDDR()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledDDR (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_SPI_DDR_EN_Msk) ==
                    (QSPI_SPI_CTRLR0_SPI_DDR_EN_ENABLE)) ? ENABLE : DISABLE;
}


/**
 * @brief   Enable dual-data rate transfer for Instruction phase
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_DisableInstDDR(), QSPI_IsEnabledInstDDR()
 */
__STATIC_INLINE void
QSPI_EnableInstDDR (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->SPI_CTRLR0 , QSPI_SPI_CTRLR0_INST_DDR_EN_Msk,
                                            QSPI_SPI_CTRLR0_INST_DDR_EN_ENABLE);
}


/**
 * @brief   Disable dual-data rate transfer for Instruction phase
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_EnableInstDDR(),QSPI_IsEnabledInstDDR()
 */
__STATIC_INLINE void
QSPI_DisableInstDDR (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->SPI_CTRLR0 , QSPI_SPI_CTRLR0_INST_DDR_EN_Msk,
                                           QSPI_SPI_CTRLR0_INST_DDR_EN_DISABLE);
}


/**
 * @brief   Check if dual-data rate transfer for Instruction phase is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if dual-data rate transfer for Instruction phase is enabled,
 *          0 otherwise
 * @see     QSPI_EnableInstDDR(), QSPI_DisableInstDDR()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledInstDDR (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->SPI_CTRLR0, QSPI_SPI_CTRLR0_INST_DDR_EN_Msk) ==
                    (QSPI_SPI_CTRLR0_INST_DDR_EN_ENABLE)) ? ENABLE : DISABLE;
}


/**
 * @brief   Enable internal Tx-Rx shift registers loop
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_DisableShiftRegLoop(), QSPI_IsEnabledShiftRegLoop()
 */
__STATIC_INLINE void
QSPI_EnableShiftRegLoop (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0, QSPI_CTRLR0_SRL_Msk, QSPI_CTRLR0_SRL_TESTING);
}


/**
 * @brief   Disable internal Tx-Rx shift registers loop
 * @param   QSPIx The QSPI instance
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_EnableShiftRegLoop(), QSPI_IsEnabledShiftRegLoop()
 */
__STATIC_INLINE void
QSPI_DisableShiftRegLoop (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->CTRLR0, QSPI_CTRLR0_SRL_Msk, QSPI_CTRLR0_SRL_NORMAL);
}


/**
 * @brief   Check if internal Tx-Rx shift registers loop is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if internal Tx-Rx shift registers loop is enabled, 0 otherwise
 * @see     QSPI_EnableShiftRegLoop(), QSPI_DisableShiftRegLoop()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledShiftRegLoop (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->CTRLR0, QSPI_CTRLR0_SRL_Msk) ==
                    (QSPI_CTRLR0_SRL_TESTING)) ? ENABLE : DISABLE;
}


/**
 * @brief   Configure number of data frames to be continuously received
 * @note    The QSPI continues to receive serial data until the number of data
 *          frames received is equal to this register value plus 1, which
 *          enables you to receive up to 64KB of data in a
 *          continuous transfer.
 * @param   QSPIx The QSPI instance
 * @param   NumOfDataFrames number of data frames to be continuously received
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetNumOfDataFrames()
 */
__STATIC_INLINE void
QSPI_SetNumOfDataFrames (QSPI_TypeDef *QSPIx, uint16_t NumOfDataFrames) {
    WRITE_REG(QSPIx->CTRLR1, NumOfDataFrames);
}


/**
 * @brief   Get the number of data frames to be continuously received
 * @param   QSPIx The QSPI instance
 * @retval  number of data frames to be continuously received
 * @see     QSPI_SetNumOfDataFrames()
 */
__STATIC_INLINE uint16_t
QSPI_GetNumOfDataFrames (QSPI_TypeDef *QSPIx) {
    return (uint16_t)(READ_REG(QSPIx->CTRLR1));
}


/**
 * @brief   Configure slave select line
 * @param   QSPIx The QSPI instance
 * @param   SlaveSel slave select line
 * @retval  None
 * @see     QSPI_GetSlaveSelect()
 */
__STATIC_INLINE void
QSPI_SetSlaveSelect (QSPI_TypeDef *QSPIx, QSPI_SlaveSel_TypeDef SlaveSel) {
    WRITE_REG(QSPIx->SER, SlaveSel);
}


/**
 * @brief   Get the slave select line
 * @param   QSPIx The QSPI instance
 * @retval  slave select line
 * @see     QSPI_SetSlaveSelect()
 */
__STATIC_INLINE QSPI_SlaveSel_TypeDef
QSPI_GetSlaveSelect (QSPI_TypeDef *QSPIx) {
    return (QSPI_SlaveSel_TypeDef)(READ_REG(QSPIx->SER));
}


/**
 * @brief   Configure the transmit FIFO threshold level
 * @param   QSPIx The QSPI instance
 * @param   Threshold transmit FIFO threshold level
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetFIFOTxThreshold()
 */
__STATIC_INLINE void
QSPI_SetFIFOTxThreshold (QSPI_TypeDef *QSPIx, uint8_t Threshold) {
    WRITE_REG(QSPIx->TXFTLR, Threshold);
}


/**
 * @brief   Get the transmit FIFO threshold level
 * @param   QSPIx The QSPI instance
 * @retval  Transmit FIFO threshold level
 * @see     QSPI_SetFIFOTxThreshold()
 */
__STATIC_INLINE uint8_t
QSPI_GetFIFOTxThreshold (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->TXFTLR));
}


/**
 * @brief   Configure the receive FIFO threshold level
 * @param   QSPIx The QSPI instance
 * @param   Threshold receive FIFO threshold level
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetFIFORxThreshold()
 */
__STATIC_INLINE void
QSPI_SetFIFORxThreshold (QSPI_TypeDef *QSPIx, uint8_t Threshold) {
    WRITE_REG(QSPIx->RXFTLR, Threshold);
}


/**
 * @brief   Get the receive FIFO threshold level
 * @param   QSPIx The QSPI instance
 * @retval  Receive FIFO threshold level
 * @see     QSPI_SetFIFORxThreshold()
 */
__STATIC_INLINE uint8_t
QSPI_GetFIFORxThreshold (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->RXFTLR));
}


/**
 * @brief   Get the transmit FIFO level
 * @param   QSPIx The QSPI instance
 * @retval  Transmit FIFO level value
 */
__STATIC_INLINE uint8_t
QSPI_GetFIFOTxLvl (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->TXFLR));
}


/**
 * @brief   Get the receive FIFO level
 * @param   QSPIx The QSPI instance
 * @retval  Receive FIFO level value
 */
__STATIC_INLINE uint8_t
QSPI_GetFIFORxLvl (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->RXFLR));
}


/**
 * @brief   Configure RX Sample Delay value
 * @details Control the number of QSPI clock cycles that are delayed (from the
 *          default sample time) before the actual sample of the
 *          input signal occurs.  This additional logic can help to increase
 *          the maximum achievable frequency on the bus.
 * @param   QSPIx The QSPI instance
 * @param   Delay RX Sample Delay value (0..QSPI_RX_DELAY_DEPTH_MAX)
 * @note    It is incorrect to execute this function when the QSPI is enabled
 * @retval  None
 * @see     QSPI_GetRxSampleDelay()
 */
__STATIC_INLINE void
QSPI_SetRxSampleDelay (QSPI_TypeDef *QSPIx, uint8_t Delay) {
    WRITE_REG(QSPIx->RX_SAMPLE_DLY, Delay);
}


/**
 * @brief   Get the RX Sample Delay value
 * @param   QSPIx The QSPI instance
 * @retval  RX Sample Delay value
 * @see     QSPI_SetRxSampleDelay()
 */
__STATIC_INLINE uint8_t
QSPI_GetRxSampleDelay (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->RX_SAMPLE_DLY));
}


/**
 * @brief   Check if specified QSPI status flag is set
 * @param   QSPIx The QSPI instance
 * @param   Flag Requested flag. Can be one of QSPI_Flag_TypeDef values.
 * @retval  1 if the flag was set; 0 if the flag was cleared
 */
__STATIC_INLINE FunctionalState
QSPI_IsActiveFlag(QSPI_TypeDef *QSPIx, QSPI_Flag_TypeDef Flag) {
    return (READ_BIT(QSPIx->SR, (1UL << Flag)) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable specified interrupt
 * @param   QSPIx The QSPI instance
 * @param   IT The nterrupt source. Can be one of the QSPI_IT_TypeDef values
 * @retval  None
 * @see     QSPI_DisableIT(), QSPI_IsEnabledIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveIT(), QSPI_IsActiveITRaw
 */
__STATIC_INLINE void
QSPI_EnableIT (QSPI_TypeDef *QSPIx, QSPI_IT_TypeDef IT) {
    SET_BIT(QSPIx->IMR, (1UL << IT));
}


/**
 * @brief   Disable specified interrupt
 * @param   QSPIx The QSPI instance
 * @param   IT The nterrupt source. Can be one of the QSPI_IT_TypeDef values
 * @retval  None
 * @see     QSPI_EnableIT(), QSPI_IsEnabledIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveIT(), QSPI_IsActiveITRaw
 */
__STATIC_INLINE void
QSPI_DisableIT (QSPI_TypeDef *QSPIx, QSPI_IT_TypeDef IT) {
    CLEAR_BIT(QSPIx->IMR, (1UL << IT));
}


/**
 * @brief   Checks if specified interrupt is enabled
 * @param   QSPIx The QSPI instance
 * @param   IT The interrupt source. Can be one of QSPI_IT_TypeDef values
 * @retval  1 if the interrupt is enabled, otherwise 0
 * @see     QSPI_EnableIT(), QSPI_DisableIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveIT(), QSPI_IsActiveITRaw
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledIT (QSPI_TypeDef *QSPIx, QSPI_IT_TypeDef IT) {
    return (READ_BIT(QSPIx->IMR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Clear interrupt
 * @note    Clears all QSPI interrupt flags
 *          It is possible to clear QSPI_IT_TXO, QSPI_IT_RXU, QSPI_IT_RXO and
 *          QSPI_IT_MST separately be reading TXOICR, RXUICR, RXOICR and MSTICR
 *          registers respectively
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_EnableIT(), QSPI_DisableIT(), QSPI_IsEnabledIT(),
 *          QSPI_IsActiveIT(), QSPI_IsActiveITRaw
 */
__STATIC_INLINE void
QSPI_ClearIT (QSPI_TypeDef *QSPIx) {
    (void) READ_BIT(QSPIx->ICR, QSPI_ICR_ICR_Msk);
}


/**
 * @brief   Check if the specified QSPI interrupt is active
 * @param   QSPIx The QSPI instance
 * @param   IT The interrupt source. Can be one of QSPI_IT_TypeDef values
 * @retval  1 if interrupt is active, otherwise 0
 * @see     QSPI_EnableIT(), QSPI_DisableIT(), QSPI_IsEnabledIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveITRaw
 */
__STATIC_INLINE FunctionalState
QSPI_IsActiveIT (QSPI_TypeDef *QSPIx, QSPI_IT_TypeDef IT) {
    return (READ_BIT(QSPIx->ISR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Check if the specified QSPI interrupt is active (raw status)
 * @param   QSPIx The QSPI instance
 * @param   IT The interrupt source. Can be one of QSPI_IT_TypeDef values
 * @retval  1 if interrupt is active, otherwise 0
 * @see     QSPI_EnableIT(), QSPI_DisableIT(), QSPI_IsEnabledIT(),
 *          QSPI_ClearIT(), QSPI_IsActiveIT()
 */
__STATIC_INLINE FunctionalState
QSPI_IsActiveITRaw (QSPI_TypeDef *QSPIx, QSPI_IT_TypeDef IT) {
    return (READ_BIT(QSPIx->RISR, (1UL << IT)) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable the DMA Transmit operation
 * @note    Enables the transmit FIFO DMA channel
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_DisableDMATx(), QSPI_IsEnabledDMATx
 */
__STATIC_INLINE void
QSPI_EnableDMATx (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->DMACR, QSPI_DMACR_TDMAE_Msk, QSPI_DMACR_TDMAE_ENABLE);
}


/**
 * @brief   Disable the DMA Transmit operation
 * @note    Disables the transmit FIFO DMA channel
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_EnableDMATx(), QSPI_IsEnabledDMATx
 */
__STATIC_INLINE void
QSPI_DisableDMATx (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->DMACR, QSPI_DMACR_TDMAE_Msk, QSPI_DMACR_TDMAE_DISABLE);
}


/**
 * @brief   Checks if DMA Receiving is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if DMA Rx is enabled, otherwise 0
 * @see     QSPI_EnableDMATx(), SPI_DisableDMATx()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledDMATx (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->DMACR, QSPI_DMACR_TDMAE_Msk) ? ENABLE : DISABLE);
}


/**
 * @brief   Enable the DMA Receive operation
 * @note    Enables the receive FIFO DMA channel
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_DisableDMARx(), QSPI_IsEnabledDMARx()
 */
__STATIC_INLINE void
QSPI_EnableDMARx (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->DMACR, QSPI_DMACR_RDMAE_Msk, QSPI_DMACR_RDMAE_ENABLE);
}


/**
 * @brief   Disable the DMA Receive operation
 * @note    Disables the receive FIFO DMA channel
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_EnableDMARx(), QSPI_IsEnabledDMARx()
 */
__STATIC_INLINE void
QSPI_DisableDMARx (QSPI_TypeDef *QSPIx) {
    MODIFY_REG(QSPIx->DMACR, QSPI_DMACR_RDMAE_Msk, QSPI_DMACR_RDMAE_DISABLE);
}


/**
 * @brief   Checks if DMA Receiving is enabled
 * @param   QSPIx The QSPI instance
 * @retval  1 if DMA Rx is enabled, otherwise 0
 * @see     QSPI_EnableDMARx(), QSPI_DisableDMARx()
 */
__STATIC_INLINE FunctionalState
QSPI_IsEnabledDMARx (QSPI_TypeDef *QSPIx) {
    return (READ_BIT(QSPIx->DMACR, QSPI_DMACR_RDMAE_Msk) ? ENABLE : DISABLE);
}


/**
 * @brief   Set DMA Transmit data level
 * @param   QSPIx The QSPI instance
 * @param   DataLvl The level at which a DMA request is made by the transmit
 *          logic. It is equal to the watermark level; that is, the DMA Tx
 *          request signal is generated when the number of valid data entries in
 *          the transmit FIFO is equal to or below this value, and QSPI DMA Tx
 *          is enabled (QSPI->DMACR:TDMAE = 1).
 *          DMA Tx request signal is asserted when DataLvl or less data entries
 *          are present in the transmit FIFO
 *          Range of acceptable values: 0..QSPI_DMATDL_MAX
 * @see     QSPI_GetDMATxDataLvl()
 */
__STATIC_INLINE void
QSPI_SetDMATxDataLvl (QSPI_TypeDef *QSPIx, uint32_t DataLvl) {
    WRITE_REG(QSPIx->DMATDLR, DataLvl);
}


/**
 * @brief   Get DMA Transmit data level
 * @param   QSPIx The QSPI instance
 * @retval  The level at which a DMA request is made by the transmit
 *          logic. It is equal to the watermark level; that is, the DMA Tx
 *          request signal is generated when the number of valid data entries in
 *          the transmit FIFO is equal to or below this value, and QSPI DMA Tx
 *          is enabled (QSPI->DMACR:TDMAE = 1).
 *          DMA Tx request signal is asserted when DataLvl or less data entries
 *          are present in the transmit FIFO
 * @see     QSPI_SetDMATxDataLvl()
 */
__STATIC_INLINE uint32_t
QSPI_GetDMATxDataLvl (QSPI_TypeDef *QSPIx) {
    return (uint32_t)(READ_REG(QSPIx->DMATDLR));
}


/**
 * @brief   Set DMA Receive data level
 * @param   QSPIx The QSPI instance
 * @param   DataLvl The level at which a DMA request is made by the receive
 *          logic. The watermark level = DataLvl+1; that is, DMA Rx request is
 *          generated when the number of valid data entries in the receive FIFO
 *          is equal to or above this DataLvl value + 1, and QSPI DMA Rx is
 *          enabled (QSPIx->DMACR:RDMAE=1).
 *          DMA Rx request is asserted when DataLvl or more valid data entries
 *          are present in the receive FIFO
 *          Range of acceptable values: 0..QSPI_DMARDL_MAX
 * @see     QSPI_GetDMARxDataLvl()
 */
__STATIC_INLINE void
QSPI_SetDMARxDataLvl (QSPI_TypeDef *QSPIx, uint32_t DataLvl) {
    WRITE_REG(QSPIx->DMARDLR, DataLvl);
}


/**
 * @brief   Get DMA Receive data level
 * @param   QSPIx The QSPI instance
 * @retval  The level at which a DMA request is made by the receive
 *          logic. The watermark level = DataLvl+1; that is, DMA Rx request is
 *          generated when the number of valid data entries in the receive FIFO
 *          is equal to or above this DataLvl value + 1, and QSPI DMA Rx is
 *          enabled (QSPIx->DMACR:RDMAE=1).
 *          DMA Rx request is asserted when DataLvl or more valid data entries
 *          are present in the receive FIFO
 * @see     QSPI_SetDMARxDataLvl()
 */
__STATIC_INLINE uint32_t
QSPI_GetDMARxDataLvl (QSPI_TypeDef *QSPIx) {
    return (uint32_t)(READ_REG(QSPIx->DMARDLR));
}


/**
 * @brief   Receive 8 bit data from Data Register (Rx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  8-bit eceived data
 * @see     QSPI_ReceiveData16(), QSPI_ReceiveData32()
 */
__STATIC_INLINE uint8_t
QSPI_ReceiveData8 (QSPI_TypeDef *QSPIx) {
    return (uint8_t)(READ_REG(QSPIx->DR[0]));
}


/**
 * @brief   Receive 16 bit data from Data Register (Rx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  16-bit received data
 * @see     QSPI_ReceiveData8(), QSPI_ReceiveData32()
 */
__STATIC_INLINE uint16_t
QSPI_ReceiveData16 (QSPI_TypeDef *QSPIx) {
    return (uint16_t)(READ_REG(QSPIx->DR[0]));
}


/**
 * @brief   Receive 32 bit data from Data Register (Rx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  32-bit received data
 * @see     QSPI_ReceiveData8(), QSPI_ReceiveData16()
 */
__STATIC_INLINE uint32_t
QSPI_ReceiveData32 (QSPI_TypeDef *QSPIx) {
    return READ_REG(QSPIx->DR[0]);
}


/**
 * @brief   Transmit 8-bit data to Data Register (Tx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_TransmitData16(), QSPI_TransmitData32()
 */
__STATIC_INLINE void
QSPI_TransmitData8 (QSPI_TypeDef *QSPIx, uint8_t TxData) {
    *((__IO uint8_t *)&QSPIx->DR[0]) = TxData;
}


/**
 * @brief   Transmit 16-bit data to Data Register (Tx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_TransmitData8(), QSPI_TransmitData32()
 */
__STATIC_INLINE void
QSPI_TransmitData16 (QSPI_TypeDef *QSPIx, uint16_t TxData) {
    __IO uint16_t* ptr = (__IO uint16_t *)&QSPIx->DR[0];
    *ptr = TxData;
}


/**
 * @brief   Transmit 32-bit data to Data Register (Tx FIFO)
 * @param   QSPIx The QSPI instance
 * @retval  None
 * @see     QSPI_TransmitData8(), QSPI_TransmitData16()
 */
__STATIC_INLINE void
QSPI_TransmitData32 (QSPI_TypeDef *QSPIx, uint32_t TxData) {
    WRITE_REG(QSPIx->DR[0], TxData);
}


/* Initialization and de-initialization functions prototypes ---------------- */

/**
 * @brief   Set each field of QSPI_InitStruct_TypeDef structure to default value
 * @param   QSPI_InitStruct The pointer to the QSPI_InitStruct_TypeDef structure
 * @retval  None
 */
void
QSPI_StructInit(QSPI_InitStruct_TypeDef *QSPI_InitStruct);


/**
 * @brief   Initialize QSPI registers according to the specified parameters
 *          of QSPI_InitStruct structure
 * @param   QSPIx The QSPI instance
 * @param   QSPI_InitStruct Pointer to struct with QSPI configuration parameters
 * @retval  An ErrorStatus enumeration value:
 *              - SUCCESS: QSPI was initialized according to QSPI_InitStruct
 *                         settings
 *              - ERROR:   QSPI is enabled; reconfiguration is not possible
 * @note  QSPI configuratioin is possible only when QSPI is disabled
 */
ErrorStatus
QSPI_Init (QSPI_TypeDef *QSPIx, QSPI_InitStruct_TypeDef *QSPI_InitStruct);


/**
 * @brief Denitialize QSPI peripheral block
 * @param QSPIx           QSPI instance
 * @retval An ErrorStatus enumeration value:
 *          - SUCCESS: QSPI was deinitialized successfully
 *          - ERROR:   Not applicable
 */
ErrorStatus
QSPI_DeInit (QSPI_TypeDef *QSPIx);

#ifdef __cplusplus
}
#endif

#endif // __BMCU_QSPI_H

/**
  ******************************************************************************
  * @file    MDR32FxQI_sdio.h
  * @author  Milandr Application Team
  * @version V2.0.2i
  * @date    10/03/2022
  * @brief   This file contains all the functions prototypes for the SDIO
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
#ifndef __MDR32FxQI_SDIO_H
#define __MDR32FxQI_SDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @addtogroup SDIO
  * @warning This module can be used only for MDR32FG16S1QI microcontroller's.
  * @{
  */

/** @defgroup SDIO_Exported_Types SDIO Exported Types
  * @{
  */

/**
  * @brief Data bus width
  */
typedef enum
{
    SDIO_BusWide_1b = ((uint32_t)0x01),
    SDIO_BusWide_4b = ((uint32_t)0x00)
} SDIO_Bus_Wide_TypeDef;

#define IS_SDIO_BUS_WIDE(WIDE)      (((WIDE) == SDIO_BusWide_1b) || \
                                     ((WIDE) == SDIO_BusWide_4b))

/**
  * @brief Transfer rate control
  */
typedef enum
{
   SDIO_CLOCK_DIV_4   = ((uint32_t)1 << SDIO_CR_BR_Pos), /*!< PCLK/4 */
   SDIO_CLOCK_DIV_8   = ((uint32_t)2 << SDIO_CR_BR_Pos), /*!< PCLK/8 */
   SDIO_CLOCK_DIV_16  = ((uint32_t)3 << SDIO_CR_BR_Pos), /*!< PCLK/16 */
   SDIO_CLOCK_DIV_32  = ((uint32_t)4 << SDIO_CR_BR_Pos), /*!< PCLK/32 */
   SDIO_CLOCK_DIV_64  = ((uint32_t)5 << SDIO_CR_BR_Pos), /*!< PCLK/64 */
   SDIO_CLOCK_DIV_128 = ((uint32_t)6 << SDIO_CR_BR_Pos), /*!< PCLK/128 */
   SDIO_CLOCK_DIV_256 = ((uint32_t)7 << SDIO_CR_BR_Pos)  /*!< PCLK/256 */
} SDIO_Clk_Div_TypeDef;

#define IS_SDIO_CLOCK_DIV(DIV)      (((DIV) & ~SDIO_CR_BR_Msk) == 0)

/**
  * @brief Interrupt enable mask bits
  */
typedef enum
{
    SDIO_IT_TX_FIFO_DATA_EMPTY     = ((uint32_t)SDIO_CR_TXEIE_DAT ),
    SDIO_IT_RX_FIFO_DATA_NOT_EMPTY = ((uint32_t)SDIO_CR_RXNEIE_DAT),
    SDIO_IT_RX_FIFO_DATA_FULL      = ((uint32_t)SDIO_CR_RXFIE_DAT ),
    SDIO_IT_TX_FIFO_CMD_EMPTY      = ((uint32_t)SDIO_CR_TXEIE_DAT ),
    SDIO_IT_RX_FIFO_CMD_EMPTY      = ((uint32_t)SDIO_CR_RXNEIE_CMD),
    SDIO_IT_RX_FIFO_CMD_FULL       = ((uint32_t)SDIO_CR_RXFEIE_CMD)
} SDIO_IT_Enable_TypeDef;

#define SDIO_IT_MSK     ((SDIO_IT_TX_FIFO_DATA_EMPTY)     | \
                         (SDIO_IT_RX_FIFO_DATA_NOT_EMPTY) | \
                         (SDIO_IT_RX_FIFO_DATA_FULL)      | \
                         (SDIO_IT_TX_FIFO_CMD_EMPTY)      | \
                         (SDIO_IT_RX_FIFO_CMD_EMPTY)      | \
                         (SDIO_IT_RX_FIFO_CMD_FULL))

#define IS_SDIO_IT(IT)  ((((IT) & (~SDIO_IT_MSK)) == 0) && (IT != 0))

/**
  * @brief DAT3-DAT0 lines direction selection bit
  */
typedef enum
{
    SDIO_DATA_LINE_OUT = ((uint32_t)SDIO_CR_DIRDATA),
    SDIO_DATA_LINE_IN  = ((uint32_t)0x00000000)
} SDIO_DATA_Line_Dir_TypeDef;

#define IS_DATA_LINE_DIR(DIR)   (((DIR) == SDIO_DATA_LINE_OUT) || \
                                 ((DIR) == SDIO_DATA_LINE_IN))

/**
  * @brief CMD line direction selection bit
  */
typedef enum
{
    SDIO_CMD_LINE_OUT = ((uint32_t)SDIO_CR_DIRCMD),
    SDIO_CMD_LINE_IN  = ((uint32_t)0x00000000)
} SDIO_CMD_Line_Dir_TypeDef;

#define IS_CMD_LINE_DIR(DIR)    (((DIR) == SDIO_CMD_LINE_OUT) || \
                                 ((DIR) == SDIO_CMD_LINE_IN))

/**
  * @brief DATA CRC line
  */
typedef enum
{
    SDIO_DATA_LINE_0 = (0x00),
    SDIO_DATA_LINE_1 = (0x04),
    SDIO_DATA_LINE_2 = (0x08),
    SDIO_DATA_LINE_3 = (0x0C)
} SDIO_DATA_CRC_Line_TypeDef;

#define IS_SDIO_DATA_LINE(data_line)    (((data_line) == SDIO_DATA_LINE_0) || \
                                         ((data_line) == SDIO_DATA_LINE_1) || \
                                         ((data_line) == SDIO_DATA_LINE_2) || \
                                         ((data_line) == SDIO_DATA_LINE_3))

typedef struct
{
  FunctionalState       WaitStartBitCmd;  /*!< Specifies waiting or the formation of a start bit in the command line.
                                               This parameter can be a value of @ref FunctionalState */
  FunctionalState       WaitStartBitData; /*!< Specifies waiting or the formation of a start bit in the data lines.
                                               This parameter can be a value of @ref FunctionalState */
  FunctionalState       CRCCmd;           /*!< Hardware CRC calculation on line CMD.
                                               This parameter can be a value of @ref FunctionalState */
  FunctionalState       CRCData;          /*!< Hardware CRC calculation lines DAT3-DAT0.
                                               This parameter can be a value of @ref FunctionalState */
  FunctionalState       WriteCmd;         /*!< Send a response to a write command 101b.
                                               This parameter can be a value of @ref FunctionalState */
  FunctionalState       EndBusy ;         /*!< Puts BUSY on the line DAT0.
                                               This parameter can be a value of @ref FunctionalState */
  SDIO_Bus_Wide_TypeDef BusWide;          /*!< Specifies the SDIO bus width.
                                               This parameter can be a value of @ref SDIO_Bus_Wide_TypeDef */
  FunctionalState       ClockOutput;      /*!< Enable or disable output clock signal.
                                               This parameter can be a value of @ref FunctionalState */
  SDIO_Clk_Div_TypeDef  ClockDiv;         /*!< Specifies the clock frequency of the SDIO controller.
                                               This parameter can be a value of @ref SDIO_Clk_Div_TypeDef */
} SDIO_InitTypeDef;

/** @} */ /* End of group SDIO_Exported_Types */


/** @defgroup SDIO_Exported_Constants SDIO Exported Constants
  * @{
  */

/** @defgroup SDIO_FLAG SDIO_FLAG
  * @{
  */

#define SDIO_FLAG_CMD_TRANSFER_ACTIVE   ((uint32_t)SDIO_CR_WORK2)
#define SDIO_FLAG_DATA_TRANSFER_ACTIVE  ((uint32_t)SDIO_CR_WORK1)
#define SDIO_FLAG_FIFO_CMD_EMPTY        ((uint32_t)SDIO_SR_FIFOCMD_EMPTY | 0x80000000)
#define SDIO_FLAG_FIFO_DAT_EMPTY        ((uint32_t)SDIO_SR_FIFODAT_EMPTY | 0x80000000)
#define SDIO_FLAG_FIFO_CMD_FULL         ((uint32_t)SDIO_SR_FIFOCMD_FULL  | 0x80000000)
#define SDIO_FLAG_FIFO_DAT_FULL         ((uint32_t)SDIO_SR_FIFODAT_FULL  | 0x80000000)

#define IS_SDIO_FLAG(FLAG)              ((FLAG == SDIO_FLAG_CMD_TRANSFER_ACTIVE ) || \
                                         (FLAG == SDIO_FLAG_DATA_TRANSFER_ACTIVE) || \
                                         (FLAG == SDIO_FLAG_FIFO_CMD_EMPTY      ) || \
                                         (FLAG == SDIO_FLAG_FIFO_DAT_EMPTY      ) || \
                                         (FLAG == SDIO_FLAG_FIFO_CMD_FULL       ) || \
                                         (FLAG == SDIO_FLAG_FIFO_DAT_FULL       ))

/** @} */ /* End of group SDIO_FLAG */

/** @defgroup SDIO_TX_BIT SDIO_TX_BIT
  * @{
  */
#define SD_TX_BIT   0x40

/** @} */ /* End of group SDIO_TX_BIT */

/** @} */ /* End of group SDIO_Exported_Constants */


/** @defgroup SDIO_Exported_Functions SDIO Exported Functions
  * @{
  */

void SDIO_DeInit(void);
void SDIO_StructInit(SDIO_InitTypeDef * SDIO_InitStruct);
void SDIO_Init(SDIO_InitTypeDef * SDIO_InitStruct);
void SDIO_Cmd(FunctionalState NewState);
void SDIO_ItConfig(uint32_t SDIO_IT, FunctionalState NewState);
void SDIO_CMDTxCmd(FunctionalState NewState);
void SDIO_DATATxCmd(FunctionalState NewState);
void SDIO_SetDataLinesDirection(SDIO_DATA_Line_Dir_TypeDef data_direction);
void SDIO_SetCMDLineDirection(SDIO_CMD_Line_Dir_TypeDef cmd_direction);
void SDIO_SetBaudRatePrescaler(SDIO_Clk_Div_TypeDef prescaler);
FlagStatus SDIO_GetFlagStatus(uint32_t SDIO_FLAG);
void SDIO_SendCMD(uint32_t cmd);
void SDIO_SendData(uint32_t data);
uint32_t SDIO_ReceiveCMD(void);
uint32_t SDIO_ReceiveData(void);
void SDIO_ClearCMD_CRC(void);
void SDIO_ClearDataCRC(SDIO_Bus_Wide_TypeDef data_bus_wide);
uint8_t SDIO_GetCRC_CMD(void);
uint16_t SDIO_GetDataCRC(SDIO_DATA_CRC_Line_TypeDef data_line);
void SDIO_SetCMDTransferNumberBit(uint16_t number_bit);
void SDIO_SetDATATransferNumberBit(uint16_t number_bit);
uint16_t SDIO_GetCMDTransferNumberBit(void);
uint16_t SDIO_GetDATATransferNumberBit(void);
void SDIO_SendCommand(uint32_t cmd, uint32_t arg);
void SDIO_SendBlock(uint8_t *buff, uint32_t n_byte);


/** @} */ /* End of group SDIO_Exported_Functions */

/** @} */ /* End of group SDIO */
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_SDIO_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_sdio.h */



/**
  ******************************************************************************
  * @file    MDR32FxQI_sdio.c
  * @author  Milandr Application Team
  * @version V2.0.2i
  * @date    17/03/2022
  * @brief   This file contains all the SDIO firmware functions.
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

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_sdio.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @defgroup SDIO SDIO
  * @warning This module can be used only for MCU MDR32FG16S1QI.
  * @{
  */

/** @defgroup SDIO_Exported_Functions SDIO Exported Functions
  * @{
  */

/**
  * @brief   Resets the SDIO peripheral registers to their default reset values.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None
  * @retval  None
  */
void SDIO_DeInit(void)
{
    MDR_SDIO->CR           = 0;
    MDR_SDIO->SR           = 0;
    MDR_SDIO->CMDDR        = 0;
    MDR_SDIO->DATDR        = 0;
    MDR_SDIO->CMDCRC       = 0;
    MDR_SDIO->DAT0CRC      = 0;
    MDR_SDIO->DAT1CRC      = 0;
    MDR_SDIO->DAT2CRC      = 0;
    MDR_SDIO->DAT3CRC      = 0;
    MDR_SDIO->CMD_TRANSFER = 0;
    MDR_SDIO->DAT_TRANSFER = 0;
}


/**
  * @brief   Fills each SDIO_InitStruct member with its default value.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   SDIO_InitStruct: pointer to a @ref SDIO_InitTypeDef structure
  *          that is to be initialized.
  * @retval  None
  */
void SDIO_StructInit(SDIO_InitTypeDef * SDIO_InitStruct)
{
    SDIO_InitStruct->WaitStartBitCmd  = DISABLE;
    SDIO_InitStruct->WaitStartBitData = DISABLE;
    SDIO_InitStruct->CRCCmd           = DISABLE;
    SDIO_InitStruct->CRCData          = DISABLE;
    SDIO_InitStruct->WriteCmd         = DISABLE;
    SDIO_InitStruct->EndBusy          = DISABLE;
    SDIO_InitStruct->BusWide          = SDIO_BusWide_1b;
    SDIO_InitStruct->ClockOutput      = DISABLE;
    SDIO_InitStruct->ClockDiv         = SDIO_CLOCK_DIV_4;
}

/**
  * @brief   Initializes the SDIO peripheral according to the specified
  *          parameters in the SDIO_InitStruct.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   SDIO_InitStruct: pointer to a @ref SDIO_InitTypeDef structure
  *          that contains the configuration information for the specified SDIO peripheral.
  * @retval  None
  */
void SDIO_Init(SDIO_InitTypeDef * SDIO_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->WaitStartBitCmd));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->WaitStartBitData));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->CRCCmd));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->CRCData));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->WriteCmd));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->EndBusy));
    assert_param(IS_SDIO_BUS_WIDE(SDIO_InitStruct->BusWide));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->ClockOutput));
    assert_param(IS_SDIO_CLOCK_DIV(SDIO_InitStruct->ClockDiv));

    MDR_SDIO->CR = (SDIO_InitStruct->WaitStartBitCmd  << SDIO_CR_SBITCMD_Pos)
                 | (SDIO_InitStruct->WaitStartBitData << SDIO_CR_SBITDAT_Pos)
                 | (SDIO_InitStruct->CRCCmd           << SDIO_CR_CRC_EN_CMD_Pos)
                 | (SDIO_InitStruct->CRCData          << SDIO_CR_CRC_EN_DATA_Pos)
                 | (SDIO_InitStruct->WriteCmd         << SDIO_CR_WRITECMD_Pos)
                 | (SDIO_InitStruct->EndBusy          << SDIO_CR_ENDBUSY_Pos)
                 | (SDIO_InitStruct->BusWide          << SDIO_CR_WIDTHDAT_Pos)
                 | (SDIO_InitStruct->ClockOutput      << SDIO_CR_CLKOE_Pos)
                 | (SDIO_InitStruct->ClockDiv);
}

/**
  * @brief   Enables or disables the specified SDIO peripheral.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - new state of the SDIO peripheral.
  * @retval  None
  */
void SDIO_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_SDIO->CR |= SDIO_CR_SDE;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_CR_SDE;
    }
}

/**
  * @brief   Enables or disables the specified SDIO interrupts.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   SDIO_IT: specifies the SDIO interrupt sources to be enabled or disabled.
  *          This parameter can be any conbination of @ref SDIO_IT_Enable_TypeDef values.
  * @param   NewState - @ref FunctionalState - new state of the specified SDIO interrupts.
  * @retval  None
  */
void SDIO_ItConfig(uint32_t SDIO_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SDIO_IT(SDIO_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_SDIO->CR |= SDIO_IT;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_IT;
    }
}

/**
  * @brief   Start or stop command transaction.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - new status of command transaction.
  * @retval  None
  */
void SDIO_CMDTxCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_SDIO->CR |= SDIO_CR_WORK2;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_CR_WORK2;
    }
}

/**
  * @brief   Start or stop data transaction.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - new status of data transaction.
  * @retval  None
  */
void SDIO_DATATxCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_SDIO->CR |= SDIO_CR_WORK1;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_CR_WORK1;
    }
}

/**
  * @brief   Set the data D0-D3 line direction.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   data_direction - @ref SDIO_DATA_Line_Dir_TypeDef - specifies the D0-D3 line direction.
  * @retval  None
  */
void SDIO_SetDataLinesDirection(SDIO_DATA_Line_Dir_TypeDef data_direction)
{
    /* Check the parameters */
    assert_param(IS_DATA_LINE_DIR(data_direction));

    if(data_direction == SDIO_DATA_LINE_OUT)
    {
        MDR_SDIO->CR |= SDIO_CR_DIRDATA;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_CR_DIRDATA;
    }
}

/**
  * @brief   Set the CMD line direction.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   cmd_direction - @ref SDIO_CMD_Line_Dir_TypeDef - specifies the CMD line direction.
  * @retval  None
  */
void SDIO_SetCMDLineDirection(SDIO_CMD_Line_Dir_TypeDef cmd_direction)
{
    /* Check the parameters */
    assert_param(IS_CMD_LINE_DIR(cmd_direction));

    if(cmd_direction == SDIO_CMD_LINE_OUT)
    {
        MDR_SDIO->CR |= SDIO_CR_DIRCMD;
    }
    else
    {
        MDR_SDIO->CR &= ~SDIO_CR_DIRCMD;
    }
}

/**
  * @brief   Initializes the SDIO peripheral clock according to the specified parameters.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   prescaler - @ref SDIO_Clk_Div_TypeDef - specifies the HCLK division factor.
  * @retval  None
  */
void SDIO_SetBaudRatePrescaler(SDIO_Clk_Div_TypeDef prescaler)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_SDIO_CLOCK_DIV(prescaler));

    /* Clear old prescaler */
    tmpreg = MDR_SDIO->CR & (~SDIO_CR_BR_Msk);

    /* Set new prescaler */
    tmpreg |= (uint32_t)prescaler;

    /* Save new register value */
    MDR_SDIO->CR = tmpreg;
}

/**
  * @brief   Checks whether the specified SDIO flag is set or not.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   SDIO_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *              @arg SDIO_FLAG_CMD_TRANSFER_ACTIVE
  *              @arg SDIO_FLAG_DATA_TRANSFER_ATCIVE
  *              @arg SDIO_FLAG_FIFO_CMD_EMPTY
  *              @arg SDIO_FLAG_FIFO_DAT_EMPTY
  *              @arg SDIO_FLAG_FIFO_CMD_FULL
  *              @arg SDIO_FLAG_FIFO_DAT_FULL
  * @retval  @ref FunctionalState - The state of SDIO_FLAG (SET or RESET).
  */
FlagStatus SDIO_GetFlagStatus(uint32_t SDIO_FLAG)
{
    FlagStatus result;

    /* Check the parameters */
    assert_param(IS_SDIO_FLAG(SDIO_FLAG));

    if(SDIO_FLAG & 0x80000000)
    {
        result = (FlagStatus)((SDIO_FLAG & MDR_SDIO->SR) != 0);
    }
    else
    {
        result = (FlagStatus)((SDIO_FLAG & MDR_SDIO->CR) != 0);
    }

    return (result);
}

/**
  * @brief   Write the command word into the SDIO CMD FIFO.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   cmd: the transmitted command word.
  * @retval  None.
  */
void SDIO_SendCMD(uint32_t cmd)
{
    /* Write cmd in SDIO FIFO CMD */
    MDR_SDIO->CMDDR = cmd;
}

/**
  * @brief   Write the data word into the SDIO DATA FIFO.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   data: the transmitted data word.
  * @retval  None
  */
void SDIO_SendData(uint32_t data)
{
    /* Write data in SDIO FIFO DATA */
    MDR_SDIO->DATDR = data;
}

/**
  * @brief   Returns the most recent received command by the SDIO peripheral.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  The received command.
  */
uint32_t SDIO_ReceiveCMD(void)
{
    return (MDR_SDIO->CMDDR);
}

/**
  * @brief   Returns the most recent received data by the SDIO peripheral.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  The received data.
  */
uint32_t SDIO_ReceiveData(void)
{
    return (MDR_SDIO->DATDR);
}

/**
  * @brief   Clear SDIO CRC command register.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  None
  */
void SDIO_ClearCMD_CRC(void)
{
    MDR_SDIO->CMDCRC = 0;
}

/**
  * @brief   Clear SDIO data register.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   data_bus_wide - @ref SDIO_Bus_Wide_TypeDef - determines
  *          the width of the data bus.
  * @retval  None
  */
void SDIO_ClearDataCRC(SDIO_Bus_Wide_TypeDef data_bus_wide)
{
    /* Check the parameters */
    assert_param(IS_SDIO_BUS_WIDE(data_bus_wide));

    MDR_SDIO->DAT0CRC = 0;

    if(data_bus_wide == SDIO_BusWide_4b)
    {
        MDR_SDIO->DAT1CRC = 0;
        MDR_SDIO->DAT2CRC = 0;
        MDR_SDIO->DAT3CRC = 0;
    }
}

/**
  * @brief   Returns the calculated CRC data value.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  CRC data value.
  */
uint8_t SDIO_GetCRC_CMD(void)
{
    return (MDR_SDIO->CMDCRC);
}

/**
  * @brief   Return the calculated CRC data for the specified number line.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   data_line - @ref SDIO_DATA_CRC_Line_TypeDef - specifies the data line number.
  * @retval  CRC value of the specified data line.
  */
uint16_t SDIO_GetDataCRC(SDIO_DATA_CRC_Line_TypeDef data_line)
{
    uint32_t * ptr_crc;

    /* Check the parameters */
    assert_param(IS_SDIO_DATA_LINE(data_line));

    /* Get pointer to the data line CRC register */
    ptr_crc = (uint32_t *)((uint32_t)&MDR_SDIO->DAT0CRC + ((uint32_t)data_line));

    return (*ptr_crc);
}

/**
  * @brief   Sets the number of bits that need to be transmitted/received
  *          in the line of command.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   number_bit: the number bit to transfer.
  * @retval  None
  */
void SDIO_SetCMDTransferNumberBit(uint16_t number_bit)
{
    MDR_SDIO->CMD_TRANSFER = ((uint32_t)number_bit);
}

/**
  * @brief   Sets the number of bits that need to be
  *          transmitted/received in the line of data.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   number_bit: the number bit to transfer.
  * @retval  None
  */
void SDIO_SetDATATransferNumberBit(uint16_t number_bit)
{
    MDR_SDIO->DAT_TRANSFER = ((uint32_t)number_bit);
}

/**
  * @brief   Gets the number of bits that need to be
  *          transmitted/received in the line of command.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  Remaining number of bits that need to transmit / receive command line.
  */
uint16_t SDIO_GetCMDTransferNumberBit(void)
{
    return (MDR_SDIO->CMD_TRANSFER);
}

/**
  * @brief   Gets the number of bits that need to be
  *          transmitted/received in the line of data.
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   None.
  * @retval  Remaining number of bits that need to transmit / receive data line.
  */
uint16_t SDIO_GetDATATransferNumberBit(void)
{
    return (MDR_SDIO->DAT_TRANSFER);
}

/** 
  * @brief   Send the command from SDIO interface
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   cmd: comand.
  * @param   arg: argument.
  * @retval  None
 */
void SDIO_SendCommand(uint32_t cmd, uint32_t arg)
{
    MDR_SDIO->CR &= ~(SDIO_CR_DIRCMD | SDIO_CR_SBITCMD); /* Clear transaction bits */

    do {                                         /* Wait while CmdActive bit is set */
        MDR_SDIO->CR &= ~SDIO_CR_WORK2;          /* Reset Work2 */
    } while (MDR_SDIO->CR & SDIO_CR_WORK2);

    MDR_SDIO->CR |= SDIO_CR_DIRCMD | SDIO_CR_SBITCMD; /* Command TX & wait start bit */

    //  Write command and arg
    MDR_SDIO->CMDDR  = ((cmd | SD_TX_BIT) & 0x0000007F) |
                        (arg >> 16 & 0x0000FF00) |
                        (arg       & 0x00FF0000) |
                        (arg << 16 & 0xFF000000) ;

    MDR_SDIO->CMDDR  = arg & 0x000000FF;
    MDR_SDIO->CMDCRC = 0x00000000;       /* Clear CRC */
    MDR_SDIO->CMD_TRANSFER = 48;         /* Command length */

    MDR_SDIO->CR |= SDIO_CR_WORK2;       /* Initiate command transaction */

    while ((MDR_SDIO->CR & SDIO_CR_WORK2) != 0){} // Waiting for transaction to complete
}

/**
  * @brief   Send block data SDIO by SDIO
  * @warning This function can be used only for microcontroller series MDR32FG16S1QI.
  * @param   *buff: pointer to array data.
  * @param   n_byte: numbers of bytes.
  * @retval  None
  */
void SDIO_SendBlock(uint8_t *buff, uint32_t n_byte)
{
    uint32_t i = 0;

    MDR_SDIO->CR &= ~(SDIO_CR_DIRDATA | SDIO_CR_SBITDAT); /* Clear transaction bits */

    do {                                  /* Wait while DATActive bit is set */
        MDR_SDIO->CR &= ~SDIO_CR_WORK1;   /* Reset Work1 */
    } while (MDR_SDIO->CR & SDIO_CR_WORK1);

    MDR_SDIO->CR |= SDIO_CR_DIRDATA | SDIO_CR_SBITDAT; /* Data TX & start bit */

    MDR_SDIO->DAT0CRC = 0x00000000;       /* Clear CRC DATA0*/
    MDR_SDIO->DAT1CRC = 0x00000000;       /* Clear CRC DATA1*/
    MDR_SDIO->DAT2CRC = 0x00000000;       /* Clear CRC DATA2*/
    MDR_SDIO->DAT3CRC = 0x00000000;       /* Clear CRC DATA3*/

    while(i < n_byte)
    {
        MDR_SDIO->DATDR = (uint32_t)(buff[i] | buff[i+1] << 8 | buff[i+2] << 16 | buff[i+3] << 24);
        i = i + 4;
    }

    MDR_SDIO->DAT_TRANSFER = n_byte * 8  + (16 + 1); /* Set length */
    MDR_SDIO->CR |= SDIO_CR_WORK1;                   /* Initiate data transaction */

    while( MDR_SDIO->SR & SDIO_SR_FIFODAT_FULL );    /* Write dummy word */
    MDR_SDIO->DATDR = 0x00000000;

    while ((MDR_SDIO->CR & SDIO_CR_WORK1) != 0) {}   // Waiting for transaction to complete
}

/** @} */ /* End of group SDIO_Exported_Functions */

/** @} */ /* End of group SDIO */
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_sdio.c */



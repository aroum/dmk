/**
  ******************************************************************************
  * @file    MDR32FxQI_i2c.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/07/2024
  * @brief   This file contains all the I2C firmware functions.
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
#include "MDR32FxQI_i2c.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/** @defgroup I2C I2C
  * @warning  This module can be used only for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @{
  */

/** @defgroup I2C_Exported_Functions I2C Exported Functions
  * @{
  */

/**
  * @brief  Resets the I2C peripheral registers to their default reset values.
  * @param  None.
  * @retval None.
  */
void I2C_DeInit(void)
{
    MDR_I2C->CTR = 0;
    MDR_I2C->STA = 0;
    MDR_I2C->CMD = I2C_CMD_CLRINT;
    MDR_I2C->PRL = 0;
    MDR_I2C->PRH = 0;
}

/**
  * @brief  Initializes the I2C peripheral according to the specified
  *         parameters in the I2C_InitStruct.
  * @param  I2C_InitStruct: pointer to a @ref I2C_InitTypeDef structure that
  *         contains the configuration information for the specified I2C peripheral.
  * @retval None.
  */
void I2C_Init(const I2C_InitTypeDef* I2C_InitStruct)
{
    uint32_t tmpreg_CTR;
    uint32_t tmpreg_PR;

    /* Check the parameters */
    assert_param(IS_I2C_SPEED(I2C_InitStruct->I2C_Speed));
    assert_param(IS_I2C_CLKDIV(I2C_InitStruct->I2C_ClkDiv));

   /* Form new values */
   tmpreg_CTR = MDR_I2C->CTR;

    if ((I2C_InitStruct->I2C_Speed & I2C_SPEED_Msk) != I2C_SPEED_UP_TO_400KHz)
    {
        tmpreg_CTR |= I2C_CTR_S_I2C;
    }
    else
    {
        tmpreg_CTR &= ~I2C_CTR_S_I2C;
    }

    /* Configure I2C registers with new values */
    MDR_I2C->CTR = tmpreg_CTR;

    tmpreg_PR = I2C_InitStruct->I2C_ClkDiv;
    MDR_I2C->PRL = tmpreg_PR;
    MDR_I2C->PRH = tmpreg_PR >> 8;
}

/**
  * @brief  Fills each I2C_InitStruct member with its default value.
  * @param  I2C_InitStruct: pointer to a @ref I2C_InitTypeDef structure
  *         which will be initialized.
  * @retval None.
  */
void I2C_StructInit(I2C_InitTypeDef* I2C_InitStruct)
{
    /* Reset I2C initialization structure parameters values */
    I2C_InitStruct->I2C_Speed  = I2C_SPEED_UP_TO_400KHz;
    I2C_InitStruct->I2C_ClkDiv = 1;
}

/**
  * @brief  Enables or disables the I2C peripheral.
  * @param  NewState - @ref FunctionalState - new state of the I2C peripheral.
  * @retval None.
  */
void I2C_Cmd(FunctionalState NewState)
{
    uint32_t tmpreg_CTR;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CTR = MDR_I2C->CTR;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable I2C by setting the EN_I2C bit in the CTR register */
        tmpreg_CTR |= I2C_CTR_EN_I2C;
    }
    else
    {
        /* Disable I2C by resetting the EN_I2C bit in the CTR register */
        tmpreg_CTR &= ~I2C_CTR_EN_I2C;
    }

    /* Configure CTR register with new value */
    MDR_I2C->CTR = tmpreg_CTR;
}

/**
  * @brief  Enables or disables the I2C interrupts.
  * @param  NewState - @ref FunctionalState - new state of the I2C interrupts.
  * @retval None.
  */
void I2C_ITConfig(FunctionalState NewState)
{
    uint32_t tmpreg_CTR;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CTR = MDR_I2C->CTR;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the I2C interrupts by setting the EN_INT bit in the CTR register */
        tmpreg_CTR |= I2C_CTR_EN_INT;
    }
    else
    {
        /* Disable the I2C interrupts by resetting the EN_INT bit in the CTR register */
        tmpreg_CTR &= ~I2C_CTR_EN_INT;
    }

    /* Configure CTR register with new value */
    MDR_I2C->CTR = tmpreg_CTR;
}

/**
  * @brief  Checks whether the I2C interrupt has occurred or not.
  * @param  None.
  * @retval @ref ITStatus - The current state of I2C_STA_INT (SET or RESET).
  */
ITStatus I2C_GetITStatus(void)
{
    ITStatus bitstatus;

    if ((MDR_I2C->STA & I2C_STA_INT) == I2C_STA_INT)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}

/**
  * @brief  Clears the I2C_STA_INT interrupt pending bit.
  * @param  None.
  * @retval None.
  */
void I2C_ClearITPendingBit(void)
{
    MDR_I2C->CMD |= I2C_CMD_CLRINT;
}

/**
  * @brief  Transmits the address byte to select the Slave device.
  * @param  Address: specifies the Slave address which will be transmitted.
  *         The Address[0] bit value is ignored. So only Address[7..1] bits
  *         are the seven-bit Slave address.
  * @param  Direction - @ref I2C_Transfer_Direction - specifies whether
  *         the I2C device will be a Transmitter or a Receiver.
  * @retval None.
  */
void I2C_Send7bitAddress(uint8_t Address, I2C_Transfer_Direction Direction)
{
    /* Check the parameters */
    assert_param(IS_I2C_DIRECTION(Direction));

    MDR_I2C->TXD = (Address & ~I2C_Direction_Msk) | Direction;
    MDR_I2C->CMD = I2C_CMD_START | I2C_CMD_WR;
}

/**
  * @brief  Sends a byte through the I2C peripheral.
  * @param  ByteToSend: Byte to be transmitted.
  * @retval None.
  */
void I2C_SendByte(uint8_t ByteToSend)
{
    MDR_I2C->TXD = ByteToSend;
    MDR_I2C->CMD = I2C_CMD_WR;
}

/**
  * @brief  Generates I2C communication STOP condition.
  * @param  None.
  * @retval None.
  */
void I2C_SendSTOP(void)
{
    MDR_I2C->CMD = I2C_CMD_STOP;
}

/**
  * @brief  Starts data receiving from Slave.
  * @param  Acknowledge - @ref I2C_M2S_Acknowledge - specifies
  *         the acknowledge state at end of data receiving.
  * @retval None.
  */
void I2C_StartReceiveData(I2C_M2S_Acknowledge Acknowledge)
{
    /* Check the parameters */
    assert_param(IS_I2C_SEND_ACKNOWLEDGE(Acknowledge));

    MDR_I2C->CMD = I2C_CMD_RD | Acknowledge;
}

/**
  * @brief  Returns received from Slave data byte.
  * @param  None.
  * @retval Received from data byte.
  */
uint8_t I2C_GetReceivedData(void)
{
    return (uint8_t)(MDR_I2C->RXD);
}

/**
  * @brief  Checks whether the last I2C Event is equal
  *         to the one passed as parameter.
  * @param  Event: specifies the event to be checked.
  *         This parameter can be one of the values discribed in
  *         @ref I2C_EVENTS or user specified combination of flags
  *         (@ref I2C_FLAGS).
  * @note   For detailed description of Events, please refer to section
  *         @ref I2C_EVENTS in MDR32FxQI_i2c.h file.
  * @retval An ErrorStatus enumuration value:
  *         - SUCCESS: Last event is equal to the Event;
  *         - ERROR: Last event is different from the Event.
  */
ErrorStatus I2C_CheckEvent(uint32_t Event)
{
    ErrorStatus errstatus;

    /* Check the parameters */
    assert_param(IS_I2C_FLAGS(Event));

    if ((I2C_GetLastEvent() & Event) == Event)
    {
        errstatus = SUCCESS;
    }
    else
    {
        errstatus = ERROR;
    }

    return errstatus;
}

/**
  * @brief  Returns the last I2C Event.
  * @param  None.
  * @retval The last event
  * @note   For detailed description of Events, please refer to section
  *         @ref I2C_EVENTS in MDR32FxQI_i2c.h file.
  */
uint32_t I2C_GetLastEvent(void)
{
    uint32_t event;

    event = MDR_I2C->CMD | (MDR_I2C->STA << 8);
    event |= ~event << 16;

    return event;
}

/**
  * @brief  Checks whether the specified I2C flag is set or not.
  * @param  Flag: specifies the flag to check.
  *         This parameter can be one of the values:
  *           @arg I2C_FLAG_CMD_NACK           Master will be send NACK after reading from Slave;
  *           @arg I2C_FLAG_CMD_ACK            Master will be send ACK after reading from Slave;
  *           @arg I2C_FLAG_CMD_WR             The Master to Slave transfer in progress;
  *           @arg I2C_FLAG_CMD_nWR            No the Master to Slave transfer;
  *           @arg I2C_FLAG_CMD_RD             The Slave to Master transfer in progress;
  *           @arg I2C_FLAG_CMD_nRD            No the Slave to Master transfer;
  *           @arg I2C_FLAG_STOP               STOP condition transfer is selected;
  *           @arg I2C_FLAG_nSTOP              STOP condition is resetted;
  *           @arg I2C_FLAG_START              START condition transfer is selected;
  *           @arg I2C_FLAG_nSTART             START condition is resetted;
  *           @arg I2C_FLAG_IRQ                The I2C interrupt pending flag is set;
  *           @arg I2C_FLAG_nIRQ               The I2C interrupt pending flag is cleared;
  *           @arg I2C_FLAG_TRANS              transfer in progress;
  *           @arg I2C_FLAG_nTRANS             No transfer;
  *           @arg I2C_FLAG_LOST_ARB           Arbitration is lost at last byte transfer;
  *           @arg I2C_FLAG_ARB_OK             Arbitration is Ok at last byte transfer;
  *           @arg I2C_FLAG_BUS_BUSY           Some device holds the I2C bus;
  *           @arg I2C_FLAG_BUS_FREE           I2C bus is free (SDA = SCL = 1);
  *           @arg I2C_FLAG_SLAVE_NACK         Not Acknowledge (NACK) received from Slave;
  *           @arg I2C_FLAG_SLAVE_ACK          Acknowledge (ACK) received from Slave.
  * @retval The new state of I2C_FLAG (SET or RESET).
  */
FlagStatus I2C_GetFlagStatus(uint32_t Flag)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_I2C_FLAG(Flag));

    if ((I2C_GetLastEvent() & Flag) != 0)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}

/** @} */ /* End of group I2C_Exported_Functions */

#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group I2C */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_i2c.c */



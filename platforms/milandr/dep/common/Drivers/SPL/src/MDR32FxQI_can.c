/**
  ******************************************************************************
  * @file    MDR32FxQI_can.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the CAN firmware functions.
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
#include "MDR32FxQI_can.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)

/** @defgroup CAN CAN
  * @{
  */

/** @defgroup CAN_Private_Defines CAN Private Defines
  * @{
  */

#define IS_CAN_ALL_PERIPH(PERIPH)  (((PERIPH) == MDR_CAN1) || \
                                    ((PERIPH) == MDR_CAN2))

/** @} */ /* End of group CAN_Private_Defines */


/** @defgroup CAN_Exported_Functions CAN Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the CANx peripheral registers to their default reset values.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @retval None
  */
void CAN_DeInit(MDR_CAN_TypeDef* CANx)
{
    uint32_t i;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));

    CANx->CONTROL = 0;
    CANx->STATUS = 0;
    CANx->BITTMNG = 0;
    CANx->INT_EN = 0;
    CANx->OVER = 0;
    CANx->INT_RX = 0;
    CANx->INT_TX = 0;

    for (i = 0; i < 32; i++)
    {
        CANx->BUF_CON[i] = 0;
    }
}

/**
  * @brief  Initializes the CANx peripheral according to the specified
  *         parameters in the CAN_InitStruct.
  * @note   This function may be called only if appropriate CANx controller
  *         is in the disabled state.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  CAN_InitStruct: pointer to a @ref CAN_InitTypeDef structure
  *         that contains the configuration information for the specified CAN peripheral.
  * @retval None
  */
void CAN_Init(MDR_CAN_TypeDef* CANx, const CAN_InitTypeDef* CAN_InitStruct)
{
    uint32_t tmpreg_CONTROL = 0;
    uint32_t tmpreg_BITTMNG;
    uint32_t i;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_ROP));
    assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_SAP));
    assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_STM));
    assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_ROM));
    assert_param(IS_CAN_PSEG_VALUE(CAN_InitStruct->CAN_PSEG));
    assert_param(IS_CAN_SEG1_VALUE(CAN_InitStruct->CAN_SEG1));
    assert_param(IS_CAN_SEG2_VALUE(CAN_InitStruct->CAN_SEG2));
    assert_param(IS_CAN_SJW_VALUE(CAN_InitStruct->CAN_SJW));
    assert_param(IS_CAN_SAMPLING_MODE(CAN_InitStruct->CAN_SB));

    /* Configure CONTROL register*/
    if (CAN_InitStruct->CAN_ROP != DISABLE)
    {
        tmpreg_CONTROL |= CAN_CONTROL_ROP;
    }
    if (CAN_InitStruct->CAN_SAP != DISABLE)
    {
        tmpreg_CONTROL |= CAN_CONTROL_SAP;
    }
    if (CAN_InitStruct->CAN_STM != DISABLE)
    {
        tmpreg_CONTROL |= CAN_CONTROL_STM;
    }
    if (CAN_InitStruct->CAN_ROM != DISABLE)
    {
        tmpreg_CONTROL |= CAN_CONTROL_ROM;
    }
    CANx->CONTROL = tmpreg_CONTROL;

    /* Configure BITTMNG register*/
    tmpreg_BITTMNG = CAN_InitStruct->CAN_PSEG + CAN_InitStruct->CAN_SEG1 +
                     CAN_InitStruct->CAN_SEG2 + CAN_InitStruct->CAN_SJW +
                     (uint32_t)(CAN_InitStruct->CAN_BRP);
    tmpreg_BITTMNG |= CAN_InitStruct->CAN_SB;
    CANx->BITTMNG = tmpreg_BITTMNG;

    /* Initialize Filters */
    for (i = 0; i < 32; i++)
    {
        CANx->CAN_BUF_FILTER[i].FILTER = 0;
        CANx->CAN_BUF_FILTER[i].MASK = 0;
    }

    /* Configure OVER register*/
    CANx->OVER = (uint32_t)(CAN_InitStruct->CAN_OVER_ERROR_MAX);
}

/**
  * @brief  Fills each CAN_InitStruct member with its default value.
  * @param  CAN_InitStruct: pointer to a @ref CAN_InitTypeDef structure
  *         that is to be initialized.
  * @retval None
  */
void CAN_StructInit(CAN_InitTypeDef* CAN_InitStruct)
{
    /* Reset CAN init structure parameters values */
    /* Own packet reception: disabled */
    CAN_InitStruct->CAN_ROP = DISABLE;
    /* Sending ACK on own packets: disabled */
    CAN_InitStruct->CAN_SAP = DISABLE;
    /* Self Test mode: disabled */
    CAN_InitStruct->CAN_STM = DISABLE;
    /* Read Only mode: disabled */
    CAN_InitStruct->CAN_ROM = DISABLE;
    /* CAN_PSEG member */
    CAN_InitStruct->CAN_PSEG = CAN_PSEG_Mul_1TQ;
    /* CAN_SEG1 member */
    CAN_InitStruct->CAN_SEG1 = CAN_SEG1_Mul_2TQ;
    /* CAN_SEG2 member */
    CAN_InitStruct->CAN_SEG2 = CAN_SEG2_Mul_6TQ;
    /* CAN_SEG2 member */
    CAN_InitStruct->CAN_SJW = CAN_SJW_Mul_1TQ;
    /* Sampling mode member */
    CAN_InitStruct->CAN_SB = CAN_SB_1_SAMPLE;
    /* CAN_BRP member */
    CAN_InitStruct->CAN_BRP = 0;
    /* CAN_OVER_ERROR_MAX member */
    CAN_InitStruct->CAN_OVER_ERROR_MAX = 255;
}

/**
  * @brief  Initializes the CANx Buffer filter and mask according to
  *         the specified parameters in the CAN_FilterInitStruct.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for reception.
  * @param  CAN_FilterInitStruct: pointer to a @ref CAN_FilterInitTypeDef structure
  *         that contains the configuration information.
  * @retval None.
  */
void CAN_FilterInit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_FilterInitTypeDef* CAN_FilterInitStruct)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_CAN_ID(CAN_FilterInitStruct->Filter_ID));
    assert_param(IS_CAN_ID(CAN_FilterInitStruct->Mask_ID));

    CANx->CAN_BUF_FILTER[BufferNumber].FILTER  = CAN_FilterInitStruct->Filter_ID;
    CANx->CAN_BUF_FILTER[BufferNumber].MASK    = CAN_FilterInitStruct->Mask_ID;
}

/**
  * @brief  Enables or disables the specified CAN interrupts.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  CAN_IT: specifies the CAN interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of @ref CAN_IT_Def values.
  * @param  NewState: new state of the CAN interrupts.
  * @retval None.
  */
void CAN_ITConfig(MDR_CAN_TypeDef* CANx, uint32_t CAN_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_ITConfig(CAN_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        CANx->INT_EN |= CAN_IT;
    }
    else
    {
        CANx->INT_EN &= ~CAN_IT;
    }
}

/**
  * @brief  Searches for disabled (i.e., not currently used for reception
  *         or transmission) buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @retval Number of the first found (with lesser number) disabled buffer
  *         or @ref CAN_BUFFER_NUMBER (= 32) if there is no such a buffer.
  */
uint32_t CAN_GetDisabledBuffer(MDR_CAN_TypeDef* CANx)
{
    uint32_t buffer_number;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));

    for (buffer_number = 0;
         ((buffer_number < CAN_BUFFER_NUMBER) && ((CANx->BUF_CON[buffer_number] & CAN_STATUS_EN) != 0));
         buffer_number++);

    return buffer_number;
}

/**
  * @brief  Searches for an empty transfer buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @retval Number of the first found (with lesser number) empty buffer
  *         or CAN_BUFFER_NUMBER (= 32) if there is no such a buffer.
  */
uint32_t CAN_GetEmptyTransferBuffer(MDR_CAN_TypeDef* CANx)
{
    uint32_t buffer_number;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));

    for (buffer_number = 0; (buffer_number < CAN_BUFFER_NUMBER) &&
         ((CANx->BUF_CON[buffer_number] & (CAN_STATUS_EN | CAN_STATUS_TX_REQ | CAN_STATUS_RX_TXn)) != CAN_STATUS_EN);
         buffer_number++);

    return buffer_number;
}

/**
  * @brief  Initiates the transmission of a message.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for transmission.
  * @param  TxMessage: pointer to a @ref CAN_TxMsgTypeDef structure.
  * @retval None
  */
void CAN_Transmit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_TxMsgTypeDef* TxMessage)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_CAN_IDTYPE(TxMessage->IDE));
    assert_param(IS_CAN_DLC(TxMessage->DLC));
    assert_param(IS_FUNCTIONAL_STATE(TxMessage->PRIOR_0));
    assert_param(IS_CAN_ID(TxMessage->ID));

    /* Configure CAN_BUF_ID and CAN_BUF_DLC registers */
    /* Packet format */
    if (TxMessage->IDE == CAN_ID_STD)
    {
        /* Standard */
        tmpreg = CAN_BUF_DLC_STD + TxMessage->DLC;
    }
    else
    {
        /* Extended */
        tmpreg = CAN_BUF_DLC_EXT + TxMessage->DLC;
    }
    CANx->CAN_BUF[BufferNumber].DLC = tmpreg;
    CANx->CAN_BUF[BufferNumber].ID  = TxMessage->ID;

    /* Buffer data unit */
    CANx->CAN_BUF[BufferNumber].DATAL = TxMessage->Data[0];
    CANx->CAN_BUF[BufferNumber].DATAH = TxMessage->Data[1];

    /* Configure CAN_BUF_CON register */
    tmpreg = 0;
    /* Transmission priority */
    if (TxMessage->PRIOR_0 != DISABLE)
    {
        tmpreg |= CAN_BUF_CON_PRIOR_0;
    }
    /* Set transmission request bit */
    tmpreg |= CAN_BUF_CON_TX_REQ;
    /* Enable buffer */
    tmpreg |= CAN_BUF_CON_EN;

    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Initiates the Remote Transfer Request.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for transmission.
  * @param  RTRMessage: pointer to a @ref CAN_RTRMessageTypeDef structure.
  * @retval None
  */
void CAN_SendRTR(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RTRMessageTypeDef* RTRMessage)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_CAN_IDTYPE(RTRMessage->IDE));
    assert_param(IS_FUNCTIONAL_STATE(RTRMessage->PRIOR_0));
    assert_param(IS_CAN_ID(RTRMessage->ID));

    /* Configure CAN_BUF_ID and CAN_BUF_DLC registers */
    /* Packet format */
    if (RTRMessage->IDE == CAN_ID_STD)
    {
        /* Standard */
        tmpreg = CAN_BUF_DLC_STD;
    }
    else
    {
        /* Extended */
        tmpreg = CAN_BUF_DLC_EXT;
    }
    CANx->CAN_BUF[BufferNumber].DLC = tmpreg + CAN_RTR_REMOTE;
    CANx->CAN_BUF[BufferNumber].ID  = RTRMessage->ID;

    /* Configure CAN_BUF_CON register */
    tmpreg = 0;
    /* Transmission priority */
    if (RTRMessage->PRIOR_0 != DISABLE)
    {
        tmpreg |= CAN_BUF_CON_PRIOR_0;
    }
    /* Set transmission request bit */
    tmpreg |= CAN_BUF_CON_TX_REQ;
    /* Enable buffer */
    tmpreg |= CAN_BUF_CON_EN;

    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Initiates the Remote Transfer Request reply.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for transmission.
  * @param  RTRReplyMessage: pointer to a @ref CAN_RTRReplyTypeDef structure.
  * @retval None
  */
void CAN_RTRReply(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RTRReplyTypeDef* RTRReplyMessage)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_CAN_IDTYPE(RTRReplyMessage->IDE));
    assert_param(IS_CAN_ID(RTRReplyMessage->ID));
    assert_param(IS_CAN_DLC(RTRReplyMessage->DLC));

    /* Configure CAN_BUF_ID and CAN_BUF_DLC registers */
    /* Configure CAN_BUF_ID and CAN_BUF_DLC registers */
    /* Packet format */
    if (RTRReplyMessage->IDE == CAN_ID_STD)
    {
        /* Standard */
        tmpreg = CAN_BUF_DLC_STD + RTRReplyMessage->DLC;
    }
    else
    {
        /* Extended */
        tmpreg = CAN_BUF_DLC_EXT + RTRReplyMessage->DLC;
    }
    CANx->CAN_BUF[BufferNumber].DLC = tmpreg;
    CANx->CAN_BUF[BufferNumber].ID  = RTRReplyMessage->ID;

    /* Buffer data unit */
    CANx->CAN_BUF[BufferNumber].DATAL = RTRReplyMessage->Data[0];
    CANx->CAN_BUF[BufferNumber].DATAH = RTRReplyMessage->Data[1];

    /* Configure CAN_BUF_CON register */
    /* Set RTR_EN bit */
    tmpreg = CAN_STATUS_RTR_EN;
    /* Enable buffer */
    tmpreg |= CAN_BUF_CON_EN;

    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Starts the waiting for the receiving of a message.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for reception.
  * @param  OverWrite - @ref FunctionalState - Enables or disables overwrite mode.
  * @retval None
  */
void CAN_Receive(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, FunctionalState OverWrite)
{
    uint32_t tmpreg = 0;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_FUNCTIONAL_STATE(OverWrite));

    /* Configure CAN_BUF_CON register */
    /* Overwrite mode */
    if (OverWrite != DISABLE)
    {
        tmpreg = CAN_BUF_CON_OVER_EN;
    }
    /* Configure buffer for receiving and enable it */
    tmpreg |= CAN_BUF_CON_RX_TXN | CAN_BUF_CON_EN;

    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Reads the received data from buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for reception.
  * @param  RxBuffer: @ref CAN_DataTypeDef array to place received data to.
  * @retval None
  */
void CAN_GetReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_DataTypeDef RxBuffer)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    RxBuffer[0] = CANx->CAN_BUF[BufferNumber].DATAL;
    RxBuffer[1] = CANx->CAN_BUF[BufferNumber].DATAH;
}

/**
  * @brief  Reads received message (containing both header and data) from buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer that is used for reception.
  * @param  RxMessage: pointer to a @ref CAN_RxMsgTypeDef.
  * @retval None
  */
void CAN_GetRawReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_RxMsgTypeDef* RxMessage)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    /* Get the DLC */
    tmpreg = CANx->CAN_BUF[BufferNumber].DLC;

    RxMessage->Rx_Header.DLC = (uint8_t)(tmpreg & CAN_DLC_DATA_LENGTH);
    /* Get the IDE */
    RxMessage->Rx_Header.IDE = CAN_ID_STD;
    if ((tmpreg & CAN_DLC_IDE) != 0)
    {
        RxMessage->Rx_Header.IDE = CAN_ID_EXT;
    }
    /* Get the OVER_EN */
    RxMessage->Rx_Header.OVER_EN = DISABLE;
    tmpreg = CANx->BUF_CON[BufferNumber];
    if ((tmpreg & CAN_BUF_CON_OVER_EN) != 0)
    {
        RxMessage->Rx_Header.OVER_EN = ENABLE;
    }
    /* Get the Id */
    RxMessage->Rx_Header.ID = CANx->CAN_BUF[BufferNumber].ID;

    /* Get the data field */
    RxMessage->Data[0] = CANx->CAN_BUF[BufferNumber].DATAL;
    RxMessage->Data[1] = CANx->CAN_BUF[BufferNumber].DATAH;
}

/**
  * @brief  Releases the buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: the number of the buffer to be released.
  * @retval None.
  */
void CAN_BufferRelease(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    tmpreg = CANx->BUF_CON[BufferNumber];
    tmpreg &= ~CAN_STATUS_EN;
    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Enables or disables the specified CAN peripheral.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  NewState - @ref FunctionalState - new state of the CANx peripheral.
  * @retval None
  */
void CAN_Cmd(MDR_CAN_TypeDef* CANx, FunctionalState NewState)
{
    uint32_t tmpreg_CONTROL;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CONTROL = CANx->CONTROL;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable CANx by setting the CAN_EN bit in the CONTROL register */
        tmpreg_CONTROL |= CAN_CONTROL_CAN_EN;
    }
    else
    {
        /* Disable CANx by resetting the CAN_EN bit in the CONTROL register */
        tmpreg_CONTROL &= ~CAN_CONTROL_CAN_EN;
    }

    /* Configure CONTROL register with new value */
    CANx->CONTROL = tmpreg_CONTROL;
}

/**
  * @brief  Returns the CANx Status Register value.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @retval The CANx_STATUS Register value.
  */
uint32_t CAN_GetStatus(MDR_CAN_TypeDef* CANx)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));

    return CANx->STATUS;
}

/**
  * @brief  Returns the CANx_BUF_xx_CON Register value.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: The number of the buffer.
  * @retval The CANx_BUF_xx_CON Register value.
  */
uint32_t CAN_GetBufferStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    return CANx->BUF_CON[BufferNumber];
}

/**
  * @brief  Checks whether the specified CAN interrupt enable or disable.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  CAN_IT - @ref CAN_IT_Def - specifies the CAN interrupt sources to be enabled or disabled.
  * @retval @ref ITStatus - The state of CAN_IT (SET or RESET).
  */
ITStatus CAN_GetITState(MDR_CAN_TypeDef* CANx, CAN_IT_Def CAN_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_ITConfig(CAN_IT));

    if ((CANx->INT_EN & CAN_IT) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Clears the CANx interrupt errors pending bits.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  Status_Flag: specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
            CAN_STATUS_ERROR_OVER:    Flag indicating that TEC or REC exceeds ERROR_MAX value
            CAN_STATUS_BIT_ERR:       Transmitting frame bits error flag
            CAN_STATUS_BIT_STUFF_ERR: Staff frame bits error flag
            CAN_STATUS_CRC_ERR:       Frame CRC error flag
            CAN_STATUS_FRAME_ERR:     Frame format error flag
            CAN_STATUS_ACK_ERR:       Reception acknowledge error flag
  * @retval None.
  */
void CAN_ITClearErrorPendingBit(MDR_CAN_TypeDef* CANx, uint32_t Status_Flag)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_IT_ERROR_FLAG(Status_Flag));

    CANx->STATUS &= ~Status_Flag;
}

/**
  * @brief  Enables or disables interrupts from the reception buffers.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  Buffer_IT: specifies the reception buffers to enable or disable interrupts of.
  *         Each buffer is designated by bit number in the mask. See @CAN_Buffer_Interrupts
  *         group.
  * @param  NewState - @ref FunctionalState - new state of the reception buffer interrupts.
  * @retval None.
  */
void CAN_RxITConfig(MDR_CAN_TypeDef* CANx, uint32_t Buffer_IT, FunctionalState NewState)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg = CANx->INT_RX;

    if (NewState != DISABLE)
    {
        tmpreg |= Buffer_IT;
    }
    else
    {
        tmpreg &= ~Buffer_IT;
    }

    CANx->INT_RX = tmpreg;
}

/**
  * @brief  Checks whether the specified reception buffer interrupt has occurred or not.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: The number of the buffer
  * @retval @ref ITStatus - Current state of RX_FULL[BufferNumber] flag (SET or RESET).
  */
ITStatus CAN_GetRxITStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    if ((CANx->RX & CANx->INT_RX & (((uint32_t)0x1) << BufferNumber)) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Returns the CANx_Rx Register value.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @retval The CANx_Rx Register value.
  */
uint32_t CAN_GetRx(MDR_CAN_TypeDef* CANx)
{
    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));

    return CANx->RX;
}

/**
  * @brief  Enables or disables interrupts from the transmission buffers.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  Buffer_IT: specifies the transmission buffers to enable or disable interrupts of.
  *         Each buffer is designated by bit number in the mask. See @CAN_Buffer_Interrupts group.
  * @param  NewState - @ref FunctionalState - new state of the reception buffer interrupts.
  * @retval None.
  */
void CAN_TxITConfig(MDR_CAN_TypeDef* CANx, uint32_t Buffer_IT, FunctionalState NewState)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg = CANx->INT_TX;

    if (NewState != DISABLE)
    {
        tmpreg |= Buffer_IT;
    }
    else
    {
        tmpreg &= ~Buffer_IT;
    }

    CANx->INT_TX = tmpreg;
}

/**
  * @brief  Checks whether the specified transmission buffer interrupt has occurred or not.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: The number of the buffer
  * @retval @ref ITStatus - Current state of ~TX_REQ[BufferNumber] flag (SET or RESET).
  */
ITStatus CAN_GetTxITStatus(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));

    if ((CANx->TX & CANx->INT_TX & (((uint32_t)0x1) << BufferNumber)) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Clears the CANx transmission or reception buffer interrupt pending bits.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  BufferNumber: The number of the buffer
  * @param  Status_Flag: specifies the interrupt pending bit to clear.
  *         This parameter can be of the following values:
  *         @arg CAN_STATUS_RX_READY: Flag indicating that there are messages received
  *         @arg CAN_STATUS_TX_READY: Flag indicating that there are buffers for transmitting
  * @retval None.
  */
void CAN_ITClearRxTxPendingBit(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, uint32_t Status_Flag)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_BUFFER(BufferNumber));
    assert_param(IS_CAN_IT_RXTX_FLAG(Status_Flag));

    tmpreg = CANx->BUF_CON[BufferNumber];

    if (Status_Flag == CAN_STATUS_RX_READY)
    {
        tmpreg &= ~CAN_STATUS_RX_FULL;
    }
    else if (Status_Flag == CAN_STATUS_TX_READY)
    {
        tmpreg |= CAN_STATUS_TX_REQ;
    }

    CANx->BUF_CON[BufferNumber] = tmpreg;
}

/**
  * @brief  Initializes the CANx peripheral Clock according to the
  *         specified parameters.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values: MDR_CAN1, MDR_CAN2.
  * @param  CAN_BRG - @ref CAN_Clock_BRG - specifies the HCLK division factor.
  * @retval None
  */
void CAN_BRGInit(MDR_CAN_TypeDef* CANx, CAN_Clock_BRG CAN_BRG)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_CAN_ALL_PERIPH(CANx));
    assert_param(IS_CAN_CLOCK_BRG(CAN_BRG));

    tmpreg = MDR_RST_CLK->CAN_CLOCK;

    if (CANx == MDR_CAN1)
    {
        tmpreg |= RST_CLK_CAN_CLOCK_CAN1_CLK_EN;
        tmpreg &= ~RST_CLK_CAN_CLOCK_CAN1_BRG_Msk;
        tmpreg |= CAN_BRG;
    }
    else if (CANx == MDR_CAN2)
    {
        tmpreg |= RST_CLK_CAN_CLOCK_CAN2_CLK_EN;
        tmpreg &= ~RST_CLK_CAN_CLOCK_CAN2_BRG_Msk;
        tmpreg |= (CAN_BRG << 8);
    }

    MDR_RST_CLK->CAN_CLOCK = tmpreg;
}

/** @} */ /* End of group CAN_Exported_Functions */

/** @} */ /* End of group CAN */

#endif /* #if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_can.c */


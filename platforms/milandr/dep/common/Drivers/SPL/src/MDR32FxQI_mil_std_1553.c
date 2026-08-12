/**
  ******************************************************************************
  * @file    MDR32FxQI_mil_std_1553.c
  * @author  Milandr Application Team
  * @version V2.0.4i
  * @date    24/07/2024
  * @brief   This file contains all the MIL STD 1553 firmware functions.
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
#include "MDR32FxQI_mil_std_1553.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE1xI)
/** @defgroup MIL_STD_1553 MIL_STD_1553
 *  @warning This module can be used only for MCU K1986VE1xI.
  * @{
  */

/** @defgroup MIL_STD_1553_Private_Defines MIL_STD_1553 Private Defines
  * @{
  */

#define IS_MIL_STD_1553_SUBADDRES(SUBADDRES)  (((SUBADDRES) != 0) && \
                                               ((SUBADDRES) <= 0x1F) )

#define IS_MIL_STD_1553_NUMBER_DATA_WORDS(NUMBER_DATA_WORDS)  (((NUMBER_DATA_WORDS) != 0 ) && \
                                                               ((NUMBER_DATA_WORDS) <=32))

/** @} */ /* End of group MIL_STD_1553_Private_Defines */


/** @defgroup MIL_STD_1553_Exported_Functions MIL_STD_1553 Exported Functions
  * @{
  */

/**
  * @brief  Initializes the MIL_STD_1553 peripheral Clock according to the specified parameters.
  * @param  MIL_STD_1553_BRG_Value - @ref MIL_STD_1553_BRG - specifies the HCLK division factor.
  * @retval None
  */
void  MIL_STD_1553_BRGInit(MIL_STD_1553_BRG MIL_STD_1553_BRG_Value)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_BRG(MIL_STD_1553_BRG_Value));

    tmpreg = MDR_RST_CLK->ETH_CLOCK;

    tmpreg |= RST_CLK_ETH_CLOCK_MAN_CLK_EN;
    tmpreg &= ~RST_CLK_ETH_CLOCK_MAN_BRG_Msk;
    tmpreg |= ((uint32_t)MIL_STD_1553_BRG_Value << RST_CLK_ETH_CLOCK_MAN_BRG_Pos);

    /* Store the new value */
    MDR_RST_CLK->ETH_CLOCK = tmpreg;
}

/**
  * @brief  Reset the MIL_STD_1553x peripheral registers to their default reset values.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval None
  */
void MIL_STD_1553_DeInit(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    MIL_STD_1553x->CONTROL      = 0x1;
    MIL_STD_1553x->INTEN        = 0x0;
    MIL_STD_1553x->CommandWord1 = 0x0;
    MIL_STD_1553x->CommandWord2 = 0x0;
    MIL_STD_1553x->ModeData     = 0x0;
    MIL_STD_1553x->StatusWord1  = 0x0;
    MIL_STD_1553x->StatusWord2  = 0x0;
}

/**
  * @brief  Initializes the MIL_STD_1553x peripheral according to the specified
  *         parameters in the MIL_STD_1553_InitStruct.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  MIL_STD_1553_InitStruct: pointer to a @ref MIL_STD_1553_InitTypeDef structure
  *         that contains the configuration information for the specified MIL_STD_1553 peripheral.
  * @retval None
  */
void MIL_STD_1553_Init(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_InitTypeDef * MIL_STD_1553_InitStruct)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_MODE(MIL_STD_1553_InitStruct->MIL_STD_1553_Mode));
    assert_param(IS_MIL_STD_1553_DIVIDER(MIL_STD_1553_InitStruct->MIL_STD_1553_DIV));
    assert_param(IS_MIL_STD_1553_RTA(MIL_STD_1553_InitStruct->MIL_STD_1553_RTA));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_RERR));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_TRA));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_TRB));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_Inversion_Signals));
#if defined (USE_MDR32F1QI_REV3_4)
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_Filtration_Flow));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_Automatic_Adjustment));
#elif defined (USE_MDR32F1QI_REV6) || (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_EN_FLT));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_INPINV));
    assert_param(IS_FUNCTIONAL_STATE(MIL_STD_1553_InitStruct->MIL_STD_1553_RT_HW));
#endif

    /* MIL_STD_1553x CONTROL Configuration */
    tmpreg = (MIL_STD_1553_InitStruct->MIL_STD_1553_RERR              << MIL_STD_1553_CONTROL_RERR_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_Mode              << MIL_STD_1553_CONTROL_BCMODE_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_DIV               << MIL_STD_1553_CONTROL_DIV_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_RTA               << MIL_STD_1553_CONTROL_RTA_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_TRA               << MIL_STD_1553_CONTROL_TRA_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_TRB               << MIL_STD_1553_CONTROL_TRB_Pos)
           | (MIL_STD_1553_InitStruct->MIL_STD_1553_Inversion_Signals << MIL_STD_1553_CONTROL_INVTR_Pos);

#if defined  (USE_MDR32F1QI_REV3_4)
    tmpreg |= (MIL_STD_1553_InitStruct->MIL_STD_1553_Automatic_Adjustment << MIL_STD_1553_CONTROL_AUTOTUNE_Pos)
            | (MIL_STD_1553_InitStruct->MIL_STD_1553_Filtration_Flow      << MIL_STD_1553_CONTROL_ENFILTER_Pos);

#elif defined (USE_MDR32F1QI_REV6) || (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
    tmpreg |= (MIL_STD_1553_InitStruct->MIL_STD_1553_EN_FLT << MIL_STD_1553_CONTROL_EN_FLT_Pos)
            | (MIL_STD_1553_InitStruct->MIL_STD_1553_INPINV << MIL_STD_1553_CONTROL_INPINV_Pos)
            | (MIL_STD_1553_InitStruct->MIL_STD_1553_RT_HW  << MIL_STD_1553_CONTROL_RT_HW_Pos);
#endif

    tmpreg |= MIL_STD_1553_CONTROL_MR;

    MIL_STD_1553x->CONTROL = tmpreg;
}

/**
  * @brief  Fills each MIL_STD_1553_InitStruct member with its default value.
  * @param  MIL_STD_1553_InitStruct: pointer to a @ref MIL_STD_1553_InitTypeDef structure
  *         that is to be initialized.
  * @retval None
  */
void MIL_STD_1553xStructInit(MIL_STD_1553_InitTypeDef * MIL_STD_1553_InitStruct)
{
    /* MIL_STD_1553_InitStruct members default value */
    /* Initialize the MIL_STD_1553_DIV member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_DIV = 0;
    /* Initialize the MIL_STD_1553_Mode member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_Mode = MIL_STD_1553_ModeTerminal;
    /* Initialize the MIL_STD_1553_RERR member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_RERR = DISABLE;
    /* Initialize the MIL_STD_1553_RTA member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_RTA = 0;
    /* Initialize the MIL_STD_1553_TRA member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_TRA = DISABLE;
    /* Initialize the MIL_STD_1553_TRB member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_TRB = DISABLE;
    /* Initialize the MIL_STD_1553_Inversion_Signals member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_Inversion_Signals = DISABLE;

#if defined  (USE_MDR32F1QI_REV3_4)
    /* Initialize the MIL_STD_1553_Filtration_Flow member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_Filtration_Flow = DISABLE;
    /* Initialize the MIL_STD_1553_Automatic_Adjustment member */
    MIL_STD_1553_InitStruct->MIL_STD_1553_Automatic_Adjustment = DISABLE;
#elif defined (USE_MDR32F1QI_REV6) || (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
    MIL_STD_1553_InitStruct->MIL_STD_1553_EN_FLT = DISABLE;
    MIL_STD_1553_InitStruct->MIL_STD_1553_INPINV = DISABLE;
    MIL_STD_1553_InitStruct->MIL_STD_1553_RT_HW = DISABLE;
#endif
}

/**
  * @brief  Enables or disables the specified MIL_STD_1553 peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  NewState - @ref FunctionalState - new state of the MIL_STD_1553x peripheral.
  * @retval None
  */
void MIL_STD_1553_Cmd(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        /* Enable the selected MIL_STD_1553 by clearing the MR bit in the CONTROL register */
        MIL_STD_1553x->CONTROL &= ~MIL_STD_1553_CONTROL_MR;
    }
    else
    {
        /* Disable the selected MIL_STD_1553 by setting the MR bit in the CONTROL register */
        MIL_STD_1553x->CONTROL |= MIL_STD_1553_CONTROL_MR;
    }
}

/**
  * @brief  Return the CommandWordX received by the MIL_STD_1553x peripher.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  COMMAND_WORDx - @ref MIL_STD_1553_Number_CommandWord - Number of received the CommandWord.
  * @retval The received command word.
  */
uint32_t MIL_STD_1553_GetCommandWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_CommandWord COMMAND_WORDx)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_COMMAND_WORD(COMMAND_WORDx));

    if(COMMAND_WORDx == MIL_STD_1553_COMMAND_WORD1)
    {
        return (MIL_STD_1553x->CommandWord1);
    }
    else
    {
        return (MIL_STD_1553x->CommandWord2);
    }
}

/**
  * @brief  Set the CommandWordX register for the specified MIL_STD_1553 peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  COMMAND_WORDx - @ref MIL_STD_1553_Number_CommandWord - Number of the CommandWord.
  * @param  CommandWord: pointer to a @ref MIL_STD_1553_CommandWordTypeDef structure
  *         that contains the command information for the specified MIL_STD_1553 command register.
  * @retval None
  */
void MIL_STD_1553_SetCommandWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_CommandWord COMMAND_WORDx, MIL_STD_1553_CommandWordTypeDef * CommandWord)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_COMMAND_WORD(COMMAND_WORDx));

    if(COMMAND_WORDx == MIL_STD_1553_COMMAND_WORD1)
    {
        MIL_STD_1553x->CommandWord1 = CommandWord->CommandWord;
    }
    else
    {
        MIL_STD_1553x->CommandWord2 = CommandWord->CommandWord;
    }
}

/**
  * @brief  Return the data word of management commands received by the MIL_STD_1553x peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval The received ModeData.
  */
uint32_t MIL_STD_1553_GetModeData(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));

    return (MIL_STD_1553x->ModeData);
}

/**
  * @brief  Returns the MSG register value of the specified MIL_STD_1553x peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval Returns code message received in the mode of the TD and M.
  */
uint32_t MIL_STD_1553_GetMSG(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));

    return (MIL_STD_1553x->MSG);
}

/**
  * @brief  Set the StatusWordX register for the specified MIL_STD_1553 peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  STATUS_WORDx - @ref MIL_STD_1553_Number_StatusWord - Number of the StatusWord.
  * @param  StatusWord: pointer to a @ref MIL_STD_1553_StatusWordTypeDef structure
  *         that contains the status information for the specified MIL_STD_1553 status register.
  * @retval None
  */
void MIL_STD_1553_SetStatusWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_StatusWord STATUS_WORDx, MIL_STD_1553_StatusWordTypeDef * StatusWord)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_STATUS_WORD(STATUS_WORDx));

    if(STATUS_WORDx == MIL_STD_1553_STATUS_WORD1)
    {
        MIL_STD_1553x->StatusWord1 = StatusWord->StatusWord;
    }
    else
    {
        MIL_STD_1553x->StatusWord2 = StatusWord->StatusWord;
    }
}

/**
  * @brief  Returns th StatusWordx register value of the specified MIL_STD_1553x peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  STATUS_WORDx - @ref MIL_STD_1553_Number_StatusWord - Number of the StatusWord.
  * @retval Return the StatusWordx register value.
  */
uint32_t MIL_STD_1553_GetStatusWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_StatusWord STATUS_WORDx)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_STATUS_WORD(STATUS_WORDx));

    if(STATUS_WORDx == MIL_STD_1553_STATUS_WORD1)
    {
        return (MIL_STD_1553x->StatusWord1);
    }
    else
    {
        return (MIL_STD_1553x->StatusWord2);
    }
}

/**
  * @brief  Return the Status register value of the specified MIL_STD_1553x peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval STATUS: the register's value.
  */
uint32_t MIL_STD_1553_GetStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));

    return (MIL_STD_1553x->STATUS);
}

/**
  * @brief  Checks whether the specified MIL_STD_1553 flag is set or not.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  MIL_STD_1553_FLAG - @ref MIL_STD_1553_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - The state of MIL_STD_1553_FLAG (SET or RESET).
  */
FlagStatus MIL_STD_1553_GetFlagStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Flags MIL_STD_1553_FLAG)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_FLAG(MIL_STD_1553_FLAG));

    if (MIL_STD_1553x->STATUS & (uint32_t)MIL_STD_1553_FLAG)
    {
        bitstatus = SET;
    }
    else {
        bitstatus = RESET;
    }

    return (bitstatus);
}

/**
  * @brief  Enables or disables the specified MIL_STD_1553x interrupts.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  MIL_STD_1553_IT: specifies the MIL_STD_1553x interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of the @ref MIL_STD_1553_IT_Def values.
  * @param  NewState - @ref FunctionalState - new state of the specified MIL_STD_1553x interrupts.
  * @retval None
  */
void MIL_STD_1553_ITConfig(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t MIL_STD_1553_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_ITS(MIL_STD_1553_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MIL_STD_1553x->INTEN |= MIL_STD_1553_IT;
    }
    else
    {
        MIL_STD_1553x->INTEN &= (~MIL_STD_1553_IT);
    }
}

/**
  * @brief  Reads the received data from buffer.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  Subaddress: the number of the subaddress that is used for reception.
  * @param  NumberDataWords: the number of the data word to read.
  * @param  ptr_Dest: pointer to array to place received data to.
  * @retval None
  */
void MIL_STD_1553_ReceiveData(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t Subaddress, uint32_t NumberDataWords, uint32_t * ptr_Dest)
{
    uint32_t i;
    uint32_t *ptr_data;

    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_SUBADDRES(Subaddress));
    assert_param(IS_MIL_STD_1553_NUMBER_DATA_WORDS(NumberDataWords));

    ptr_data = (uint32_t *)((uint32_t)MIL_STD_1553x + sizeof(uint32_t)*Subaddress*32);

    if(NumberDataWords == 0)
    {
        NumberDataWords = 32;
    }

    for(i = 0; i < NumberDataWords; i++)
    {
        ptr_Dest[i] = (uint16_t)ptr_data[i];
    }
}

/**
  * @brief  Writes data to the transmit buffer.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  Subaddress: the number of the subaddress that is used for transmiting.
  * @param  NumberDataWords: the number of the data word to write.
  * @param  ptr_Src: pointer to the array from which the data is read.
  * @retval None
  */
void MIL_STD_1553_WriteDataToSendBuffer(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t Subaddress, uint32_t NumberDataWords, uint32_t * ptr_Src)
{
    uint32_t i;
    uint32_t * ptr_data;

    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_SUBADDRES(Subaddress));
    assert_param(IS_MIL_STD_1553_NUMBER_DATA_WORDS(NumberDataWords));

    ptr_data = (uint32_t*)((uint32_t)MIL_STD_1553x + sizeof(uint32_t)*Subaddress*32);

    if(NumberDataWords == 0)
    {
        NumberDataWords = 32;
    }

    for(i = 0; i < NumberDataWords; i++)
    {
        ptr_data[i] = (uint32_t)ptr_Src[i];
    }
}

/**
  * @brief  Initiates transmission of a message in the channel in the BC mode.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval None
  */
void MIL_STD_1553_StartTransmission(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));

    MIL_STD_1553x->CONTROL |= MIL_STD_1553_CONTROL_BCSTART;
}

/**
  * @brief  Enables or disables the specified MIL_STD_1553x transmitter.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param   TRANSMITTERx - @ref MIL_STD_1553_Number_Transmitter - specifies the MIL_STD_1553 number transmitter.
  * @param  NewState - @ref FunctionalState - new state of the specified MIL_STD_1553x transmitter.
  * @retval None
  */
void MIL_STD_1553_Transmitter_CMD(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_Transmitter TRANSMITTERx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_MIL_STD_1553_TRANSMITTER_CHANNEL(TRANSMITTERx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MIL_STD_1553x->CONTROL |= (uint32_t)TRANSMITTERx;
    }
    else
    {
        MIL_STD_1553x->CONTROL &= ~(uint32_t)TRANSMITTERx;
    }
}

/**
  * @brief  Return the Error register value of the specified MIL_STD_1553x peripheral.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @retval ERROR: the register's value.
  */
uint32_t MIL_STD_1553_GetErrorStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));

    return (MIL_STD_1553x->ERROR);
}

/**
  * @brief  Enable or disable RERR bit in the CONTROL register of the
  *         specified MIL_STD_1553x periphera.
  * @param  MIL_STD_1553x: Select the MIL_STD_1553 peripheral.
  *         This parameter can be one of the following values:
  *             MDR_MIL_STD_15531, MDR_MIL_STD_15532.
  * @param  NewState - @ref FunctionalState - new state of the specified MIL_STD_1553x interrupts.
  * @retval None
  */
void MIL_STD_1553_RERRCmd(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_MIL_STD_1553_ALL_PERIPH(MIL_STD_1553x));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MIL_STD_1553x->CONTROL |= MIL_STD_1553_CONTROL_RERR;
    }
    else
    {
        MIL_STD_1553x->CONTROL &= ~MIL_STD_1553_CONTROL_RERR;
    }
}

/** @} */ /* End of group MIL_STD_1553_Exported_Functions */

#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group MIL_STD_1553 */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_mil_std_1553.c */



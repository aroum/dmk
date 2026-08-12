/**
  ******************************************************************************
  * @file    MDR32FxQI_dma.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/07/2024
  * @brief   This file contains all the DMA firmware functions.
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
#include "MDR32FxQI_dma.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup DMA DMA
  * @{
  */

/** @addtogroup DMA_Private_Variables DMA Private Variables
  * @{
  */

/**
  * @brief DMA Channel Control Data Table
  */

#if defined (__ICCARM__) /* IAR Compiler */
    #pragma data_alignment = 1024
    #if defined (USE_K1986VE1xI)
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number] IAR_SECTION ("EXECUTABLE_MEMORY_SECTION");
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number] IAR_SECTION ("EXECUTABLE_MEMORY_SECTION");
        #endif
    #else
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number];
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number];
        #endif		
    #endif /* #if defined (USE_K1986VE1xI) */

#elif defined (__CMCARM__) /* Phyton CMC-ARM Compiler */
    #if defined (USE_K1986VE1xI)
    #pragma locate DMA_ControlTable 0x20100000 noinit
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number];
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number];
        #endif
        #else
        #pragma locate DMA_ControlTable 0x20000000 noinit
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number];
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number];
        #endif
    #endif /* #if defined (USE_K1986VE1xI) */

#elif defined (__ARMCC_VERSION) /* ARM Compiler */
    #if defined (USE_K1986VE1xI)
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (1024)));
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (1024)));
        #endif
    #else
        #if (DMA_AlternateData == 0)
            DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number] __attribute__ ((aligned (1024)));
        #endif
        #if (DMA_AlternateData == 1)
            DMA_CtrlDataTypeDef DMA_ControlTable[(32 * DMA_AlternateData) + DMA_Channels_Number] __attribute__ ((aligned (1024)));
        #endif
    #endif /* #if defined (USE_K1986VE1xI) */

#endif

/** @} */ /* End of group DMA_Private_Variables */


/** @addtogroup DMA_Exported_Functions DMA Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the DMA registers to their default reset values
  *         and clears pending DMA NVIC interrupt request.
  * @param  None
  * @retval None
  */
void DMA_DeInit(void)
{
    MDR_DMA->CFG               = 0x00;       /* Master Enable Off */
    MDR_DMA->CTRL_BASE_PTR     = 0x00;       /* Control data base pointer */
    MDR_DMA->CHNL_SW_REQUEST   = 0x00;       /* Disable all sw requests */
    MDR_DMA->CHNL_USEBURST_SET = 0xFFFFFFFF; /* Disable burst mode */
    MDR_DMA->CHNL_REQ_MASK_SET = 0xFFFFFFFF; /* Disable all channels */

    MDR_DMA->CHNL_ENABLE_CLR   = 0xFFFFFFFF; /* Clear channel enable */
    MDR_DMA->CHNL_PRI_ALT_CLR  = 0xFFFFFFFF; /* Reset to primary data structure */
    MDR_DMA->CHNL_PRIORITY_CLR = 0xFFFFFFFF; /* Reset to default priority */
    MDR_DMA->ERR_CLR           = 0x01;       /* Clear dma_err status */

    MDR_DMA->CFG               = 0x01;       /* Master Enable On */
    MDR_DMA->CHNL_REQ_MASK_SET = 0xFFFFFFFF;
    MDR_DMA->CHNL_ENABLE_SET   = 0xFFFFFFFF; /* Enablee all channels */
    NVIC_ClearPendingIRQ(DMA_IRQn);
}

/**
  * @brief  Initializes the DMA control data structure
  *         according to the specified parameters.
  * @param  DMA_ctrl_data_ptr: pointer to a @ref DMA_CtrlDataInitTypeDef structure
  *         that contains the control data structure to initialize.
  * @param  DMA_ctrl_table_ptr: pointer to a @ref DMA_CtrlDataTypeDef structure that
  *         contains the initial control data configuration, provided by the application.
  * @retval None
  */
void DMA_CtrlDataInit(DMA_CtrlDataInitTypeDef *DMA_ctrl_data_ptr, DMA_CtrlDataTypeDef *DMA_ctrl_table_ptr)
{
    /* Check DMA configuration parameters */
    assert_param(IS_DMA_CHANNELS(DMA_Channels_Number));
    assert_param(IS_DMA_ALTERNATE_DATA(DMA_AlternateData));

    /* Check the parameters */
    assert_param(IS_DMA_SOURCE_INC_STATE(DMA_ctrl_data_ptr->DMA_SourceIncSize));
    assert_param(IS_DMA_DEST_INC_STATE(DMA_ctrl_data_ptr->DMA_DestIncSize));
    assert_param(IS_DMA_MEMORY_DATA_SIZE(DMA_ctrl_data_ptr->DMA_MemoryDataSize));
    assert_param(IS_DMA_MODE(DMA_ctrl_data_ptr->DMA_Mode));
    assert_param(IS_DMA_CYCLE_SIZE(DMA_ctrl_data_ptr->DMA_CycleSize));
    assert_param(IS_DMA_CONTINUOUS_NUMBER(DMA_ctrl_data_ptr->DMA_NumContinuous));
    assert_param(IS_DMA_SOURCE_PROT(DMA_ctrl_data_ptr->DMA_SourceProtCtrl));
    assert_param(IS_DMA_DEST_PROT(DMA_ctrl_data_ptr->DMA_DestProtCtrl));

    /* DMA Source Data End Address */
    if (DMA_ctrl_data_ptr->DMA_SourceIncSize == DMA_SourceIncNo)
    {
        DMA_ctrl_table_ptr->DMA_SourceEndAddr = DMA_ctrl_data_ptr->DMA_SourceBaseAddr;
    }
    else
    {
        DMA_ctrl_table_ptr->DMA_SourceEndAddr = (DMA_ctrl_data_ptr->DMA_SourceBaseAddr + \
            ((DMA_ctrl_data_ptr->DMA_CycleSize - 1) << ((uint32_t)(DMA_ctrl_data_ptr->DMA_SourceIncSize))));
    }

    /* DMA Destination Data End Address */
    if (DMA_ctrl_data_ptr->DMA_Mode == DMA_Mode_MemScatterPri)
    {
        /* Memory Scatter-Gather mode */
        DMA_ctrl_table_ptr->DMA_DestEndAddr = (DMA_ctrl_data_ptr->DMA_DestBaseAddr + 12);
    }
    else
    {
        /* Other modes */
        if (DMA_ctrl_data_ptr->DMA_DestIncSize == DMA_DestIncNo)
        {
            DMA_ctrl_table_ptr->DMA_DestEndAddr = DMA_ctrl_data_ptr->DMA_DestBaseAddr;
        }
        else
        {
            DMA_ctrl_table_ptr->DMA_DestEndAddr = ( DMA_ctrl_data_ptr->DMA_DestBaseAddr + \
              ((DMA_ctrl_data_ptr->DMA_CycleSize - 1) << ((uint32_t)(DMA_ctrl_data_ptr->DMA_DestIncSize))));
        }
    }

    /* DMA Control Data */
    DMA_ctrl_table_ptr->DMA_Control = (((uint32_t)(DMA_ctrl_data_ptr->DMA_DestIncSize) << 30)   |
                                       DMA_ctrl_data_ptr->DMA_MemoryDataSize                    |
                                       ((uint32_t)(DMA_ctrl_data_ptr->DMA_SourceIncSize) << 26) |
                                       DMA_ctrl_data_ptr->DMA_DestProtCtrl                      |
                                       DMA_ctrl_data_ptr->DMA_SourceProtCtrl                    |
                                       DMA_ctrl_data_ptr->DMA_NumContinuous                     |
                                       ((DMA_ctrl_data_ptr->DMA_CycleSize - 1) << 4)            |
                                       DMA_ctrl_data_ptr->DMA_Mode);
}

/**
  * @brief  Initializes the DMA Channel DMA_ControlTable structure
  *         according to the specified parameters.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  DMA_CtrlDataType - @ref DMA_Data_Struct_Selection - data structure selection
  * @param  DMA_CtrlStruct: pointer to a @ref DMA_CtrlDataInitTypeDef structure that contains the initial
  *         control data configuration, provided by the application for the specified DMA Channel.
  * @retval None
  */
void DMA_CtrlInit(uint8_t DMA_Channel, DMA_Data_Struct_Selection DMA_CtrlDataType, DMA_CtrlDataInitTypeDef* DMA_CtrlStruct)
{
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_SELECT_DATA_STRUCTURE(DMA_CtrlDataType));
    
    /* Primary Control Data Init */
    if (DMA_CtrlDataType == DMA_CTRL_DATA_PRIMARY)
    {
        DMA_CtrlDataInit(DMA_CtrlStruct, &DMA_ControlTable[DMA_Channel]);
    }
#if (DMA_AlternateData == 1)
    /* Alternate Control Data Init */
    else
    {
        uint32_t ptr = (MDR_DMA->ALT_CTRL_BASE_PTR + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
        DMA_CtrlDataInit(DMA_CtrlStruct, (DMA_CtrlDataTypeDef *)ptr);
    }
#endif
}

/**
  * @brief  Initializes the DMA Scatter-Gather Task structure
  *         according to the specified parameters.
  * @param  DMA_Task: specifies the current task number. Begins with 0.
  * @param  DMA_CtrlStruct: pointer to a @ref DMA_CtrlDataInitTypeDef structure
  *         that contains the control data information for the specified task.
  * @param  DMA_SG_TaskArray: pointer to a @ref DMA_CtrlDataTypeDef array that
  *         contains the copy of the alternate control data information for the specified task.
  * @retval None
  */
void DMA_SG_CtrlInit(uint32_t DMA_Task, DMA_CtrlDataTypeDef *DMA_SG_TaskArray, DMA_CtrlDataInitTypeDef* DMA_CtrlStruct)
{
    DMA_CtrlDataInit(DMA_CtrlStruct, &DMA_SG_TaskArray[DMA_Task]);
}

/**
  * @brief  Initializes the DMA Channel in Memory Scatter-Gather mode
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  DMA_SG_InitStruct: pointer to a @ref DMA_Channel_SG_InitTypeDef array that
  *         contains the configuration information for the specified DMA Channel.
  * @retval None
  */
void DMA_SG_Init(uint8_t DMA_Channel, DMA_Channel_SG_InitTypeDef *DMA_SG_InitStruct)
{
    DMA_CtrlDataInitTypeDef DMA_PriCtrlData;

    /* Check the parameters */
    assert_param(DMA_AlternateData == 1);
    assert_param(DMA_SG_InitStruct != 0);
    assert_param(DMA_SG_InitStruct->DMA_SG_TaskNumber != 0);
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_AHB_PROT(DMA_SG_InitStruct->DMA_ProtCtrl));
    assert_param(IS_DMA_PRIORITY(DMA_SG_InitStruct->DMA_Priority));
    assert_param(IS_DMA_SOURCE_PROT(DMA_SG_InitStruct->DMA_SourceProtCtrl));
    assert_param(IS_DMA_DEST_PROT(DMA_SG_InitStruct->DMA_DestProtCtrl));
    assert_param(IS_DMA_BURST(DMA_SG_InitStruct->DMA_UseBurst));

    /* Check the CTRL_BASE_PTR initialization */
    MDR_DMA->CTRL_BASE_PTR = (uint32_t)DMA_ControlTable;

    /* Primary Control Data Init */
    DMA_PriCtrlData.DMA_SourceBaseAddr = (uint32_t)(DMA_SG_InitStruct->DMA_SG_TaskArray);
    DMA_PriCtrlData.DMA_DestBaseAddr   = (MDR_DMA->ALT_CTRL_BASE_PTR + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
    DMA_PriCtrlData.DMA_SourceIncSize  = DMA_SourceIncWord;
    DMA_PriCtrlData.DMA_DestIncSize    = DMA_DestIncWord;
    DMA_PriCtrlData.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_PriCtrlData.DMA_Mode           = DMA_Mode_MemScatterPri;
    DMA_PriCtrlData.DMA_CycleSize      = DMA_SG_InitStruct->DMA_SG_TaskNumber << 2;
    DMA_PriCtrlData.DMA_NumContinuous  = DMA_Transfers_4;
    DMA_PriCtrlData.DMA_SourceProtCtrl = DMA_SG_InitStruct->DMA_SourceProtCtrl;
    DMA_PriCtrlData.DMA_DestProtCtrl   = DMA_SG_InitStruct->DMA_DestProtCtrl;

    DMA_CtrlDataInit(&DMA_PriCtrlData, &DMA_ControlTable[DMA_Channel]);

    /* DMA configuration register */
    MDR_DMA->CFG = DMA_CFG_MASTER_ENABLE | DMA_SG_InitStruct->DMA_ProtCtrl;

    /* Burst mode */
    if (DMA_SG_InitStruct->DMA_UseBurst == DMA_BurstSet)
    {
        MDR_DMA->CHNL_USEBURST_SET = (1 << DMA_Channel);
    }
    else
    {
        MDR_DMA->CHNL_USEBURST_CLR = (1 << DMA_Channel);
    }

    /* Channel mask clear */
    MDR_DMA->CHNL_REQ_MASK_CLR = (1 << DMA_Channel);

    /* Enable channel */
    MDR_DMA->CHNL_ENABLE_SET = (1 << DMA_Channel);

    /* Primary - Alternate control data structure selection */
    MDR_DMA->CHNL_PRI_ALT_CLR = (1 << DMA_Channel); /* Use Primary */

    /* Channel priority set */
    if (DMA_SG_InitStruct->DMA_Priority == DMA_Priority_High)
    {
        MDR_DMA->CHNL_PRIORITY_SET = (1 << DMA_Channel); /* High priority */
    }
    else
    {
        MDR_DMA->CHNL_PRIORITY_CLR = (1 << DMA_Channel); /* Default priority */
    }
}

/**
  * @brief  Initializes the DMA Channel according to
  *         the specified parameters in the DMA_InitStruct.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  DMA_InitStruct: pointer to a @ref DMA_ChannelInitTypeDef structure that
  *         contains the configuration information for the specified DMA Channel.
  * @retval None
  */
void DMA_Init(uint8_t DMA_Channel, DMA_ChannelInitTypeDef* DMA_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_AHB_PROT(DMA_InitStruct->DMA_ProtCtrl));
    assert_param(IS_DMA_PRIORITY(DMA_InitStruct->DMA_Priority));
    assert_param(IS_DMA_BURST(DMA_InitStruct->DMA_UseBurst));
    assert_param(IS_DMA_SELECT_DATA_STRUCTURE(DMA_InitStruct->DMA_SelectDataStructure));

    /* Check the CTRL_BASE_PTR initialisation */
    MDR_DMA->CTRL_BASE_PTR = (uint32_t)DMA_ControlTable;

    /* Primary Control Data Init */
    if (DMA_InitStruct->DMA_PriCtrlData)
    {
        DMA_CtrlDataInit(DMA_InitStruct->DMA_PriCtrlData, &DMA_ControlTable[DMA_Channel]);
    }

#if (DMA_AlternateData == 1)
    /* Alternate Control Data Init */
    if (DMA_InitStruct->DMA_AltCtrlData)
    {
        uint32_t ptr = (MDR_DMA->ALT_CTRL_BASE_PTR + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
        DMA_CtrlDataInit(DMA_InitStruct->DMA_AltCtrlData, (DMA_CtrlDataTypeDef *)ptr);
    }
#endif

    /* DMA configuration register */
    MDR_DMA->CFG = DMA_CFG_MASTER_ENABLE | DMA_InitStruct->DMA_ProtCtrl;

    /* Burst mode */
    if (DMA_InitStruct->DMA_UseBurst == DMA_BurstSet)
    {
        MDR_DMA->CHNL_USEBURST_SET = (1 << DMA_Channel);
    }
    else
    {
        MDR_DMA->CHNL_USEBURST_CLR = (1 << DMA_Channel);
    }

    /* Channel mask clear */
    MDR_DMA->CHNL_REQ_MASK_CLR = (1 << DMA_Channel);

    /* Enable channel */
    MDR_DMA->CHNL_ENABLE_SET = (1 << DMA_Channel);

    /* Primary - Alternate control data structure selection */
    if (DMA_InitStruct->DMA_SelectDataStructure == DMA_CTRL_DATA_PRIMARY)
    {
        MDR_DMA->CHNL_PRI_ALT_CLR = (1 << DMA_Channel); /* Use Primary */
    }
    else
    {
        MDR_DMA->CHNL_PRI_ALT_SET = (1 << DMA_Channel); /* Use Alternate */
    }

    /* Channel priority set */
    if (DMA_InitStruct->DMA_Priority == DMA_Priority_High)
    {
        MDR_DMA->CHNL_PRIORITY_SET = (1 << DMA_Channel); /* High priority */
    }
    else
    {
        MDR_DMA->CHNL_PRIORITY_CLR = (1 << DMA_Channel); /* Default priority */
    }
}

/**
  * @brief  Fills each DMA_InitStruct member with its default value.
  * @param  DMA_InitStruct: pointer to a @ref DMA_ChannelInitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void DMA_StructInit(DMA_ChannelInitTypeDef* DMA_InitStruct)
{
    DMA_InitStruct->DMA_PriCtrlData         = 0;
    DMA_InitStruct->DMA_AltCtrlData         = 0;
    DMA_InitStruct->DMA_ProtCtrl            = 0;
    DMA_InitStruct->DMA_Priority            = DMA_Priority_Default;
    DMA_InitStruct->DMA_UseBurst            = DMA_BurstClear;
    DMA_InitStruct->DMA_SelectDataStructure = DMA_CTRL_DATA_PRIMARY;
}

/**
  * @brief  Fills each DMA_SG_InitStruct member with its default value.
  * @param  DMA_SG_InitStruct: pointer to a @ref DMA_Channel_SG_InitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void DMA_SG_StructInit(DMA_Channel_SG_InitTypeDef* DMA_SG_InitStruct)
{
    DMA_SG_InitStruct->DMA_SG_TaskArray   = 0;
    DMA_SG_InitStruct->DMA_SG_TaskNumber  = 0;
    DMA_SG_InitStruct->DMA_SourceProtCtrl = 0;
    DMA_SG_InitStruct->DMA_DestProtCtrl   = 0;
    DMA_SG_InitStruct->DMA_ProtCtrl       = 0;
    DMA_SG_InitStruct->DMA_Priority       = DMA_Priority_Default;
    DMA_SG_InitStruct->DMA_UseBurst       = DMA_BurstClear;
}

/**
  * @brief  Enables or disables the specified DMA Channel.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  NewState - @ref FunctionalState - new state of the DMA Channel.
  * @retval None
  */
void DMA_Cmd(uint8_t DMA_Channel, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    /* Channel Enable/Disable */
    if (NewState != DISABLE)
    {
        MDR_DMA->CHNL_ENABLE_SET = (1 << DMA_Channel);
    }
    else
    {
        MDR_DMA->CHNL_ENABLE_CLR = (1 << DMA_Channel);
    }
}

/**
  * @brief  Generates the specified DMA Channel software request.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @retval None
  */
void DMA_Request(uint8_t DMA_Channel)
{
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));

    /* Set SW Request */
    MDR_DMA->CHNL_SW_REQUEST = (1 << DMA_Channel);
}

/**
  * @brief  Clears the DMA Error bit.
  * @param  None
  * @retval None
  */
void DMA_ClearError(void)
{
    MDR_DMA->ERR_CLR = 0x01; /* Clear dma_err status */
}

/**
  * @brief  Reload DMA channel CycleSize and ChannelMode.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  SelectDataStruct - @ref DMA_Data_Struct_Selection - data structure to change.
  * @param  CycleSize: new cycle size.
  * @param  ChannelMode - @ref DMA_Operating_Mode - new channel mode.
  * @retval None.
  */
void DMA_ChannelReloadCycle(uint8_t DMA_Channel, DMA_Data_Struct_Selection SelectDataStruct,
                            uint32_t CycleSize, DMA_Operating_Mode ChannelMode)
{
    uint32_t dma_table_base, tmp;
    DMA_CtrlDataTypeDef *ch_data;
    
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_SELECT_DATA_STRUCTURE(SelectDataStruct));
    assert_param(IS_DMA_CYCLE_SIZE(CycleSize));
    assert_param(IS_DMA_MODE(ChannelMode));
    
    if (SelectDataStruct == DMA_CTRL_DATA_PRIMARY)
    {
        dma_table_base = MDR_DMA->CTRL_BASE_PTR;
    }
#if (DMA_AlternateData == 1)
    else
    {
        dma_table_base = MDR_DMA->ALT_CTRL_BASE_PTR;
    }
#endif
    
    ch_data = (DMA_CtrlDataTypeDef *)(dma_table_base + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
    tmp = (ch_data->DMA_Control & ~(0x3FF << 4))
        | ((CycleSize - 1) << 4) | ((uint32_t)ChannelMode);
    ch_data->DMA_Control = tmp;
}

/**
  * @brief  Returns the number of remaining transfers
  *         in the current DMA Channel cycle.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  DMA_CtrlData - @ref DMA_Data_Struct_Selection - specifies
  *         the primary / alternate control data table to check.
  * @retval The number of remaining transfers in the current
  *         DMA Channel cycle and the current control data structure.
  */
uint32_t DMA_GetCurrTransferCounter(uint8_t DMA_Channel, DMA_Data_Struct_Selection DMA_CtrlData)
{
    DMA_CtrlDataTypeDef *ptr; /* Pointer to the channels's data structure */

    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_SELECT_DATA_STRUCTURE(DMA_CtrlData));

    /* Init the control data pointer */
    if (DMA_CtrlData == DMA_CTRL_DATA_PRIMARY)
    {
        ptr = (DMA_CtrlDataTypeDef *)(MDR_DMA->CTRL_BASE_PTR + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
    }
    else
    {
        ptr = (DMA_CtrlDataTypeDef *)(MDR_DMA->ALT_CTRL_BASE_PTR + (DMA_Channel * sizeof(DMA_CtrlDataTypeDef)));
    }

    /* Read the number of remaining transfers */
    return (((ptr->DMA_Control & DMA_CONTROL_MINUS_1) >> 4) + 1);
}

/**
  * @brief  Checks whether the specified DMA Channel flag is set or not.
  * @param  DMA_Channel: can be 0 to 31 or a value of @ref DMA_Valid_Channels to select the DMA Channel.
  * @param  DMA_Flag - @ref DMA_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - Current state of DMA_FLAG (SET or RESET).
  */
FlagStatus DMA_GetFlagStatus(uint8_t DMA_Channel, DMA_Flags DMA_Flag)
{
    /* Check the parameters */
    assert_param(IS_DMA_CHANNEL(DMA_Channel));
    assert_param(IS_DMA_FLAG(DMA_Flag));

    /* Read Flag Status */
    switch(DMA_Flag)
    {
        case DMA_FLAG_DMA_ENA:
            return (FlagStatus)((MDR_DMA->STATUS & DMA_STATUS_MASTER_ENABLE) != 0);
        case DMA_FLAG_DMA_ERR:
            return (FlagStatus)((MDR_DMA->ERR_CLR & 0x01) == 0);
        case DMA_FLAG_CHNL_ENA:
            return (FlagStatus)((MDR_DMA->CHNL_ENABLE_SET & (1 << DMA_Channel)) != 0);
        case DMA_FLAG_CHNL_MASK:
            return (FlagStatus)((MDR_DMA->CHNL_REQ_MASK_SET & (1 << DMA_Channel)) != 0);
        case DMA_FLAG_CHNL_WAIT:
            return (FlagStatus)((MDR_DMA->WAITONREQ_STATUS & (1 << DMA_Channel)) != 0);
        case DMA_FLAG_CHNL_BURST:
            return (FlagStatus)((MDR_DMA->CHNL_USEBURST_SET & (1 << DMA_Channel)) != 0);
        case DMA_FLAG_CHNL_ALT:
            return (FlagStatus)((MDR_DMA->CHNL_PRI_ALT_SET & (1 << DMA_Channel)) != 0);
        case DMA_FLAG_CHNL_PRIORITY:
            return (FlagStatus)((MDR_DMA->CHNL_PRIORITY_SET & (1 << DMA_Channel)) != 0);
        default:
            return (FlagStatus)0;
    }
}

/** @} */ /* End of group DMA_Exported_Functions */

/** @} */ /* End of group DMA */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_dma.c */



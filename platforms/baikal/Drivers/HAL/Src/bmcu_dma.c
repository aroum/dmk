/**
 * *****************************************************************************
 *  @file       bmcu_dma.c
 *  @author     Baikal electronics SDK team
 *  @brief      Direct Memory Access (DMA) module driver
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

#include "bmcu_dma.h"
#include "bmcu_def.h"

#define IS_DMA_CHANNEL(__VALUE__)               (((__VALUE__) == DMA_CH0) || \
                                                 ((__VALUE__) == DMA_CH1) || \
                                                 ((__VALUE__) == DMA_CH2) || \
                                                 ((__VALUE__) == DMA_CH3) || \
                                                 ((__VALUE__) == DMA_CH4) || \
                                                 ((__VALUE__) == DMA_CH5) || \
                                                 ((__VALUE__) == DMA_CH6) || \
                                                 ((__VALUE__) == DMA_CH7))

#define IS_DMA_PRIORITY(__VALUE__)              (((__VALUE__) == DMA_PRIORITY_0) || \
                                                 ((__VALUE__) == DMA_PRIORITY_1) || \
                                                 ((__VALUE__) == DMA_PRIORITY_2) || \
                                                 ((__VALUE__) == DMA_PRIORITY_3) || \
                                                 ((__VALUE__) == DMA_PRIORITY_4) || \
                                                 ((__VALUE__) == DMA_PRIORITY_5) || \
                                                 ((__VALUE__) == DMA_PRIORITY_6) || \
                                                 ((__VALUE__) == DMA_PRIORITY_7))

#define IS_DMA_DIRECTION(__VALUE__)             (((__VALUE__) == DMA_DIR_MEMORY_TO_MEMORY) || \
                                                 ((__VALUE__) == DMA_DIR_MEMORY_TO_PERIPH) || \
                                                 ((__VALUE__) == DMA_DIR_PERIPH_TO_MEMORY) || \
                                                 ((__VALUE__) == DMA_DIR_PERIPH_TO_PERIPH))

#define IS_DMA_MODE(__VALUE__)                  (((__VALUE__) == DMA_MODE_NORMAL) || \
                                                 ((__VALUE__) == DMA_MODE_AUTO_RELOAD))

#define IS_DMA_INC_MODE(__VALUE__)              (((__VALUE__) == DMA_INC_MODE_INCREMENT) || \
                                                 ((__VALUE__) == DMA_INC_MODE_DECREMENT) || \
                                                 ((__VALUE__) == DMA_INC_MODE_NO_CHANGE))

#define IS_DMA_TR_WIDTH(__VALUE__)              (((__VALUE__) == DMA_TR_WIDTH_8BITS) || \
                                                 ((__VALUE__) == DMA_TR_WIDTH_16BITS) || \
                                                 ((__VALUE__) == DMA_TR_WIDTH_32BITS))

#define IS_DMA_BURST_LENGTH(__VALUE__)          (((__VALUE__) == DMA_BURST_LENGTH_1) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_4) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_8) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_16) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_32) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_64) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_128) || \
                                                 ((__VALUE__) == DMA_BURST_LENGTH_256))

#define IS_DMA_FIFO_MODE(__VALUE__)             (((__VALUE__) == DMA_FIFO_MODE_0) || \
                                                 ((__VALUE__) == DMA_FIFO_MODE_1))

#define IS_DMA_HANDSHAKE_MODE(__VALUE__)        (((__VALUE__) == DMA_HS_HARDWARE) || \
                                                 ((__VALUE__) == DMA_HS_SOFTWARE))

#define IS_DMA_HANDSHAKE_POLARITY(__VALUE__)    (((__VALUE__) == DMA_HS_POLARITY_HIGH) || \
                                                 ((__VALUE__) == DMA_HS_POLARITY_LOW))

#define IS_DMA_HANDSHAKE_INTERFACE(__VALUE__)   ((__VALUE__) <= 0xFU)

/**
 * @brief Deinitializes a DMA channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The DMA channel has been deinitialized.
 *         - ERROR: The DMA channel has not been deinitialized.
 */
ErrorStatus DMA_DeInit(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_DMA_ALL_INSTANCE(DMAx));
    assert(IS_DMA_CHANNEL(Channel));

    if (READ_BIT(DMAx->MSC.CFG, DMA_MSC_CFG_EN) == DMA_MSC_CFG_EN)
    {
        /* DMA controller is enabled */

        /* Disable the selected DMA channel */
        WRITE_REG(DMAx->MSC.CH_EN, (DMA_MSC_CH_EN_WE_CH0 << Channel));

        if (READ_BIT(DMAx->MSC.CH_EN, (DMA_MSC_CH_EN_WE_CH0 << Channel)) == 0x0UL)
        {
            /* The selected channel is disabled */

            /* Reset control registers */
            WRITE_REG(DMAx->CH[Channel].CTL0,
                    (DMA_CH_CTL0_TT_FC_3 |
                    DMA_CH_CTL0_SRC_MSIZE_1 |
                    DMA_CH_CTL0_DST_MSIZE_1 |
                    DMA_CH_CTL0_INT_EN_ENABLE));
            WRITE_REG(DMAx->CH[Channel].CTL1, (0x2UL << DMA_CH_CTL1_BLOCK_TS_Pos));
            WRITE_REG(DMAx->CH[Channel].CFG0,
                      (DMA_CH_CFG0_HS_SEL_SRC_SW | DMA_CH_CFG0_HS_SEL_DST_SW));
            WRITE_REG(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_PROTCTL_1);

            /* Reset source address register */
            WRITE_REG(DMAx->CH[Channel].SAR, 0x0UL);

            /* Reset destination address register */
            WRITE_REG(DMAx->CH[Channel].DAR, 0x0UL);

            /* Reset linked list pointer register */
            WRITE_REG(DMAx->CH[Channel].LLP, 0x0UL);

            /* Mask all interrupts */
            WRITE_REG(DMAx->INT.MASK_TFR, (DMA_INT_MASK_TFR_WE_CH0 << Channel));
            WRITE_REG(DMAx->INT.MASK_BLOCK, (DMA_INT_MASK_BLOCK_WE_CH0 << Channel));
            WRITE_REG(DMAx->INT.MASK_SRC_TRAN, (DMA_INT_MASK_SRC_TRAN_WE_CH0 << Channel));
            WRITE_REG(DMAx->INT.MASK_DST_TRAN, (DMA_INT_MASK_DST_TRAN_WE_CH0 << Channel));
            WRITE_REG(DMAx->INT.MASK_ERR, (DMA_INT_MASK_ERR_WE_CH0 << Channel));

            /* Clear all interrupts */
            WRITE_REG(DMAx->INT.CLR_TFR, (DMA_INT_CLR_TFR_CH0 << Channel));
            WRITE_REG(DMAx->INT.CLR_BLOCK, (DMA_INT_CLR_BLOCK_CH0 << Channel));
            WRITE_REG(DMAx->INT.CLR_SRC_TRAN, (DMA_INT_CLR_SRC_TRAN_CH0 << Channel));
            WRITE_REG(DMAx->INT.CLR_DST_TRAN, (DMA_INT_CLR_DST_TRAN_CH0 << Channel));
            WRITE_REG(DMAx->INT.CLR_ERR, (DMA_INT_CLR_ERR_CH0 << Channel));

            status = SUCCESS;
        }
    }

    return status;
}

/**
 * @brief Initializes a DMA channel.
 * @note DMA_InitStruct structure should be initialized prior to calling this function.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param DMA_InitStruct The pointer to DMA_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The DMA channel has been initialized.
 *         - ERROR: The DMA channel has not been initialized.
 */
ErrorStatus DMA_Init(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_InitStruct_TypeDef *DMA_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_DMA_ALL_INSTANCE(DMAx));
    assert(IS_DMA_CHANNEL(Channel));
    assert(IS_DMA_PRIORITY(DMA_InitStruct->Priority));
    assert(IS_DMA_DIRECTION(DMA_InitStruct->Direction));
    assert(IS_DMA_FIFO_MODE(DMA_InitStruct->FIFOMode));
    assert(IS_DMA_MODE(DMA_InitStruct->SrcMode));
    assert(IS_DMA_MODE(DMA_InitStruct->DstMode));
    assert(IS_DMA_INC_MODE(DMA_InitStruct->SrcIncMode));
    assert(IS_DMA_INC_MODE(DMA_InitStruct->DstIncMode));
    assert(IS_DMA_TR_WIDTH(DMA_InitStruct->SrcTransferWidth));
    assert(IS_DMA_TR_WIDTH(DMA_InitStruct->DstTransferWidth));
    assert(IS_DMA_BURST_LENGTH(DMA_InitStruct->SrcBurstLength));
    assert(IS_DMA_BURST_LENGTH(DMA_InitStruct->DstBurstLength));
    assert(IS_DMA_HANDSHAKE_MODE(DMA_InitStruct->SrcHandshake));
    assert(IS_DMA_HANDSHAKE_MODE(DMA_InitStruct->DstHandshake));
    assert(IS_DMA_HANDSHAKE_POLARITY(DMA_InitStruct->SrcHWHandshakePolarity));
    assert(IS_DMA_HANDSHAKE_POLARITY(DMA_InitStruct->DstHWHandshakePolarity));
    assert(IS_FUNCTIONAL_STATE(DMA_InitStruct->SrcLinkedListCtrl));
    assert(IS_FUNCTIONAL_STATE(DMA_InitStruct->DstLinkedListCtrl));

    if (READ_BIT(DMAx->MSC.CH_EN, (DMA_MSC_CH_EN_WE_CH0 << Channel)) == 0x0UL)
    {
        /* The selected channel is disabled */

        /* Configure control registers */
        MODIFY_REG(DMAx->CH[Channel].CTL0,
                   (DMA_CH_CTL0_LLP_SRC_EN | DMA_CH_CTL0_LLP_DST_EN | DMA_CH_CTL0_TT_FC |
                   DMA_CH_CTL0_SRC_MSIZE | DMA_CH_CTL0_DST_MSIZE | DMA_CH_CTL0_SINC |
                   DMA_CH_CTL0_DINC | DMA_CH_CTL0_STW | DMA_CH_CTL0_DTW),
                   (((uint32_t)DMA_InitStruct->SrcLinkedListCtrl << DMA_CH_CTL0_LLP_SRC_EN_Pos) |
                   ((uint32_t)DMA_InitStruct->DstLinkedListCtrl << DMA_CH_CTL0_LLP_DST_EN_Pos) |
                   ((uint32_t)DMA_InitStruct->Direction) |
                   ((uint32_t)DMA_InitStruct->SrcBurstLength << DMA_CH_CTL0_SRC_MSIZE_Pos) |
                   ((uint32_t)DMA_InitStruct->DstBurstLength << DMA_CH_CTL0_DST_MSIZE_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcIncMode << DMA_CH_CTL0_SINC_Pos) |
                   ((uint32_t)DMA_InitStruct->DstIncMode << DMA_CH_CTL0_DINC_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcTransferWidth << DMA_CH_CTL0_STW_Pos) |
                   ((uint32_t)DMA_InitStruct->DstTransferWidth << DMA_CH_CTL0_DTW_Pos)));
        MODIFY_REG(DMAx->CH[Channel].CTL1,
                   (DMA_CH_CTL1_DONE | DMA_CH_CTL1_BLOCK_TS),
                   ((uint32_t)DMA_InitStruct->NbData << DMA_CH_CTL1_BLOCK_TS_Pos));
        MODIFY_REG(DMAx->CH[Channel].CFG0,
                   (DMA_CH_CFG0_RELOAD_DST | DMA_CH_CFG0_RELOAD_SRC | DMA_CH_CFG0_SRC_HS_POL |
                   DMA_CH_CFG0_DST_HS_POL | DMA_CH_CFG0_HS_SEL_SRC | DMA_CH_CFG0_HS_SEL_DST |
                   DMA_CH_CFG0_SUSP | DMA_CH_CFG0_PRIOR),
                   (((uint32_t)DMA_InitStruct->DstMode << DMA_CH_CFG0_RELOAD_DST_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcMode << DMA_CH_CFG0_RELOAD_SRC_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcHWHandshakePolarity << DMA_CH_CFG0_SRC_HS_POL_Pos) |
                   ((uint32_t)DMA_InitStruct->DstHWHandshakePolarity << DMA_CH_CFG0_DST_HS_POL_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcHandshake << DMA_CH_CFG0_HS_SEL_SRC_Pos) |
                   ((uint32_t)DMA_InitStruct->DstHandshake << DMA_CH_CFG0_HS_SEL_DST_Pos) |
                   ((uint32_t)DMA_InitStruct->Priority)));
        MODIFY_REG(DMAx->CH[Channel].CFG1,
                   (DMA_CH_CFG1_DST_PER | DMA_CH_CFG1_SRC_PER |
                   DMA_CH_CFG1_PROTCTL | DMA_CH_CFG1_FIFO_MODE),
                   (((uint32_t)DMA_InitStruct->DstHWHandshakeInterface << DMA_CH_CFG1_DST_PER_Pos) |
                   ((uint32_t)DMA_InitStruct->SrcHWHandshakeInterface << DMA_CH_CFG1_SRC_PER_Pos) |
                   (0x1UL << DMA_CH_CFG1_PROTCTL_Pos) |
                   ((uint32_t)DMA_InitStruct->FIFOMode)));

        /* Set source address register */
        WRITE_REG(DMAx->CH[Channel].SAR, DMA_InitStruct->SrcAddress);

        /* Set destination address register */
        WRITE_REG(DMAx->CH[Channel].DAR, DMA_InitStruct->DstAddress);

        /* Set linked list pointer register */
        WRITE_REG(DMAx->CH[Channel].LLP, DMA_InitStruct->LinkedListPointer);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of DMA_InitStruct_TypeDef structure to default value.
 * @param DMA_InitStruct The pointer to DMA_InitStruct_TypeDef structure.
 */
void DMA_StructInit(DMA_InitStruct_TypeDef *DMA_InitStruct)
{
    /* Set DMA_InitStruct fields to default values */
    DMA_InitStruct->SrcAddress = 0x0UL;
    DMA_InitStruct->SrcMode = DMA_MODE_NORMAL;
    DMA_InitStruct->SrcIncMode = DMA_INC_MODE_INCREMENT;
    DMA_InitStruct->SrcTransferWidth = DMA_TR_WIDTH_8BITS;
    DMA_InitStruct->SrcBurstLength = DMA_BURST_LENGTH_4;
    DMA_InitStruct->SrcLinkedListCtrl = DISABLE;
    DMA_InitStruct->SrcHandshake = DMA_HS_SOFTWARE;
    DMA_InitStruct->SrcHWHandshakePolarity = DMA_HS_POLARITY_HIGH;
    DMA_InitStruct->SrcHWHandshakeInterface = 0U;
    DMA_InitStruct->DstAddress = 0x0UL;
    DMA_InitStruct->DstMode = DMA_MODE_NORMAL;
    DMA_InitStruct->DstIncMode = DMA_INC_MODE_INCREMENT;
    DMA_InitStruct->DstTransferWidth = DMA_TR_WIDTH_8BITS;
    DMA_InitStruct->DstBurstLength = DMA_BURST_LENGTH_4;
    DMA_InitStruct->DstLinkedListCtrl = DISABLE;
    DMA_InitStruct->DstHandshake = DMA_HS_SOFTWARE;
    DMA_InitStruct->DstHWHandshakePolarity = DMA_HS_POLARITY_HIGH;
    DMA_InitStruct->DstHWHandshakeInterface = 0U;
    DMA_InitStruct->Direction = DMA_DIR_PERIPH_TO_PERIPH;
    DMA_InitStruct->Priority = DMA_PRIORITY_0;
    DMA_InitStruct->FIFOMode = DMA_FIFO_MODE_0;
    DMA_InitStruct->LinkedListPointer = 0x0UL;
    DMA_InitStruct->NbData = 2U;
}

/**
 * @brief Initializes a DMA block descriptor (LLI).
 * @note DMA_InitStruct structure should be initialized prior to calling this function.
 * @param LLI The pointer to the block descriptor.
 * @param DMA_InitStruct The pointer to DMA_InitStruct_TypeDef structure.
 */
void DMA_LLI_Init(DMA_LLI_TypeDef *LLI, DMA_InitStruct_TypeDef *DMA_InitStruct)
{
    /* Check parameters */
    assert(IS_DMA_DIRECTION(DMA_InitStruct->Direction));
    assert(IS_DMA_INC_MODE(DMA_InitStruct->SrcIncMode));
    assert(IS_DMA_INC_MODE(DMA_InitStruct->DstIncMode));
    assert(IS_DMA_TR_WIDTH(DMA_InitStruct->SrcTransferWidth));
    assert(IS_DMA_TR_WIDTH(DMA_InitStruct->DstTransferWidth));
    assert(IS_DMA_BURST_LENGTH(DMA_InitStruct->SrcBurstLength));
    assert(IS_DMA_BURST_LENGTH(DMA_InitStruct->DstBurstLength));
    assert(IS_FUNCTIONAL_STATE(DMA_InitStruct->SrcLinkedListCtrl));
    assert(IS_FUNCTIONAL_STATE(DMA_InitStruct->DstLinkedListCtrl));

    /* Set control registers */
    LLI->CTL0 = (((uint32_t)DMA_InitStruct->SrcLinkedListCtrl << DMA_CH_CTL0_LLP_SRC_EN_Pos) |
                 ((uint32_t)DMA_InitStruct->DstLinkedListCtrl << DMA_CH_CTL0_LLP_DST_EN_Pos) |
                 ((uint32_t)DMA_InitStruct->Direction) |
                 ((uint32_t)DMA_InitStruct->SrcBurstLength << DMA_CH_CTL0_SRC_MSIZE_Pos) |
                 ((uint32_t)DMA_InitStruct->DstBurstLength << DMA_CH_CTL0_DST_MSIZE_Pos) |
                 ((uint32_t)DMA_InitStruct->SrcIncMode << DMA_CH_CTL0_SINC_Pos) |
                 ((uint32_t)DMA_InitStruct->DstIncMode << DMA_CH_CTL0_DINC_Pos) |
                 ((uint32_t)DMA_InitStruct->SrcTransferWidth << DMA_CH_CTL0_STW_Pos) |
                 ((uint32_t)DMA_InitStruct->DstTransferWidth << DMA_CH_CTL0_DTW_Pos));
    LLI->CTL1 = ((uint32_t)DMA_InitStruct->NbData << DMA_CH_CTL1_BLOCK_TS_Pos);

    /* Set source address register */
    LLI->SAR = DMA_InitStruct->SrcAddress;

    /* Set destination address register */
    LLI->DAR = DMA_InitStruct->DstAddress;

    /* Set linked list pointer register */
    LLI->LLP = DMA_InitStruct->LinkedListPointer;
}

/**
 * ******************************************************************************
 *  @file       bmcu_spi.c
 *  @author     Baikal electronics SDK team
 *  @brief      Serial Peripheral Interface (SPI) module driver
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - SPI Init/Deinit functions implementation
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bmcu_spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define IS_SPI_POLARITY(__VALUE__)   ((__VALUE__) == SPI_POLARITY_LOW || \
                                      (__VALUE__) == SPI_POLARITY_HIGH)

#define IS_SPI_PHASE(__VALUE__)      ((__VALUE__) == SPI_PHASE_MIDDLE || \
                                      (__VALUE__) == SPI_PHASE_START)

#define IS_SPI_XFER_MODE(__VALUE__)  ((__VALUE__) == SPI_XFER_TXRX || \
                                      (__VALUE__) == SPI_XFER_TX || \
                                      (__VALUE__) == SPI_XFER_RX || \
                                      (__VALUE__) == SPI_XFER_EEPROM )

#define IS_SPI_DATA_FRAME(__VALUE__) ((__VALUE__) == SPI_DATA_FRAME_SIZE_4 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_5 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_6 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_7 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_8 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_9 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_10 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_11 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_12 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_13 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_14 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_15 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_16 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_17 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_18 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_19 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_20 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_21 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_22 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_23 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_24 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_25 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_26 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_27 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_28 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_29 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_30 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_31 || \
                                      (__VALUE__) == SPI_DATA_FRAME_SIZE_32)

#define IS_SPI_SLAVESEL(__VALUE__)   ((__VALUE__) == SPI_SSEL_NOT_SELECTED || \
                                      (__VALUE__) == SPI_SSEL_SELECTED)

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void
SPI_StructInit (SPI_InitStruct_TypeDef *SPI_InitStruct) {
    SPI_InitStruct->ClkDivider      = SPI_CLKDIVIDER_DEFAULT;
    SPI_InitStruct->Polarity        = SPI_POLARITY_LOW;
    SPI_InitStruct->Phase           = SPI_PHASE_MIDDLE;
    SPI_InitStruct->XferMode        = SPI_XFER_TXRX;
    SPI_InitStruct->DataFrameSize   = SPI_DATA_FRAME_SIZE_8;
    SPI_InitStruct->SlaveSel        = SPI_SSEL_NOT_SELECTED;
    SPI_InitStruct->SlaveSelToggle  = DISABLE;
    SPI_InitStruct->EndianConv      = DISABLE;
    SPI_InitStruct->ShiftRegLoop    = DISABLE;
    SPI_InitStruct->FIFOTxThreshold = SPI_FIFOTXTHRESHOLD_DEFAULT;
    SPI_InitStruct->FIFORxThreshold = SPI_FIFORXTHRESHOLD_DEFAULT;
    SPI_InitStruct->RxSampleDelay   = SPI_RX_DELAY_DEPTH_DEFAULT;
}


ErrorStatus
SPI_Init (SPI_TypeDef *SPIx, SPI_InitStruct_TypeDef *SPI_InitStruct) {
    ErrorStatus status = ERROR;

    assert(IS_SPI_ALL_INSTANCE(SPIx));
    assert(IS_SPI_POLARITY(SPI_InitStruct->Polarity));
    assert(IS_SPI_PHASE(SPI_InitStruct->Phase));
    assert(IS_SPI_XFER_MODE(SPI_InitStruct->XferMode));
    assert(IS_SPI_DATA_FRAME(SPI_InitStruct->DataFrameSize));
    assert(IS_SPI_SLAVESEL(SPI_InitStruct->SlaveSel));

    if (DISABLE == SPI_IsEnabled(SPIx)) {
        SPI_SetClkDivider(SPIx, SPI_InitStruct->ClkDivider);
        SPI_SetClockPolarity(SPIx, SPI_InitStruct->Polarity);
        SPI_SetClockPhase(SPIx, SPI_InitStruct->Phase);
        SPI_SetTransferMode(SPIx, SPI_InitStruct->XferMode);
        SPI_SetDataFrameSize(SPIx, SPI_InitStruct->DataFrameSize);
        SPI_SetSlaveSelect(SPIx, SPI_InitStruct->SlaveSel);

        if (ENABLE == SPI_InitStruct->SlaveSelToggle) {
            SPI_EnableSlaveSelToggle(SPIx);

        } else {
            SPI_DisableSlaveSelToggle(SPIx);
        }

        if (ENABLE == SPI_InitStruct->EndianConv) {
            SPI_EnableEndianConversion(SPIx);

        } else {
            SPI_DisableEndianConversion(SPIx);
        }

        if (ENABLE == SPI_InitStruct->ShiftRegLoop) {
            SPI_EnableShiftRegLoop(SPIx);

        } else {
            SPI_DisableShiftRegLoop(SPIx);
        }

        SPI_SetFIFOTxThreshold(SPIx, SPI_InitStruct->FIFOTxThreshold);
        SPI_SetFIFORxThreshold(SPIx, SPI_InitStruct->FIFORxThreshold);
        SPI_SetRxSampleDelay(SPIx, SPI_InitStruct->RxSampleDelay);

        SPI_ClearIT(SPIx);

        status = SUCCESS;
    }

    return status;
}


ErrorStatus
SPI_DeInit (SPI_TypeDef *SPIx) {
    SPI_InitStruct_TypeDef SPI_InitStruct;
    uint32_t trycount;
    ErrorStatus status = SUCCESS;

    assert(IS_SPI_ALL_INSTANCE(SPIx));    

    SPI_DisableIT(SPIx, SPI_IT_TXE);
    SPI_DisableIT(SPIx, SPI_IT_TXO);
    SPI_DisableIT(SPIx, SPI_IT_RXU);
    SPI_DisableIT(SPIx, SPI_IT_RXO);
    SPI_DisableIT(SPIx, SPI_IT_RXF);
    SPI_DisableIT(SPIx, SPI_IT_MST);
    
    SPI_DisableDMATx(SPIx);
    SPI_DisableDMARx(SPIx);

    // Wait for the SPI idle state
    if (ENABLE == SPI_IsEnabled(SPIx)) {
        trycount = SPI_DEINIT_RETRY_COUNT;
        while (0 == SPI_IsActiveFlag(SPIx, SPI_FLAG_TFE) && trycount--);
        
        trycount = SPI_DEINIT_RETRY_COUNT;
        while (0 != SPI_IsActiveFlag(SPIx, SPI_FLAG_BUSY) && trycount--);

        SPI_Disable(SPIx);
    }

    // Flush receive FIFO
    while (0 != SPI_IsActiveFlag(SPIx, SPI_FLAG_RFNE)) {
        SPI_ReceiveData32(SPIx);
    }

    // Set default configuration
    SPI_StructInit(&SPI_InitStruct);
    SPI_Init(SPIx, &SPI_InitStruct);

    SPI_SetDMATxDataLvl(SPIx, SPI_DMATDL_MAX / 2);
    SPI_SetDMARxDataLvl(SPIx, SPI_DMARDL_MAX / 2);

    return status;
}

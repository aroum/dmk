/**
 * *****************************************************************************
 *  @file       bmcu_qspi.c
 *  @author     Baikal electronics SDK team
 *  @brief      Quad SPI (QSPI) module driver
 *  @version    2.2.0
 *  @date       2026.04.02
 *
 *  File content:
 *      - QSPI Init/Deinit functions implementation
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bmcu_qspi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define IS_QSPI_POLARITY(__VALUE__)        ((__VALUE__) == QSPI_POLARITY_LOW || \
                                            (__VALUE__) == QSPI_POLARITY_HIGH)

#define IS_QSPI_PHASE(__VALUE__)           ((__VALUE__) == QSPI_PHASE_MIDDLE || \
                                            (__VALUE__) == QSPI_PHASE_START)

#define IS_QSPI_XFER_MODE(__VALUE__)       ((__VALUE__) == QSPI_XFER_TXRX || \
                                            (__VALUE__) == QSPI_XFER_TX || \
                                            (__VALUE__) == QSPI_XFER_RX || \
                                            (__VALUE__) == QSPI_XFER_EEPROM )

#define IS_QSPI_DATA_FRAME(__VALUE__)      ((__VALUE__) == QSPI_DATA_FRAME_SIZE_4 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_5 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_6 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_7 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_8 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_9 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_10 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_11 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_12 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_13 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_14 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_15 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_16 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_17 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_18 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_19 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_20 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_21 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_22 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_23 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_24 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_25 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_26 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_27 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_28 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_29 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_30 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_31 || \
                                            (__VALUE__) == QSPI_DATA_FRAME_SIZE_32)

#define IS_QSPI_SLAVESEL(__VALUE__)        ((__VALUE__) == QSPI_SSEL_NOT_SELECTED || \
                                            (__VALUE__) == QSPI_SSEL_0 || \
                                            (__VALUE__) == QSPI_SSEL_1 || \
                                            (__VALUE__) == QSPI_SSEL_2)

#define IS_QSPI_FRAMEFORMAT(__VALUE__)     ((__VALUE__) == QSPI_FRAMEFORMAT_STD || \
                                            (__VALUE__) == QSPI_FRAMEFORMAT_DUAL || \
                                            (__VALUE__) == QSPI_FRAMEFORMAT_QUAD)

#define IS_QSPI_ADDRINTXFERMODE(__VALUE__) ((__VALUE__) == QSPI_XFER_STD || \
                                            (__VALUE__) == QSPI_XFER_INST || \
                                            (__VALUE__) == QSPI_XFER_BOTH)

#define IS_QSPI_ADDRLEN(__VALUE__)         ((__VALUE__) == QSPI_ADDRLEN_0 || \
                                            (__VALUE__) == QSPI_ADDRLEN_4 || \
                                            (__VALUE__) == QSPI_ADDRLEN_8 || \
                                            (__VALUE__) == QSPI_ADDRLEN_12 || \
                                            (__VALUE__) == QSPI_ADDRLEN_16 || \
                                            (__VALUE__) == QSPI_ADDRLEN_20 || \
                                            (__VALUE__) == QSPI_ADDRLEN_24 || \
                                            (__VALUE__) == QSPI_ADDRLEN_28 || \
                                            (__VALUE__) == QSPI_ADDRLEN_32 || \
                                            (__VALUE__) == QSPI_ADDRLEN_36 || \
                                            (__VALUE__) == QSPI_ADDRLEN_40 || \
                                            (__VALUE__) == QSPI_ADDRLEN_44 || \
                                            (__VALUE__) == QSPI_ADDRLEN_48 || \
                                            (__VALUE__) == QSPI_ADDRLEN_52 || \
                                            (__VALUE__) == QSPI_ADDRLEN_56 || \
                                            (__VALUE__) == QSPI_ADDRLEN_60)

#define IS_QSPI_INSTLEN(__VALUE__)         ((__VALUE__) == QSPI_INSTLEN_0 || \
                                            (__VALUE__) == QSPI_INSTLEN_4 || \
                                            (__VALUE__) == QSPI_INSTLEN_8 || \
                                            (__VALUE__) == QSPI_INSTLEN_16)

#define IS_QSPI_WAITCYCLES(__VALUE__)      ((__VALUE__) == QSPI_WAITCYCLES_0 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_1 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_2 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_3 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_4 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_5 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_6 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_7 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_8 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_9 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_10 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_11 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_12 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_13 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_14 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_15 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_16 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_17 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_18 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_19 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_20 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_21 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_22 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_23 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_24 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_25 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_26 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_27 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_28 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_29 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_30 || \
                                            (__VALUE__) == QSPI_WAITCYCLES_31)
                                         
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void
QSPI_StructInit (QSPI_InitStruct_TypeDef *QSPI_InitStruct) {
    QSPI_InitStruct->ClkDivider       = QSPI_CLKDIVIDER_DEFAULT;
    QSPI_InitStruct->Polarity         = QSPI_POLARITY_LOW;
    QSPI_InitStruct->Phase            = QSPI_PHASE_MIDDLE;
    QSPI_InitStruct->XferMode         = QSPI_XFER_TXRX;
    QSPI_InitStruct->DataFrameSize    = QSPI_DATA_FRAME_SIZE_8;
    QSPI_InitStruct->SlaveSel         = QSPI_SSEL_NOT_SELECTED;
    QSPI_InitStruct->FrameFormat      = QSPI_FRAMEFORMAT_STD;
    QSPI_InitStruct->AddrInstXferMode = QSPI_XFER_STD;
    QSPI_InitStruct->AddrLen          = QSPI_ADDRLEN_0;
    QSPI_InitStruct->InstLen          = QSPI_INSTLEN_8;
    QSPI_InitStruct->WaitCycles       = QSPI_WAITCYCLES_0;
    QSPI_InitStruct->SlaveSelToggle   = DISABLE;
    QSPI_InitStruct->EndianConv       = DISABLE;
    QSPI_InitStruct->ShiftRegLoop     = DISABLE;
    QSPI_InitStruct->DDR              = DISABLE;
    QSPI_InitStruct->InstDDR          = DISABLE;
    QSPI_InitStruct->FIFOTxThreshold  = QSPI_FIFOTXTHRESHOLD_DEFAULT;
    QSPI_InitStruct->FIFORxThreshold  = QSPI_FIFORXTHRESHOLD_DEFAULT;
    QSPI_InitStruct->RxSampleDelay    = QSPI_RX_DELAY_DEPTH_DEFAULT;
}


ErrorStatus
QSPI_Init (QSPI_TypeDef *QSPIx, QSPI_InitStruct_TypeDef *QSPI_InitStruct) {
    ErrorStatus status = ERROR;

    assert(IS_QSPI_ALL_INSTANCE(QSPIx));
    assert(IS_QSPI_POLARITY(QSPI_InitStruct->Polarity));
    assert(IS_QSPI_PHASE(QSPI_InitStruct->Phase));
    assert(IS_QSPI_XFER_MODE(QSPI_InitStruct->XferMode));
    assert(IS_QSPI_DATA_FRAME(QSPI_InitStruct->DataFrameSize));
    assert(IS_QSPI_SLAVESEL(QSPI_InitStruct->SlaveSel));
    assert(IS_QSPI_FRAMEFORMAT(QSPI_InitStruct->FrameFormat));
    assert(IS_QSPI_ADDRINTXFERMODE(QSPI_InitStruct->AddrInstXferMode));
    assert(IS_QSPI_ADDRLEN(QSPI_InitStruct->AddrLen));
    assert(IS_QSPI_INSTLEN(QSPI_InitStruct->InstLen));
    assert(IS_QSPI_WAITCYCLES(QSPI_InitStruct->WaitCycles));

    if (DISABLE == QSPI_IsEnabled(QSPIx)) {
        QSPI_SetClkDivider(QSPIx, QSPI_InitStruct->ClkDivider);
        QSPI_SetClockPolarity(QSPIx, QSPI_InitStruct->Polarity);
        QSPI_SetClockPhase(QSPIx, QSPI_InitStruct->Phase);
        QSPI_SetTransferMode(QSPIx, QSPI_InitStruct->XferMode);
        QSPI_SetDataFrameSize(QSPIx, QSPI_InitStruct->DataFrameSize);
        QSPI_SetSlaveSelect(QSPIx, QSPI_InitStruct->SlaveSel);
        QSPI_SetFrameFormat(QSPIx, QSPI_InitStruct->FrameFormat);
        QSPI_SetAddrInstXferMode(QSPIx, QSPI_InitStruct->AddrInstXferMode);
        QSPI_SetAddrLen(QSPIx, QSPI_InitStruct->AddrLen);
        QSPI_SetInstLen(QSPIx, QSPI_InitStruct->InstLen);
        QSPI_SetWaitCycles(QSPIx, QSPI_InitStruct->WaitCycles);

        if (ENABLE == QSPI_InitStruct->SlaveSelToggle) {
            QSPI_EnableSlaveSelToggle(QSPIx);

        } else {
            QSPI_DisableSlaveSelToggle(QSPIx);
        }

        if (ENABLE == QSPI_InitStruct->EndianConv) {
            QSPI_EnableEndianConversion(QSPIx);

        } else {
            QSPI_DisableEndianConversion(QSPIx);
        }

        if (ENABLE == QSPI_InitStruct->ShiftRegLoop) {
            QSPI_EnableShiftRegLoop(QSPIx);

        } else {
            QSPI_DisableShiftRegLoop(QSPIx);
        }
    
        if (ENABLE == QSPI_InitStruct->DDR) {
            QSPI_EnableDDR(QSPIx);

        } else {
            QSPI_DisableDDR(QSPIx);
        }
        
        if (ENABLE == QSPI_InitStruct->InstDDR) {
            QSPI_EnableInstDDR(QSPIx);

        } else {
            QSPI_DisableInstDDR(QSPIx);
        }

        QSPI_SetFIFOTxThreshold(QSPIx, QSPI_InitStruct->FIFOTxThreshold);
        QSPI_SetFIFORxThreshold(QSPIx, QSPI_InitStruct->FIFORxThreshold);
        QSPI_SetRxSampleDelay(QSPIx, QSPI_InitStruct->RxSampleDelay);

        QSPI_ClearIT(QSPIx);

        status = SUCCESS;
    }

    return status;
}


ErrorStatus
QSPI_DeInit (QSPI_TypeDef *QSPIx) {
    QSPI_InitStruct_TypeDef QSPI_InitStruct;
    uint32_t trycount;
    ErrorStatus status = SUCCESS;

    assert(IS_QSPI_ALL_INSTANCE(QSPIx));    

    QSPI_DisableIT(QSPIx, QSPI_IT_TXE);
    QSPI_DisableIT(QSPIx, QSPI_IT_TXO);
    QSPI_DisableIT(QSPIx, QSPI_IT_RXU);
    QSPI_DisableIT(QSPIx, QSPI_IT_RXO);
    QSPI_DisableIT(QSPIx, QSPI_IT_RXF);
    QSPI_DisableIT(QSPIx, QSPI_IT_MST);
    
    QSPI_DisableDMATx(QSPIx);
    QSPI_DisableDMARx(QSPIx);

    // Wait for the SPI idle state
    if (ENABLE == QSPI_IsEnabled(QSPIx)) {
        trycount =QSPI_DEINIT_RETRY_COUNT;
        while (0 == QSPI_IsActiveFlag(QSPIx, QSPI_FLAG_TFE) && trycount--);
        
        trycount = QSPI_DEINIT_RETRY_COUNT;
        while (0 != QSPI_IsActiveFlag(QSPIx, QSPI_FLAG_BUSY) && trycount--);

        QSPI_Disable(QSPIx);
    }

    // Flush receive FIFO
    while (0 != QSPI_IsActiveFlag(QSPIx, QSPI_FLAG_RFNE)) {
        QSPI_ReceiveData32(QSPIx);
    }

    // Set default configuration
    QSPI_StructInit(&QSPI_InitStruct);
    QSPI_Init(QSPIx, &QSPI_InitStruct);

    QSPI_SetDMATxDataLvl(QSPIx, QSPI_DMATDL_MAX / 2);
    QSPI_SetDMARxDataLvl(QSPIx, QSPI_DMARDL_MAX / 2);

    return status;
}

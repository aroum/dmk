/**
 * *****************************************************************************
 *  @file       bmcu_canfd.c
 *  @author     Baikal electronics SDK team
 *  @brief      Controller Area Network Flexible Data-Rate (CAN-FD)
 *              module driver
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

#include "bmcu_canfd.h"
#include "bmcu_def.h"

#define IS_CANFD_TX_BUFFER(__VALUE__)           ((__VALUE__) < ((uint8_t)CANFD_NUM_OF_TX_BUF))

#define IS_CANFD_RX_FIFO(__VALUE__)             (((__VALUE__) == CANFD_RX_FIFO0) || \
                                                 ((__VALUE__) == CANFD_RX_FIFO1))

#define IS_CANFD_OP_MODE(__VALUE__)             (((__VALUE__) == CANFD_OPMODE_CONFIG) ||  \
                                                 ((__VALUE__) == CANFD_OPMODE_NORMAL) || \
                                                 ((__VALUE__) == CANFD_OPMODE_LOOPBACK) || \
                                                 ((__VALUE__) == CANFD_OPMODE_SLEEP) || \
                                                 ((__VALUE__) == CANFD_OPMODE_SNOOP) || \
                                                 ((__VALUE__) == CANFD_OPMODE_PEE) || \
                                                 ((__VALUE__) == CANFD_OPMODE_BR))

#define IS_CANFD_STDID(__VALUE__)               ((__VALUE__) <= ((uint32_t)0x7FFUL))
#define IS_CANFD_EXTID(__VALUE__)               ((__VALUE__) <= ((uint32_t)0x1FFFFFFFUL))

#define IS_CANFD_IDE(__VALUE__)                 (((__VALUE__) == CANFD_IDE_STD) ||  \
                                                 ((__VALUE__) == CANFD_IDE_EXT))

#define IS_CANFD_EDL(__VALUE__)                 (((__VALUE__) == CANFD_EDL_CAN) ||  \
                                                 ((__VALUE__) == CANFD_EDL_CANFD))

#define IS_CANFD_RTR(__VALUE__)                 (((__VALUE__) == CANFD_RTR_DATA) ||  \
                                                 ((__VALUE__) == CANFD_RTR_REMOTE))

#define IS_CANFD_BRS(__VALUE__)                 (((__VALUE__) == CANFD_BRS_DISABLE) ||  \
                                                 ((__VALUE__) == CANFD_BRS_ENABLE))

#define IS_CANFD_DLC(__VALUE__)                 (((__VALUE__) == CANFD_DLC_0BYTES) ||  \
                                                 ((__VALUE__) == CANFD_DLC_1BYTE) || \
                                                 ((__VALUE__) == CANFD_DLC_2BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_3BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_4BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_5BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_6BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_7BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_8BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_12BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_16BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_20BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_24BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_32BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_48BYTES) || \
                                                 ((__VALUE__) == CANFD_DLC_64BYTES))

#define IS_CANFD_EFC(__VALUE__)                 (((__VALUE__) == CANFD_EFC_DISABLE) ||  \
                                                 ((__VALUE__) == CANFD_EFC_ENABLE))

#define IS_CANFD_ET(__VALUE__)                  (((__VALUE__) == CANFD_ET_FORCED) ||  \
                                                 ((__VALUE__) == CANFD_ET_TRANSMITTED))

#define IS_CANFD_ESI(__VALUE__)                 (((__VALUE__) == CANFD_ESI_ACTIVE) ||  \
                                                 ((__VALUE__) == CANFD_ESI_PASSIVE))

#define IS_CANFD_MFI(__VALUE__)                 ((__VALUE__) <= ((uint8_t)31U))

#define IS_CANFD_TXE_FWM(__VALUE__)             (((__VALUE__) >= ((uint8_t)1U)) && \
                                                 ((__VALUE__) <= ((uint8_t)31U)))

#define IS_CANFD_RX_FWM(__VALUE__)              (((__VALUE__) >= ((uint8_t)1U)) && \
                                                 ((__VALUE__) <= ((uint8_t)63U)))

#define IS_CANFD_RX_FP(__VALUE__)               ((__VALUE__) <= ((uint8_t)31U))

#define IS_CANFD_AP_PRESCALER(__VALUE__)        (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 256U))

#define IS_CANFD_AP_SJW(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 128U))

#define IS_CANFD_AP_TS1(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 256U))

#define IS_CANFD_AP_TS2(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 128U))

#define IS_CANFD_DP_PRESCALER(__VALUE__)        (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 256U))

#define IS_CANFD_DP_SJW(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 16U))

#define IS_CANFD_DP_TS1(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 32U))

#define IS_CANFD_DP_TS2(__VALUE__)              (((__VALUE__) >= 1U) && \
                                                 ((__VALUE__) <= 16U))

#define IS_CANFD_DP_TDCOFF(__VALUE__)           ((__VALUE__) <= 63U)

/**
 * @brief Deinitializes a CAN instance.
 * @note The function performs software reset of the CAN core. After
 *       performing reset wait for 16 APB clock cycles before initiating
 *       next APB transaction.
 * @param CANFDx The CAN instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The CAN instance have been deinitialized.
 *         - ERROR: The CAN instance have not been deinitialized.
 */
ErrorStatus CANFD_DeInit(CANFD_TypeDef *CANFDx)
{
    ErrorStatus status = SUCCESS;
  
    /* Check parameters */
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    /* Reset core */
    SET_BIT(CANFDx->SRR, CANFD_SRR_SRST);
  
    return status;
}

/**
 * @brief Initializes a CAN instance.
 * @note CANFD_InitStruct structure should be initialized prior to calling this function.
 * @param CANFDx The CAN instance.
 * @param CANFD_InitStruct The pointer to CANFD_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The CAN instance have been initialized.
 *         - ERROR: The CAN instance have not been initialized.
 */
ErrorStatus CANFD_Init(CANFD_TypeDef *CANFDx, CANFD_InitStruct_TypeDef *CANFD_InitStruct)
{
    ErrorStatus status = SUCCESS;
  
    /* Check parameters */
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_OP_MODE(CANFD_InitStruct->Mode));
    assert(IS_CANFD_AP_PRESCALER(CANFD_InitStruct->AP_Prescaler));
    assert(IS_CANFD_AP_SJW(CANFD_InitStruct->AP_SJW));
    assert(IS_CANFD_AP_TS1(CANFD_InitStruct->AP_TS1));
    assert(IS_CANFD_AP_TS2(CANFD_InitStruct->AP_TS2));
    assert(IS_CANFD_DP_PRESCALER(CANFD_InitStruct->DP_Prescaler));
    assert(IS_CANFD_DP_SJW(CANFD_InitStruct->DP_SJW));
    assert(IS_CANFD_DP_TS1(CANFD_InitStruct->DP_TS1));
    assert(IS_CANFD_DP_TS2(CANFD_InitStruct->DP_TS2));
    assert(IS_FUNCTIONAL_STATE(CANFD_InitStruct->DP_TDC));
    assert(IS_CANFD_DP_TDCOFF(CANFD_InitStruct->DP_TDCOFF));
    assert(IS_FUNCTIONAL_STATE(CANFD_InitStruct->BRSD));
    assert(IS_FUNCTIONAL_STATE(CANFD_InitStruct->DAR));
    assert(IS_FUNCTIONAL_STATE(CANFD_InitStruct->DPEE));
    assert(IS_FUNCTIONAL_STATE(CANFD_InitStruct->ABR));

    if (CANFD_IsEnabled(CANFDx) == 0U)
    {
        /* CAN is in Configuration mode */

        /* Set operating mode */
        switch (CANFD_InitStruct->Mode)
        {
            case CANFD_OPMODE_CONFIG:
            {
                /* Already in Configuration mode */
                /* No additional configuration required */
            }
            break;

            case CANFD_OPMODE_NORMAL:
            {
                /* Normal mode */
                CLEAR_BIT(CANFDx->MSR, (CANFD_MSR_SNOOP | CANFD_MSR_LBACK | CANFD_MSR_SLEEP));
            }
            break;

            case CANFD_OPMODE_LOOPBACK:
            {
                /* Loopback mode */
                MODIFY_REG(CANFDx->MSR,
                           (CANFD_MSR_SNOOP | CANFD_MSR_SLEEP),
                           CANFD_MSR_LBACK);
            }
            break;

            case CANFD_OPMODE_SLEEP:
            {
                /* Sleep mode */
                MODIFY_REG(CANFDx->MSR,
                           (CANFD_MSR_SNOOP | CANFD_MSR_LBACK),
                           CANFD_MSR_SLEEP);
            }
            break;

            case CANFD_OPMODE_SNOOP:
            {
                /* Snoop mode */
                MODIFY_REG(CANFDx->MSR,
                           (CANFD_MSR_LBACK | CANFD_MSR_SLEEP),
                           CANFD_MSR_SNOOP);
            }
            break;

            default:
            {
                /* The selected mode is not valid init mode */
                status = ERROR;
            }
            break;
        }

        if (status == SUCCESS)
        {
            /* Mode */
            MODIFY_REG(CANFDx->MSR,
                       (CANFD_MSR_ABR | CANFD_MSR_DPEE | CANFD_MSR_DAR | CANFD_MSR_BRSD),
                       (((uint32_t)CANFD_InitStruct->ABR << CANFD_MSR_ABR_Pos) |
                       ((uint32_t)CANFD_InitStruct->DPEE << CANFD_MSR_DPEE_Pos) |
                       ((uint32_t)CANFD_InitStruct->DAR << CANFD_MSR_DAR_Pos) |
                       ((uint32_t)CANFD_InitStruct->BRSD << CANFD_MSR_BRSD_Pos)));

            /* Bit timing */

            /* Arbitration phase */
            WRITE_REG(CANFDx->BRPR,
                      ((uint32_t)(CANFD_InitStruct->AP_Prescaler - 1U) << CANFD_BRPR_PR_Pos) & CANFD_BRPR_PR);
            WRITE_REG(CANFDx->BTR,
                      ((((uint32_t)(CANFD_InitStruct->AP_SJW - 1U) << CANFD_BTR_SJW_Pos) & CANFD_BTR_SJW) |
                      (((uint32_t)(CANFD_InitStruct->AP_TS2 - 1U) << CANFD_BTR_TS2_Pos) & CANFD_BTR_TS2) |
                      (((uint32_t)(CANFD_InitStruct->AP_TS1 - 1U) << CANFD_BTR_TS1_Pos) & CANFD_BTR_TS1)));

            /* Data phase */
            WRITE_REG(CANFDx->DP_BRPR,
                      ((((uint32_t)CANFD_InitStruct->DP_TDC << CANFD_DP_BRPR_TDC_Pos) & CANFD_DP_BRPR_TDC) |
                      (((uint32_t)CANFD_InitStruct->DP_TDCOFF << CANFD_DP_BRPR_TDCOFF_Pos) & CANFD_DP_BRPR_TDCOFF) |
                      (((uint32_t)(CANFD_InitStruct->DP_Prescaler - 1U) << CANFD_DP_BRPR_PR_Pos) & CANFD_DP_BRPR_PR)));
            WRITE_REG(CANFDx->DP_BTR,
                      ((((uint32_t)(CANFD_InitStruct->DP_SJW - 1U) << CANFD_DP_BTR_SJW_Pos) & CANFD_DP_BTR_SJW) |
                      (((uint32_t)(CANFD_InitStruct->DP_TS2 - 1U) << CANFD_DP_BTR_TS2_Pos) & CANFD_DP_BTR_TS2) |
                      (((uint32_t)(CANFD_InitStruct->DP_TS1 - 1U) << CANFD_DP_BTR_TS1_Pos) & CANFD_DP_BTR_TS1)));

            /* Internal Timing Optimization */
            if ((CANFD_InitStruct->AP_Prescaler == 1U) &&
                (CANFD_InitStruct->DP_Prescaler == 1U))
            {
                MODIFY_REG(CANFDx->MSR, CANFD_MSR_ITO, (0x8UL << CANFD_MSR_ITO_Pos));
            }
            else
            {
                CLEAR_BIT(CANFDx->MSR, CANFD_MSR_ITO);
            }
        }
    }

    return status;
}

/**
 * @brief Sets each field of CANFD_InitStruct_TypeDef structure to default value.
 * @param CANFD_InitStruct The pointer to CANFD_InitStruct_TypeDef structure.
 */
void CANFD_StructInit(CANFD_InitStruct_TypeDef *CANFD_InitStruct)
{
    /* Set CANFD_InitStruct fields to default values */
    CANFD_InitStruct->Mode = CANFD_OPMODE_NORMAL;
    CANFD_InitStruct->AP_Prescaler = 1U;
    CANFD_InitStruct->AP_SJW = 1U;
    CANFD_InitStruct->AP_TS1 = 1U;
    CANFD_InitStruct->AP_TS2 = 1U;
    CANFD_InitStruct->DP_Prescaler = 1U;
    CANFD_InitStruct->DP_SJW = 1U;
    CANFD_InitStruct->DP_TS1 = 1U;
    CANFD_InitStruct->DP_TS2 = 1U;
    CANFD_InitStruct->DP_TDC = DISABLE;
    CANFD_InitStruct->DP_TDCOFF = 0U;
    CANFD_InitStruct->BRSD = DISABLE;
    CANFD_InitStruct->DAR = DISABLE;
    CANFD_InitStruct->DPEE = DISABLE;
    CANFD_InitStruct->ABR = DISABLE;
}

/**
 * @brief Initializes a CAN acceptance filter.
 * @note CANFD_FilterInitStruct_TypeDef structure should be initialized prior to
 *       calling this function.
 * @param CANFDx The CAN instance.
 * @param CAN_FilterInitStruct The pointer to CANFD_FilterInitStruct_TypeDef structure.
 */
void CANFD_FilterInit(CANFD_TypeDef *CANFDx, CANFD_FilterInitStruct_TypeDef *CAN_FilterInitStruct)
{
    /* Check parameters */
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_MFI(CAN_FilterInitStruct->Index));
    assert(IS_FUNCTIONAL_STATE(CAN_FilterInitStruct->Ctrl));
    assert(IS_CANFD_IDE(CAN_FilterInitStruct->IDE));
    assert(IS_FUNCTIONAL_STATE(CAN_FilterInitStruct->CtrlIDE));
    assert(IS_CANFD_RTR(CAN_FilterInitStruct->RTR));
    assert(IS_FUNCTIONAL_STATE(CAN_FilterInitStruct->CtrlRTR));
    assert(IS_CANFD_RTR(CAN_FilterInitStruct->SRR));
    assert(IS_FUNCTIONAL_STATE(CAN_FilterInitStruct->CtrlSRR));

    /* Disable the acceptance filter */
    CLEAR_BIT(CANFDx->AFR, (1UL << CAN_FilterInitStruct->Index));

    if (CAN_FilterInitStruct->CtrlIDE == ENABLE)
    {
        if (CAN_FilterInitStruct->IDE == CANFD_IDE_EXT)
        {
            /* The mask is applicable only to Extended frame */
            WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFIR,
                      (((CAN_FilterInitStruct->ExtId >> 18) & 0x7FFUL) << CANFD_AFIR_STD_AIID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->IDE << CANFD_AFIR_AIIDE_Pos) |
                      ((CAN_FilterInitStruct->ExtId & 0x3FFFFUL) << CANFD_AFIR_EXT_AIID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->RTR << CANFD_AFIR_AIRTR_Pos));
            WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFMR,
                      (((CAN_FilterInitStruct->ExtIdMask >> 18) & 0x7FFUL) << CANFD_AFMR_STD_AMID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->CtrlIDE << CANFD_AFMR_AMIDE_Pos) |
                      ((CAN_FilterInitStruct->ExtIdMask & 0x3FFFFUL) << CANFD_AFMR_EXT_AMID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->CtrlRTR << CANFD_AFMR_AMRTR_Pos));
        }
        else
        {
            /* The mask is applicable only to Standard frame */
            WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFIR,
                      ((CAN_FilterInitStruct->StdId & 0x7FFUL) << CANFD_AFIR_STD_AIID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->SRR << CANFD_AFIR_AISRR_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->IDE << CANFD_AFIR_AIIDE_Pos));
            WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFMR,
                      ((CAN_FilterInitStruct->StdIdMask & 0x7FFUL) << CANFD_AFMR_STD_AMID_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->CtrlSRR << CANFD_AFMR_AMSRR_Pos) |
                      ((uint32_t)CAN_FilterInitStruct->CtrlIDE << CANFD_AFMR_AMIDE_Pos));
        }
    }
    else
    {
        /* The mask is applicable to both Standard and Extended frames */
        WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFIR,
                  (((CAN_FilterInitStruct->ExtId >> 18) & 0x7FFUL) << CANFD_AFIR_STD_AIID_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->SRR << CANFD_AFIR_AISRR_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->IDE << CANFD_AFIR_AIIDE_Pos) |
                  ((CAN_FilterInitStruct->ExtId & 0x3FFFFUL) << CANFD_AFIR_EXT_AIID_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->RTR << CANFD_AFIR_AIRTR_Pos));
        WRITE_REG(CANFDx->AF[CAN_FilterInitStruct->Index].AFMR,
                  (((CAN_FilterInitStruct->ExtIdMask >> 18) & 0x7FFUL) << CANFD_AFMR_STD_AMID_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->CtrlSRR << CANFD_AFMR_AMSRR_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->CtrlIDE << CANFD_AFMR_AMIDE_Pos) |
                  ((CAN_FilterInitStruct->ExtIdMask & 0x3FFFFUL) << CANFD_AFMR_EXT_AMID_Pos) |
                  ((uint32_t)CAN_FilterInitStruct->CtrlRTR << CANFD_AFMR_AMRTR_Pos));
    }

    /* Enable the acceptance filter */
    if (CAN_FilterInitStruct->Ctrl == ENABLE)
    {
        SET_BIT(CANFDx->AFR, (1UL << CAN_FilterInitStruct->Index));
    }
}

/**
 * @brief Returns the current operating mode.
 * @param CANFDx The CAN instance.
 * @returns The operating mode. Can be one of CANFD_OpMode_TypeDef values.
 */
CANFD_OpMode_TypeDef CANFD_GetMode(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    CANFD_OpMode_TypeDef Mode;
	uint32_t Status = READ_REG(CANFDx->SR);

	if ((Status & CANFD_SR_CONFIG) == CANFD_SR_CONFIG)
    {
		Mode = CANFD_OPMODE_CONFIG;
	}
    else if ((Status & CANFD_SR_SLEEP) == CANFD_SR_SLEEP)
    {
		Mode = CANFD_OPMODE_SLEEP;
	}
    else if ((Status & CANFD_SR_LBACK) == CANFD_SR_LBACK)
    {
		Mode = CANFD_OPMODE_LOOPBACK;
	}
    else if ((Status & CANFD_SR_PEE_CONFIG) == CANFD_SR_PEE_CONFIG)
    {
		Mode = CANFD_OPMODE_PEE;
	}
    else if ((Status & CANFD_SR_BSFR_CONFIG) == CANFD_SR_BSFR_CONFIG)
    {
		Mode = CANFD_OPMODE_BR;
	}
    else
    {
		if ((Status & CANFD_SR_SNOOP) == CANFD_SR_SNOOP)
        {
			Mode = CANFD_OPMODE_SNOOP;
		}
        else
        {
			Mode = CANFD_OPMODE_NORMAL;
		}
	}

	return Mode;
}

/**
 * @brief Switches device operating mode.
 * @note This function does not ensure that the device enters the requested 
 *       operating mode. The caller is responsible for checking the
 *       current operating mode.
 * @param CANFDx The CAN instance.
 * @param Mode The operating mode. Can be one of CANFD_OpMode_TypeDef values.
 */
void CANFD_RequestMode(CANFD_TypeDef *CANFDx, CANFD_OpMode_TypeDef Mode)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_OP_MODE(Mode));

    CANFD_OpMode_TypeDef CurrentMode = CANFD_GetMode(CANFDx);

	if ((CurrentMode == CANFD_OPMODE_NORMAL) &&
        (Mode == CANFD_OPMODE_SLEEP))
    {
        /* Request Sleep mode */
        SET_BIT(CANFDx->MSR, CANFD_MSR_SLEEP);
	}
	else if ((CurrentMode == CANFD_OPMODE_SLEEP) &&
		     (Mode == CANFD_OPMODE_NORMAL))
    {
        /* Request Normal mode */
        CLEAR_BIT(CANFDx->MSR, CANFD_MSR_SLEEP);
	}
    else if ((CurrentMode == CANFD_OPMODE_BR) &&
             (Mode != CANFD_OPMODE_BR))
    {
        /* Start bus-off recovery request */
        if (READ_BIT(CANFDx->MSR, CANFD_MSR_ABR) == 0UL)
        {
            SET_BIT(CANFDx->MSR, CANFD_MSR_SBR);
        }
	}
	else
    {
        /* Request Configuration mode */
        CLEAR_BIT(CANFDx->SRR, CANFD_SRR_CEN);

        /* Check if the device has entered Configuration Mode */
        if (CANFD_GetMode(CANFDx) == CANFD_OPMODE_CONFIG)
        {
            switch (Mode)
            {
                case CANFD_OPMODE_NORMAL:
                {
                    /* Request Normal mode */
                    CLEAR_BIT(CANFDx->MSR, CANFD_MSR_SNOOP | CANFD_MSR_LBACK | CANFD_MSR_SLEEP);
                    SET_BIT(CANFDx->SRR, CANFD_SRR_CEN);
                }
                break;

                case CANFD_OPMODE_LOOPBACK:
                {
                    /* Request Loopback mode */
                    MODIFY_REG(CANFDx->MSR,
                               (CANFD_MSR_SNOOP | CANFD_MSR_SLEEP),
                               CANFD_MSR_LBACK);
                    SET_BIT(CANFDx->SRR, CANFD_SRR_CEN);
                }
                break;

                case CANFD_OPMODE_SLEEP:
                {
                    /* Request Sleep mode */
                    MODIFY_REG(CANFDx->MSR,
                               (CANFD_MSR_SNOOP | CANFD_MSR_LBACK),
                               CANFD_MSR_SLEEP);
                    SET_BIT(CANFDx->SRR, CANFD_SRR_CEN);
                }
                break;

                case CANFD_OPMODE_SNOOP:
                {
                    /* Request Snoop mode */
                    MODIFY_REG(CANFDx->MSR,
                               (CANFD_MSR_LBACK | CANFD_MSR_SLEEP),
                               CANFD_MSR_SNOOP);
                    SET_BIT(CANFDx->SRR, CANFD_SRR_CEN);
                }
                break;

                default:
                {
                    /* Stay in Configuration mode */
                }
                break;
            }
        }
	}
}

/**
 * @brief Converts data length to DLC.
 * @param Length The data length.
 * @returns The DLC.
 */
CANFD_DLC_TypeDef CANFD_LENtoDLC(uint8_t Length)
{
    CANFD_DLC_TypeDef DLC;
    
	if (Length <= 8U)
		DLC = (CANFD_DLC_TypeDef)Length;
    else if (Length <= 12U)
		DLC = CANFD_DLC_12BYTES;
    else if (Length <= 16U)
		DLC = CANFD_DLC_16BYTES;
    else if (Length <= 20U)
		DLC = CANFD_DLC_20BYTES;
    else if (Length <= 24U)
		DLC = CANFD_DLC_24BYTES;
    else if (Length <= 32U)
		DLC = CANFD_DLC_32BYTES;
    else if (Length <= 48U)
		DLC = CANFD_DLC_48BYTES;
    else
		DLC = CANFD_DLC_64BYTES;

    return DLC;
}

/**
 * @brief Converts DLC to data length.
 * @param DLC The DLC.
 * @returns The data length.
 */
uint8_t CANFD_DLCtoLEN(CANFD_DLC_TypeDef DLC)
{
    assert(IS_CANFD_DLC(DLC));

    uint8_t Length;
    
	if (DLC <= CANFD_DLC_8BYTES)
		Length = (uint8_t)DLC;
	else if (DLC == CANFD_DLC_12BYTES)
		Length = 12U;
	else if (DLC == CANFD_DLC_16BYTES)
		Length = 16U;
	else if (DLC == CANFD_DLC_20BYTES)
		Length = 20U;
	else if (DLC == CANFD_DLC_24BYTES)
		Length = 24U;
	else if (DLC == CANFD_DLC_32BYTES)
		Length = 32U;
	else if (DLC == CANFD_DLC_48BYTES)
		Length = 48U;
	else
		Length = 64U;

    return Length;
}

/**
 * @brief Initiates the transmission of a message.
 * @param CANFDx The CAN instance.
 * @param TxMessage The pointer to CANFD_TxMsg_TypeDef structure.
 * @retval The number of the buffer that is used for transmission or
 *         CANFD_TX_NO_BUFFER if there is no empty buffer.
 */
uint8_t CANFD_Transmit(CANFD_TypeDef *CANFDx, CANFD_TxMsg_TypeDef *TxMessage)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_IDE(TxMessage->IDE));
    assert(IS_CANFD_EDL(TxMessage->EDL));
    assert(IS_CANFD_RTR(TxMessage->RTR));
    assert(IS_CANFD_BRS(TxMessage->BRS));
    assert(IS_CANFD_DLC(TxMessage->DLC));
    assert(IS_CANFD_EFC(TxMessage->EFC));

    uint32_t RegVal;
    uint8_t TxBufferIndex = 0U;

    /* Poll the TRR register to check current pending transmission requests */
    RegVal = READ_BIT(CANFDx->TRR, CANFD_TXBUF_ALL);
    if (RegVal != CANFD_TXBUF_ALL)
    {
        /* There are free buffers for transmission */

        /* Find index of the first free buffer */
        uint32_t Mask = 0x1UL;
        while (((RegVal & Mask) != 0UL) && 
               (TxBufferIndex < CANFD_NUM_OF_TX_BUF))
        {
            Mask <<= 1;
            TxBufferIndex++;
        }

        /* Prepare a message element in the Tx block RAM */

        /* TB ID */
        RegVal = ((uint32_t)TxMessage->IDE << CANFD_TBID_IDE_Pos);

        if (TxMessage->IDE == CANFD_IDE_STD)
        {
            /* Standard message identifier */
            RegVal |= ((TxMessage->StdId & 0x7FFUL) << CANFD_TBID_STD_ID_Pos);

            if (TxMessage->EDL == CANFD_EDL_CAN)
            {
                RegVal |= ((uint32_t)TxMessage->RTR << CANFD_TBID_SRR_RTR_RRS_Pos);
            }
        }
        else
        {
            /* Extended message identifier */
            RegVal |= ((((TxMessage->ExtId >> 18) & 0x7FFUL) << CANFD_TBID_STD_ID_Pos) |
                       ((TxMessage->ExtId & 0x3FFFFUL) << CANFD_TBID_EXT_ID_Pos));

            RegVal |= (0x01UL << CANFD_TBID_SRR_RTR_RRS_Pos);

            if (TxMessage->EDL == CANFD_EDL_CAN)
            {
                RegVal |= ((uint32_t)TxMessage->RTR << CANFD_TBID_RTR_RRS_Pos);
            }
        }

        WRITE_REG(CANFDx->TB[TxBufferIndex].ID, RegVal);

        /* TB DLC */
        RegVal = (((uint32_t)TxMessage->DLC << CANFD_TBDLC_DLC_Pos) |
                  ((uint32_t)TxMessage->EDL << CANFD_TBDLC_EDL_FDF_Pos) |
                  ((uint32_t)TxMessage->EFC << CANFD_TBDLC_EFC_Pos) |
                  ((uint32_t)TxMessage->MM << CANFD_TBDLC_MM_Pos));

        if (TxMessage->EDL == CANFD_EDL_CANFD)
        {
            RegVal |= ((uint32_t)TxMessage->BRS << CANFD_TBDLC_BRS_Pos);
        }

        WRITE_REG(CANFDx->TB[TxBufferIndex].DLC, RegVal);
        
        /* TB DW */
        uint32_t Data = 0UL;
        uint8_t Datalen = CANFD_DLCtoLEN(TxMessage->DLC);
        for (uint8_t i = 0U; i < Datalen; i++)
        {
            uint8_t WordIdx = i / 4U;
            uint8_t ByteIdx = i % 4U;

            if (ByteIdx == 0U)
                Data = 0UL;

            Data |= ((uint32_t)TxMessage->Data[i] << (8U * (3U - ByteIdx)));

            if ((ByteIdx == 3U) || (i == (Datalen - 1U)))
            {
                CANFDx->TB[TxBufferIndex].DW[WordIdx] = Data;
            }
        }

        /* Enable buffer ready request */
        SET_BIT(CANFDx->TRR, (1UL << TxBufferIndex));
    }
    else
    {
        /* There are no free buffers for transmission */
        TxBufferIndex = CANFD_TX_NO_BUFFER;
    }
  
    return TxBufferIndex;
}

/**
 * @brief Checks the message transmission status.
 * @param CANFDx The CAN instance.
 * @param TransmitBuffer The index of the transmit buffer.
 * @returns The transmission status. Can be one of CANFD_TxStatus_TypeDef values.
 */
CANFD_TxStatus_TypeDef CANFD_TransmitStatus(CANFD_TypeDef *CANFDx, uint8_t TransmitBuffer)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_TX_BUFFER(TransmitBuffer));

    CANFD_TxStatus_TypeDef status;

    uint32_t Mask = (1UL << TransmitBuffer);

    if (READ_BIT(CANFDx->TRR, Mask) == Mask)
    {
        status = CANFD_TX_STATUS_PENDING;
    }
    else
    {
        status = CANFD_TX_STATUS_OK;
    }

    return status;
}

/**
 * @brief Cancels pending transmission request.
 * @param CANFDx The CAN instance.
 * @param TransmitBuffer The index of the transmit buffer.
 */
void CANFD_CancelTransmit(CANFD_TypeDef *CANFDx, uint8_t TransmitBuffer)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_TX_BUFFER(TransmitBuffer));

    uint32_t Mask = (1UL << TransmitBuffer);

    if ((READ_BIT(CANFDx->TRR, Mask) == Mask) &&
        (READ_BIT(CANFDx->TCR, Mask) == 0UL))
    {
        SET_BIT(CANFDx->TCR, Mask);
    }
}

/**
 * @brief Returns Tx Event FIFO fill level.
 * @note Fill level is the number of stored messages in Tx Event FIFO
 *       starting from the read index.
 * @param CANFDx The CAN instance.
 * @returns The fill level.
 */
uint8_t CANFD_TXE_FIFO_GetFillLevel(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    return (uint8_t)(READ_BIT(CANFDx->TXE_FSR, CANFD_TXE_FSR_FL) >> CANFD_TXE_FSR_FL_Pos);
}

/**
 * @brief Returns Tx Event FIFO read index.
 * @param CANFDx The CAN instance.
 * @returns The read index.
 */
uint8_t CANFD_TXE_FIFO_GetReadIndex(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    return (uint8_t)(READ_BIT(CANFDx->TXE_FSR, CANFD_TXE_FSR_RI) >> CANFD_TXE_FSR_RI_Pos);
}

/**
 * @brief Increments Tx Event FIFO read index by 1.
 * @param CANFDx The CAN instance.
 */
void CANFD_TXE_FIFO_IncReadIndex(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    SET_BIT(CANFDx->TXE_FSR, CANFD_TXE_FSR_IRI);
}

/**
 * @brief Sets Tx Event FIFO full watermark level.
 * @note Can be set only when CAN is disabled.
 * @param CANFDx The CAN instance.
 * @param Level The watermark level. Must be in the range from 1 to 31.
 */
void CANFD_TXE_FIFO_SetWatermark(CANFD_TypeDef *CANFDx, uint8_t Level)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_TXE_FWM(Level));

    MODIFY_REG(CANFDx->TXE_WMR,
               CANFD_TXE_WMR_FWM,
               ((uint32_t)Level << CANFD_TXE_WMR_FWM_Pos) & CANFD_TXE_WMR_FWM);
}

/**
 * @brief Returns Tx Event FIFO full watermark level.
 * @param CANFDx The CAN instance.
 * @returns The watermark level.
 */
uint8_t CANFD_TXE_FIFO_GetWatermark(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    return (uint8_t)(READ_BIT(CANFDx->TXE_WMR, CANFD_TXE_WMR_FWM) >> CANFD_TXE_WMR_FWM_Pos);
}

/**
 * @brief Reads a Tx event from FIFO.
 * @param CANFDx The CAN instance.
 * @param TxEvent The pointer to CANFD_TxEvt_TypeDef structure.
 */
void CANFD_GetEvent(CANFD_TypeDef *CANFDx, CANFD_TxEvt_TypeDef *TxEvent)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    if (CANFD_TXE_FIFO_GetFillLevel(CANFDx) > 0U)
    {
        /* There are events in FIFO */

        /* Get the event index */
        uint8_t Index = CANFD_TXE_FIFO_GetReadIndex(CANFDx);

        /* Get event */

        /* Identifier Extension */
        TxEvent->IDE = (CANFD_IDE_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_IDE) >> CANFD_TXE_TBID_IDE_Pos);

        /* Identifier */
        if (TxEvent->IDE == CANFD_IDE_STD)
        {
            /* Standard message identifier */
            TxEvent->StdId = READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_STD_ID) >> CANFD_TXE_TBID_STD_ID_Pos;

            /* Remote Transmission Request */
            TxEvent->RTR = (CANFD_RTR_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_SRR_RTR_RRS) >> CANFD_TXE_TBID_SRR_RTR_RRS_Pos);
        }
        else
        {
            /* Extended message identifier */
            TxEvent->ExtId = (((READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_STD_ID) >> CANFD_TXE_TBID_STD_ID_Pos) << 18) |
                              (READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_EXT_ID) >> CANFD_TXE_TBID_EXT_ID_Pos));

            /* Remote Transmission Request */
            TxEvent->RTR = (CANFD_RTR_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].ID, CANFD_TXE_TBID_RTR_RRS) >> CANFD_TXE_TBID_RTR_RRS_Pos);
        }

        /* Extended Data Length */
        TxEvent->EDL = (CANFD_EDL_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_EDL_FDF) >> CANFD_TXE_TBDLC_EDL_FDF_Pos);

        /* Bit Rate Switch */
        TxEvent->BRS = (CANFD_BRS_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_BRS) >> CANFD_TXE_TBDLC_BRS_Pos);

        /* Data Length Code */
        TxEvent->DLC = (CANFD_DLC_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_DLC) >> CANFD_TXE_TBDLC_DLC_Pos);

        /* Event Type */
        TxEvent->ET = (CANFD_ET_TypeDef)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_ET) >> CANFD_TXE_TBDLC_ET_Pos);

        /* Message Marker */
        TxEvent->MM = (uint8_t)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_MM) >> CANFD_TXE_TBDLC_MM_Pos);

        /* Timestamp */
        TxEvent->TS = (uint16_t)(READ_BIT(CANFDx->TXE_TB[Index].DLC, CANFD_TXE_TBDLC_TS) >> CANFD_TXE_TBDLC_TS_Pos);

        /* Release FIFO */
        CANFD_TXE_FIFO_IncReadIndex(CANFDx);
    }
}

/**
 * @brief Returns Rx FIFO fill level.
 * @note Fill level is the number of stored messages in Rx FIFO starting
 *       from the read index.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 * @returns The fill level.
 */
uint8_t CANFD_RX_FIFO_GetFillLevel(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));

    uint8_t Level;

    if (FIFONumber == CANFD_RX_FIFO0)
    {
        /* FIFO-0 */
        Level = (uint8_t)(READ_BIT(CANFDx->FSR, CANFD_FSR_FL) >> CANFD_FSR_FL_Pos);
    }
    else
    {
        /* FIFO-1 */
        Level = (uint8_t)(READ_BIT(CANFDx->FSR, CANFD_FSR_FL_1) >> CANFD_FSR_FL_1_Pos);
    }

    return Level;
}

/**
 * @brief Returns Rx FIFO read index.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 * @returns The read index.
 */
uint8_t CANFD_RX_FIFO_GetReadIndex(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));

    uint8_t Index;

    if (FIFONumber == CANFD_RX_FIFO0)
    {
        /* FIFO-0 */
        Index = (uint8_t)(READ_BIT(CANFDx->FSR, CANFD_FSR_RI) >> CANFD_FSR_RI_Pos);
    }
    else
    {
        /* FIFO-1 */
        Index = (uint8_t)(READ_BIT(CANFDx->FSR, CANFD_FSR_RI_1) >> CANFD_FSR_RI_1_Pos);
    }

    return Index;
}

/**
 * @brief Increments Rx FIFO read index by 1.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 */
void CANFD_RX_FIFO_IncReadIndex(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));

    if (FIFONumber == CANFD_RX_FIFO0)
    {
        /* FIFO-0 */
        SET_BIT(CANFDx->FSR, CANFD_FSR_IRI);
    }
    else
    {
        /* FIFO-1 */
        SET_BIT(CANFDx->FSR, CANFD_FSR_IRI_1);
    }
}

/**
 * @brief Sets Rx FIFO full watermark level.
 * @note Can be set only when CAN is disabled.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 * @param Level The watermark level. Must be in the range from 1 to 63.
 */
void CANFD_RX_FIFO_SetWatermark(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber, uint8_t Level)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));
    assert(IS_CANFD_RX_FWM(Level));

    if (FIFONumber == CANFD_RX_FIFO0)
    {
        /* FIFO-0 */
        MODIFY_REG(CANFDx->WMR,
                   CANFD_WMR_RXFWM,
                   ((uint32_t)Level << CANFD_WMR_RXFWM_Pos) & CANFD_WMR_RXFWM);
    }
    else
    {
        /* FIFO-1 */
        MODIFY_REG(CANFDx->WMR,
                   CANFD_WMR_RXFWM_1,
                   ((uint32_t)Level << CANFD_WMR_RXFWM_1_Pos) & CANFD_WMR_RXFWM_1);
    }
}

/**
 * @brief Returns Rx FIFO full watermark level.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 * @returns The watermark level.
 */
uint8_t CANFD_RX_FIFO_GetWatermark(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));

    uint8_t Level;

    if (FIFONumber == CANFD_RX_FIFO0)
    {
        /* FIFO-0 */
        Level = (uint8_t)(READ_BIT(CANFDx->WMR, CANFD_WMR_RXFWM) >> CANFD_WMR_RXFWM_Pos);
    }
    else
    {
        /* FIFO-1 */
        Level = (uint8_t)(READ_BIT(CANFDx->WMR, CANFD_WMR_RXFWM_1) >> CANFD_WMR_RXFWM_1_Pos);
    }

    return Level;
}

/**
 * @brief Sets Rx filter partition index.
 * @note Received messages which match filter-mask pairs from 0 to Index
 *       are stored in Rx FIFO-0. Received messages which match filter-mask
 *       pairs (Index + 1) and above are stored in Rx FIFO-1.
 * @note Can be set only when CAN is disabled.
 * @param CANFDx The CAN instance.
 * @param Index The filter partition index. Must be in the range from 0x0 to 0x1F.
 */
void CANFD_SetFilterPartition(CANFD_TypeDef *CANFDx, uint8_t Index)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FP(Index));

    MODIFY_REG(CANFDx->WMR,
               CANFD_WMR_RXFP,
               ((uint32_t)Index << CANFD_WMR_RXFP_Pos) & CANFD_WMR_RXFP);
}

/**
 * @brief Returns Rx filter partition index.
 * @note Received messages which match filter-mask pairs from 0 to Index
 *       are stored in Rx FIFO-0. Received messages which match filter-mask
 *       pairs (Index + 1) and above are stored in Rx FIFO-1.
 * @param CANFDx The CAN instance.
 * @returns The filter partition index.
 */
uint8_t CANFD_GetFilterPartition(CANFD_TypeDef *CANFDx)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));

    return (uint8_t)(READ_BIT(CANFDx->WMR, CANFD_WMR_RXFP) >> CANFD_WMR_RXFP_Pos);
}

/**
 * @brief Receives a message.
 * @param CANFDx The CAN instance.
 * @param FIFONumber The Rx FIFO number.
 * @param RxMessage The pointer to CANFD_RxMsg_TypeDef structure.
 */
void CANFD_Receive(CANFD_TypeDef *CANFDx, CANFD_RxFIFO_TypeDef FIFONumber, CANFD_RxMsg_TypeDef *RxMessage)
{
    assert(IS_CANFD_ALL_INSTANCE(CANFDx));
    assert(IS_CANFD_RX_FIFO(FIFONumber));

    if (CANFD_RX_FIFO_GetFillLevel(CANFDx, FIFONumber) > 0U)
    {
        /* There are messages in FIFO */

        /* Get the message index */
        uint8_t Index = CANFD_RX_FIFO_GetReadIndex(CANFDx, FIFONumber);

        /* Get message */
        CANFD_RB_TypeDef *RxFIFO;

        if (FIFONumber == CANFD_RX_FIFO0)
        {
            /* FIFO-0 */
            RxFIFO = CANFDx->RB_FIFO0;
        }
        else
        {
            /* FIFO-1 */
            RxFIFO = CANFDx->RB_FIFO1;
        }

        /* Identifier Extension */
        RxMessage->IDE = (CANFD_IDE_TypeDef)(READ_BIT(RxFIFO[Index].ID, CANFD_RBID_IDE) >> CANFD_RBID_IDE_Pos);

        /* Identifier */
        if (RxMessage->IDE == CANFD_IDE_STD)
        {
            /* Standard message identifier */
            RxMessage->StdId = READ_BIT(RxFIFO[Index].ID, CANFD_RBID_STD_ID) >> CANFD_RBID_STD_ID_Pos;

            /* Remote Transmission Request */
            RxMessage->RTR = (CANFD_RTR_TypeDef)(READ_BIT(RxFIFO[Index].ID, CANFD_RBID_SRR_RTR_RRS) >> CANFD_RBID_SRR_RTR_RRS_Pos);
        }
        else
        {
            /* Extended message identifier */
            RxMessage->ExtId = (((READ_BIT(RxFIFO[Index].ID, CANFD_RBID_STD_ID) >> CANFD_RBID_STD_ID_Pos) << 18) |
                                (READ_BIT(RxFIFO[Index].ID, CANFD_RBID_EXT_ID) >> CANFD_RBID_EXT_ID_Pos));

            /* Remote Transmission Request */
            RxMessage->RTR = (CANFD_RTR_TypeDef)(READ_BIT(RxFIFO[Index].ID, CANFD_RBID_RTR_RRS) >> CANFD_RBID_RTR_RRS_Pos);
        }

        /* Extended Data Length */
        RxMessage->EDL = (CANFD_EDL_TypeDef)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_EDL_FDF) >> CANFD_RBDLC_EDL_FDF_Pos);

        /* Bit Rate Switch */
        RxMessage->BRS = (CANFD_BRS_TypeDef)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_BRS) >> CANFD_RBDLC_BRS_Pos);

        /* Data Length Code */
        RxMessage->DLC = (CANFD_DLC_TypeDef)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_DLC) >> CANFD_RBDLC_DLC_Pos);

        /* Error State Indicator */
        RxMessage->ESI = (CANFD_ESI_TypeDef)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_ESI) >> CANFD_RBDLC_ESI_Pos);

        /* Matched Filter Index */
        RxMessage->MFI = (uint8_t)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_MFI) >> CANFD_RBDLC_MFI_Pos);

        /* Timestamp */
        RxMessage->TS = (uint16_t)(READ_BIT(RxFIFO[Index].DLC, CANFD_RBDLC_TS) >> CANFD_RBDLC_TS_Pos);

        /* Data */
        uint8_t Datalen = CANFD_DLCtoLEN(RxMessage->DLC);
        for (uint8_t i = 0U; i < Datalen; i++)
        {
            uint8_t WordIdx = i / 4U;
            uint8_t ByteIdx = i % 4U;

            RxMessage->Data[i] = (uint8_t)((READ_REG(RxFIFO[Index].DW[WordIdx]) >> (8U * (3U - ByteIdx))) & 0xFFUL);
        }

        /* Release FIFO */
        CANFD_RX_FIFO_IncReadIndex(CANFDx, FIFONumber);
    }
}

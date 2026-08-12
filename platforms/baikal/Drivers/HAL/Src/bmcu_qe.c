/**
 * *****************************************************************************
 *  @file       bmcu_qe.c
 *  @author     Baikal electronics SDK team
 *  @brief      QE module driver
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

#include "bmcu_qe.h"
#include "bmcu_def.h"

#define IS_QE_COUNTMODE(__VALUE__)              (((__VALUE__) == QE_COUNTMODE_QUADRATURE) || \
                                                 ((__VALUE__) == QE_COUNTMODE_EDGE))

#define IS_QE_TURNMODE(__VALUE__)               (((__VALUE__) == QE_TURNMODE_MULTITURN) || \
                                                 ((__VALUE__) == QE_TURNMODE_SINGLETURN))

#define IS_QE_INITMODE(__VALUE__)               (((__VALUE__) == QE_INITMODE_RESET) || \
                                                 ((__VALUE__) == QE_INITMODE_INITSET))

#define IS_QE_POLARITY(__VALUE__)               (((__VALUE__) == QE_POLARITY_NONINVERTED) || \
                                                 ((__VALUE__) == QE_POLARITY_INVERTED))

#define IS_QE_FILTER(__VALUE__)                 (((__VALUE__) == QE_FILTER_FDIV1) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV1_N2) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV1_N4) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV1_N8) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV2_N6) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV2_N8) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV4_N6) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV4_N8) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV8_N6) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV8_N8) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV16_N5) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV16_N6) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV16_N8) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV32_N5) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV32_N6) || \
                                                 ((__VALUE__) == QE_FILTER_FDIV32_N8))

/**
 * @brief Deinitializes a QE instance.
 * @param QEx The QE instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The QE instance have been deinitialized.
 *         - ERROR: The QE instance have not been deinitialized.
 */
ErrorStatus QE_DeInit(QE_TypeDef *QEx)
{
    ErrorStatus status = ERROR;
  
    /* Check parameters */
    assert(IS_QE_ALL_INSTANCE(QEx));
  
    /* Disable the encoder instance if enabled */
    if (QE_IsEnabled(QEx) != 0UL)
    {
        QE_Disable(QEx);
    }
  
    /* Reset registers */
    if (QE_IsEnabled(QEx) == 0UL)
    {
        CLEAR_BIT(QEx->QESET, (QE_QESET_INIE | QE_QESET_OFIE | QE_QESET_INIT_MODE |
                               QE_QESET_SINGLE_MODE | QE_QESET_INDXPOL | QE_QESET_QBPOL |
                               QE_QESET_QAPOL | QE_QESET_QBF | QE_QESET_QAF |
                               QE_QESET_INDXF | QE_QESET_QESWAP | QE_QESET_QEMODE));

        MODIFY_REG(QEx->FILTSET,
                   (QE_FILTSET_INDXSF | QE_FILTSET_QBSF | QE_FILTSET_QASF),
                   (QE_FILTSET_INDXSF_FDIV1 | QE_FILTSET_QBSF_FDIV1 | QE_FILTSET_QASF_FDIV1));

        WRITE_REG(QEx->QEMAX, 0xFFFFU);
        WRITE_REG(QEx->INITSET, 0xFFFFU);
        WRITE_REG(QEx->QECNT, 0x0000U);

        status = SUCCESS;
    }
  
    return status;
}

/**
 * @brief Initializes a QE instance.
 * @note QE_InitStruct structure should be initialized prior to calling this function.
 * @param QEx The QE instance.
 * @param QE_InitStruct The pointer to QE_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The QE instance have been initialized.
 *         - ERROR: The QE instance have not been initialized.
 */
ErrorStatus QE_Init(QE_TypeDef *QEx, QE_InitStruct_TypeDef *QE_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert(IS_QE_ALL_INSTANCE(QEx));
    assert(IS_QE_COUNTMODE(QE_InitStruct->CountMode));
    assert(IS_QE_TURNMODE(QE_InitStruct->TurnMode));
    assert(IS_QE_INITMODE(QE_InitStruct->InitMode));
    assert(IS_QE_POLARITY(QE_InitStruct->QAPolarity));
    assert(IS_QE_FILTER(QE_InitStruct->QAFilter));
    assert(IS_FUNCTIONAL_STATE(QE_InitStruct->QAFilterCtrl));
    assert(IS_QE_POLARITY(QE_InitStruct->QBPolarity));
    assert(IS_QE_FILTER(QE_InitStruct->QBFilter));
    assert(IS_FUNCTIONAL_STATE(QE_InitStruct->QBFilterCtrl));
    assert(IS_QE_POLARITY(QE_InitStruct->INDXPolarity));
    assert(IS_QE_FILTER(QE_InitStruct->INDXFilter));
    assert(IS_FUNCTIONAL_STATE(QE_InitStruct->INDXFilterCtrl));
    assert(IS_FUNCTIONAL_STATE(QE_InitStruct->InputSwap));

    if (QE_IsEnabled(QEx) == 0UL)
    {
        /* The counter is disabled */

        /* Configure counter */
        MODIFY_REG(QEx->QESET,
                   (QE_QESET_INIE | QE_QESET_OFIE | QE_QESET_INIT_MODE |
                    QE_QESET_SINGLE_MODE | QE_QESET_INDXPOL | QE_QESET_QBPOL |
                    QE_QESET_QAPOL | QE_QESET_QBF | QE_QESET_QAF |
                    QE_QESET_INDXF | QE_QESET_QESWAP | QE_QESET_QEMODE),
                   (((uint32_t)QE_InitStruct->InitMode) |
                    ((uint32_t)QE_InitStruct->TurnMode) |
                    ((uint32_t)QE_InitStruct->INDXPolarity >> QE_QESET_INDXPOL_Pos) |
                    ((uint32_t)QE_InitStruct->QBPolarity >> QE_QESET_QBPOL_Pos) |
                    ((uint32_t)QE_InitStruct->QAPolarity >> QE_QESET_QAPOL_Pos) |
                    ((uint32_t)QE_InitStruct->QBFilterCtrl >> QE_QESET_QBF_Pos) |
                    ((uint32_t)QE_InitStruct->QAFilterCtrl >> QE_QESET_QAF_Pos) |
                    ((uint32_t)QE_InitStruct->INDXFilterCtrl >> QE_QESET_INDXF_Pos) |
                    ((uint32_t)QE_InitStruct->InputSwap >> QE_QESET_QESWAP_Pos) |
                    ((uint32_t)QE_InitStruct->CountMode)));

        MODIFY_REG(QEx->FILTSET,
                   (QE_FILTSET_INDXSF | QE_FILTSET_QBSF | QE_FILTSET_QASF),
                   (((uint32_t)QE_InitStruct->INDXFilter << QE_FILTSET_INDXSF_Pos) | 
                    ((uint32_t)QE_InitStruct->QBFilter << QE_FILTSET_QASF_Pos) |
                    ((uint32_t)QE_InitStruct->QAFilter << QE_FILTSET_QBSF_Pos)));

        WRITE_REG(QEx->QEMAX, QE_InitStruct->MaxCount);
        WRITE_REG(QEx->INITSET, QE_InitStruct->InitCount);
        WRITE_REG(QEx->QECNT, 0x0000U);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of QE_InitStruct_TypeDef structure to default value.
 * @param QE_InitStruct The pointer to QE_InitStruct_TypeDef structure.
 */
void QE_StructInit(QE_InitStruct_TypeDef *QE_InitStruct)
{
    /* Set QE_InitStruct fields to default values */
    QE_InitStruct->CountMode = QE_COUNTMODE_QUADRATURE;
    QE_InitStruct->TurnMode = QE_TURNMODE_MULTITURN;
    QE_InitStruct->InitMode = QE_INITMODE_RESET;
    QE_InitStruct->InitCount = 0xFFFFU;
    QE_InitStruct->MaxCount = 0xFFFFU;
    QE_InitStruct->QAPolarity = QE_POLARITY_NONINVERTED;
    QE_InitStruct->QAFilter = QE_FILTER_FDIV1;
    QE_InitStruct->QAFilterCtrl = DISABLE;
    QE_InitStruct->QBPolarity = QE_POLARITY_NONINVERTED;
    QE_InitStruct->QBFilter = QE_FILTER_FDIV1;
    QE_InitStruct->QBFilterCtrl = DISABLE;
    QE_InitStruct->INDXPolarity = QE_POLARITY_NONINVERTED;
    QE_InitStruct->INDXFilter = QE_FILTER_FDIV1;
    QE_InitStruct->INDXFilterCtrl = DISABLE;
    QE_InitStruct->InputSwap = DISABLE;
}

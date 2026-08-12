/**
 * *****************************************************************************
 *  @file       bmcu_qe.h
 *  @author     Baikal electronics SDK team
 *  @brief      QE module driver header file
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

#ifndef __BMCU_QE_H
#define __BMCU_QE_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encoder counting mode */
typedef enum {
    QE_COUNTMODE_QUADRATURE = QE_QESET_QEMODE_QUADRATURE,   /*!< Counter counts up/down on both QA and QB edges depending on the level of the other input */
    QE_COUNTMODE_EDGE       = QE_QESET_QEMODE_EDGE          /*!< Counter counts up/down on QA edge depending on QB level */
} QE_CountMode_TypeDef;

/** @brief Encoder turn mode */
typedef enum {
    QE_TURNMODE_MULTITURN  = QE_QESET_SINGLE_MODE_DISABLE,  /*!< Multiturn mode */
    QE_TURNMODE_SINGLETURN = QE_QESET_SINGLE_MODE_ENABLE    /*!< Single turn mode */
} QE_TurnMode_TypeDef;

/** @brief Encoder init mode */
typedef enum {
    QE_INITMODE_RESET   = QE_QESET_INIT_MODE_RESET,     /*!< INDX resets the counter */
    QE_INITMODE_INITSET = QE_QESET_INIT_MODE_INITSET    /*!< INDX sets the counter to INITSET value */
} QE_InitMode_TypeDef;

/** @brief Input polarity */
typedef enum {
    QE_POLARITY_NONINVERTED = 0,    /*!< Input is not inverted */
    QE_POLARITY_INVERTED    = 1     /*!< Input is inverted */
} QE_Polarity_TypeDef;

/** @brief Input filter */
typedef enum {
    QE_FILTER_FDIV1     = QE_FILTSET_QASF_FDIV1,        /*!< No filter, sampling is done at fDTS */
    QE_FILTER_FDIV1_N2  = QE_FILTSET_QASF_FDIV1_N2,     /*!< fSAMPLING = fCK_INT, N = 2 */ 
    QE_FILTER_FDIV1_N4  = QE_FILTSET_QASF_FDIV1_N4,     /*!< fSAMPLING = fCK_INT, N = 4 */
    QE_FILTER_FDIV1_N8  = QE_FILTSET_QASF_FDIV1_N8,     /*!< fSAMPLING = fCK_INT, N = 8 */
    QE_FILTER_FDIV2_N6  = QE_FILTSET_QASF_FDIV2_N6,     /*!< fSAMPLING = fDTS/2, N = 6 */
    QE_FILTER_FDIV2_N8  = QE_FILTSET_QASF_FDIV2_N8,     /*!< fSAMPLING = fDTS/2, N = 8 */
    QE_FILTER_FDIV4_N6  = QE_FILTSET_QASF_FDIV4_N6,     /*!< fSAMPLING = fDTS/4, N = 6 */
    QE_FILTER_FDIV4_N8  = QE_FILTSET_QASF_FDIV4_N8,     /*!< fSAMPLING = fDTS/4, N = 8 */
    QE_FILTER_FDIV8_N6  = QE_FILTSET_QASF_FDIV8_N6,     /*!< fSAMPLING = fDTS/8, N = 6 */
    QE_FILTER_FDIV8_N8  = QE_FILTSET_QASF_FDIV8_N8,     /*!< fSAMPLING = fDTS/8, N = 8 */
    QE_FILTER_FDIV16_N5 = QE_FILTSET_QASF_FDIV16_N5,    /*!< fSAMPLING = fDTS/16, N = 5 */
    QE_FILTER_FDIV16_N6 = QE_FILTSET_QASF_FDIV16_N6,    /*!< fSAMPLING = fDTS/16, N = 6 */
    QE_FILTER_FDIV16_N8 = QE_FILTSET_QASF_FDIV16_N8,    /*!< fSAMPLING = fDTS/16, N = 8 */
    QE_FILTER_FDIV32_N5 = QE_FILTSET_QASF_FDIV32_N5,    /*!< fSAMPLING = fDTS/32, N = 5 */
    QE_FILTER_FDIV32_N6 = QE_FILTSET_QASF_FDIV32_N6,    /*!< fSAMPLING = fDTS/32, N = 6 */
    QE_FILTER_FDIV32_N8 = QE_FILTSET_QASF_FDIV32_N8     /*!< fSAMPLING = fDTS/32, N = 8 */
} QE_Filter_TypeDef;

/** @brief Interrupts */
typedef enum {
    QE_IT_OFIE = 11,    /*!< Overflow interrupt */
    QE_IT_INIE = 12     /*!< INDX interrupt */
} QE_IT_TypeDef;

/** @brief QE Init Structure definition */
typedef struct
{
    QE_CountMode_TypeDef CountMode;     /*!< Counting mode */
    QE_TurnMode_TypeDef TurnMode;       /*!< Turn mode */
    QE_InitMode_TypeDef InitMode;       /*!< Initialization mode */
    uint16_t InitCount;                 /*!< Initialization value */
    uint16_t MaxCount;                  /*!< Maximum value */
    QE_Polarity_TypeDef QAPolarity;     /*!< QA input polarity */
    QE_Filter_TypeDef QAFilter;         /*!< QA input filter configuration */
    FunctionalState QAFilterCtrl;       /*!< QA input filter enable */
    QE_Polarity_TypeDef QBPolarity;     /*!< QB input polarity */
    QE_Filter_TypeDef QBFilter;         /*!< QB input filter configuration */
    FunctionalState QBFilterCtrl;       /*!< QB input filter enable */
    QE_Polarity_TypeDef INDXPolarity;   /*!< INDX input polarity */
    QE_Filter_TypeDef INDXFilter;       /*!< INDX input filter configuration */
    FunctionalState INDXFilterCtrl;     /*!< INDX input filter enable */
    FunctionalState InputSwap;          /*!< QA and QB swap */
} QE_InitStruct_TypeDef;

/**
 * @brief Writes a value in QE register.
 * @param INSTANCE The QE instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define QE_WriteReg(INSTANCE, REG, VALUE)       WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in QE register.
 * @param INSTANCE The QE instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define QE_ReadReg(INSTANCE, REG)               READ_REG(INSTANCE->REG)

/**
 * @brief Enables the encoder.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_Enable(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_QEN);
}

/**
 * @brief Disables the encoder.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_Disable(QE_TypeDef *QEx)
{
    CLEAR_BIT(QEx->QESET, QE_QESET_QEN);
}

/**
 * @brief Checks if the encoder is enabled.
 * @param QEx The QE instance.
 * @retval 1 if the encoder is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_IsEnabled(QE_TypeDef *QEx)
{
    return ((READ_BIT(QEx->QESET, QE_QESET_QEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables QA and QB inputs swap.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_EnableInputSwap(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_QESWAP);
}

/**
 * @brief Disables QA and QB inputs swap.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_DisableInputSwap(QE_TypeDef *QEx)
{
    CLEAR_BIT(QEx->QESET, QE_QESET_QESWAP);
}

/**
 * @brief Checks if QA and QB inputs swap is enabled.
 * @param QEx The QE instance.
 * @retval 1 if swap is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_IsEnabledInputSwap(QE_TypeDef *QEx)
{
    return ((READ_BIT(QEx->QESET, QE_QESET_QESWAP) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets the encoder counting mode.
 * @param QEx The QE instance.
 * @param CountMode The counting mode. Can be one of QE_CountMode_TypeDef values.
 */
__STATIC_INLINE void QE_SetCountMode(QE_TypeDef *QEx, QE_CountMode_TypeDef CountMode)
{
    MODIFY_REG(QEx->QESET, QE_QESET_QEMODE, (uint32_t)CountMode);
}

/**
 * @brief Returns the encoder counting mode.
 * @param QEx The QE instance.
 * @returns The counting mode. Can be one of QE_CountMode_TypeDef values.
 */
__STATIC_INLINE QE_CountMode_TypeDef QE_GetCountMode(QE_TypeDef *QEx)
{
    return (QE_CountMode_TypeDef)READ_BIT(QEx->QESET, QE_QESET_QEMODE);
}

/**
 * @brief Sets the encoder turn mode.
 * @param QEx The QE instance.
 * @param TurnMode The turn mode. Can be one of QE_TurnMode_TypeDef values.
 */
__STATIC_INLINE void QE_SetTurnMode(QE_TypeDef *QEx, QE_TurnMode_TypeDef TurnMode)
{
    MODIFY_REG(QEx->QESET, QE_QESET_SINGLE_MODE, (uint32_t)TurnMode);
}

/**
 * @brief Returns the encoder turn mode.
 * @param QEx The QE instance.
 * @returns The turn mode. Can be one of QE_TurnMode_TypeDef values.
 */
__STATIC_INLINE QE_TurnMode_TypeDef QE_GetTurnMode(QE_TypeDef *QEx)
{
    return (QE_TurnMode_TypeDef)READ_BIT(QEx->QESET, QE_QESET_SINGLE_MODE);
}

/**
 * @brief Sets the encoder initialization mode.
 * @param QEx The QE instance.
 * @param InitMode The initialization mode. Can be one of QE_InitMode_TypeDef values.
 */
__STATIC_INLINE void QE_SetInitMode(QE_TypeDef *QEx, QE_InitMode_TypeDef InitMode)
{
    MODIFY_REG(QEx->QESET, QE_QESET_INIT_MODE, (uint32_t)InitMode);
}

/**
 * @brief Returns the encoder initialization mode.
 * @param QEx The QE instance.
 * @returns The initialization mode. Can be one of QE_InitMode_TypeDef values.
 */
__STATIC_INLINE QE_InitMode_TypeDef QE_GetInitMode(QE_TypeDef *QEx)
{
    return (QE_InitMode_TypeDef)READ_BIT(QEx->QESET, QE_QESET_INIT_MODE);
}

/**
 * @brief Sets QA input polarity.
 * @param QEx The QE instance.
 * @param Polarity The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE void QE_QA_SetPolarity(QE_TypeDef *QEx, QE_Polarity_TypeDef Polarity)
{
    MODIFY_REG(QEx->QESET, QE_QESET_QAPOL, (uint32_t)Polarity << QE_QESET_QAPOL_Pos);
}

/**
 * @brief Returns QA input polarity.
 * @param QEx The QE instance.
 * @returns The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE QE_Polarity_TypeDef QE_QA_GetPolarity(QE_TypeDef *QEx)
{
    return (QE_Polarity_TypeDef)(READ_BIT(QEx->QESET, QE_QESET_QAPOL) >> QE_QESET_QAPOL_Pos);
}

/**
 * @brief Sets QB input polarity.
 * @param QEx The QE instance.
 * @param Polarity The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE void QE_QB_SetPolarity(QE_TypeDef *QEx, QE_Polarity_TypeDef Polarity)
{
    MODIFY_REG(QEx->QESET, QE_QESET_QBPOL, (uint32_t)Polarity << QE_QESET_QBPOL_Pos);
}

/**
 * @brief Returns QB input polarity.
 * @param QEx The QE instance.
 * @returns The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE QE_Polarity_TypeDef QE_QB_GetPolarity(QE_TypeDef *QEx)
{
    return (QE_Polarity_TypeDef)(READ_BIT(QEx->QESET, QE_QESET_QBPOL) >> QE_QESET_QBPOL_Pos);
}

/**
 * @brief Sets INDX input polarity.
 * @param QEx The QE instance.
 * @param Polarity The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE void QE_INDX_SetPolarity(QE_TypeDef *QEx, QE_Polarity_TypeDef Polarity)
{
    MODIFY_REG(QEx->QESET, QE_QESET_INDXPOL, (uint32_t)Polarity << QE_QESET_INDXPOL_Pos);
}

/**
 * @brief Returns INDX input polarity.
 * @param QEx The QE instance.
 * @returns The polarity. Can be one of QE_Polarity_TypeDef values.
 */
__STATIC_INLINE QE_Polarity_TypeDef QE_INDX_GetPolarity(QE_TypeDef *QEx)
{
    return (QE_Polarity_TypeDef)(READ_BIT(QEx->QESET, QE_QESET_INDXPOL) >> QE_QESET_INDXPOL_Pos);
}

/**
 * @brief Enables QA input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_QA_EnableFilter(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_QAF);
}

/**
 * @brief Disables QA input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_QA_DisableFilter(QE_TypeDef *QEx)
{
    CLEAR_BIT(QEx->QESET, QE_QESET_QAF);
}

/**
 * @brief Checks if QA input filter is enabled.
 * @param QEx The QE instance.
 * @retval 1 if the filter is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_QA_IsEnabledFilter(QE_TypeDef *QEx)
{
    return ((READ_BIT(QEx->QESET, QE_QESET_QAF) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables QB input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_QB_EnableFilter(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_QBF);
}

/**
 * @brief Disables QB input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_QB_DisableFilter(QE_TypeDef *QEx)
{
    CLEAR_BIT(QEx->QESET, QE_QESET_QBF);
}

/**
 * @brief Checks if QB input filter is enabled.
 * @param QEx The QE instance.
 * @retval 1 if the filter is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_QB_IsEnabledFilter(QE_TypeDef *QEx)
{
    return ((READ_BIT(QEx->QESET, QE_QESET_QBF) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables INDX input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_INDX_EnableFilter(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_INDXF);
}

/**
 * @brief Disables INDX input filter.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_INDX_DisableFilter(QE_TypeDef *QEx)
{
    CLEAR_BIT(QEx->QESET, QE_QESET_INDXF);
}

/**
 * @brief Checks if INDX input filter is enabled.
 * @param QEx The QE instance.
 * @retval 1 if the filter is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_INDX_IsEnabledFilter(QE_TypeDef *QEx)
{
    return ((READ_BIT(QEx->QESET, QE_QESET_INDXF) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures QA input filter.
 * @param QEx The QE instance.
 * @param Filter The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE void QE_QA_SetFilter(QE_TypeDef *QEx, QE_Filter_TypeDef Filter)
{
    MODIFY_REG(QEx->FILTSET, QE_FILTSET_QASF, (uint32_t)Filter << QE_FILTSET_QASF_Pos);
}

/**
 * @brief Returns QA input filter configuration.
 * @param QEx The QE instance.
 * @returns The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE QE_Filter_TypeDef QE_QA_GetFilter(QE_TypeDef *QEx)
{
    return (QE_Filter_TypeDef)(READ_BIT(QEx->FILTSET, QE_FILTSET_QASF) >> QE_FILTSET_QASF_Pos);
}

/**
 * @brief Configures QB input filter.
 * @param QEx The QE instance.
 * @param Filter The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE void QE_QB_SetFilter(QE_TypeDef *QEx, QE_Filter_TypeDef Filter)
{
    MODIFY_REG(QEx->FILTSET, QE_FILTSET_QBSF, (uint32_t)Filter << QE_FILTSET_QBSF_Pos);
}

/**
 * @brief Returns QB input filter configuration.
 * @param QEx The QE instance.
 * @returns The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE QE_Filter_TypeDef QE_QB_GetFilter(QE_TypeDef *QEx)
{
    return (QE_Filter_TypeDef)(READ_BIT(QEx->FILTSET, QE_FILTSET_QBSF) >> QE_FILTSET_QBSF_Pos);
}

/**
 * @brief Configures INDX input filter.
 * @param QEx The QE instance.
 * @param Filter The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE void QE_INDX_SetFilter(QE_TypeDef *QEx, QE_Filter_TypeDef Filter)
{
    MODIFY_REG(QEx->FILTSET, QE_FILTSET_INDXSF, (uint32_t)Filter << QE_FILTSET_INDXSF_Pos);
}

/**
 * @brief Returns INDX input filter configuration.
 * @param QEx The QE instance.
 * @returns The filter configuration. Can be one of QE_Filter_TypeDef values.
 */
__STATIC_INLINE QE_Filter_TypeDef QE_INDX_GetFilter(QE_TypeDef *QEx)
{
    return (QE_Filter_TypeDef)(READ_BIT(QEx->FILTSET, QE_FILTSET_INDXSF) >> QE_FILTSET_INDXSF_Pos);
}

/**
 * @brief Sets the counter value.
 * @param QEx The QE instance.
 * @param Counter The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void QE_SetCounter(QE_TypeDef *QEx, uint16_t Counter)
{
    WRITE_REG(QEx->QECNT, Counter & QE_QECNT);
}

/**
 * @brief Returns the counter value.
 * @param QEx The QE instance.
 * @returns The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t QE_GetCounter(QE_TypeDef *QEx)
{
    return (uint16_t)(READ_REG(QEx->QECNT) & QE_QECNT);
}

/**
 * @brief Sets the counter initialization value.
 * @note The counter is initialized with this value when the initialization mode
 *       is QE_INITMODE_INITSET and the INDX signal positive edge is detected.
 * @param QEx The QE instance.
 * @param Count The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void QE_SetInitCount(QE_TypeDef *QEx, uint16_t Count)
{
    WRITE_REG(QEx->INITSET, Count & QE_INITSET);
}

/**
 * @brief Returns the counter initialization value.
 * @param QEx The QE instance.
 * @returns The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t QE_GetInitCount(QE_TypeDef *QEx)
{
    return (uint16_t)(READ_REG(QEx->INITSET) & QE_INITSET);
}

/**
 * @brief Sets the maximum counter value.
 * @param QEx The QE instance.
 * @param Count The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE void QE_SetMaxCount(QE_TypeDef *QEx, uint16_t Count)
{
    WRITE_REG(QEx->QEMAX, Count & QE_QEMAX);
}

/**
 * @brief Returns the maximum counter value.
 * @param QEx The QE instance.
 * @returns The counter value in the range of 0x0 to 0xFFFF.
 */
__STATIC_INLINE uint16_t QE_GetMaxCount(QE_TypeDef *QEx)
{
    return (uint16_t)(READ_REG(QEx->QEMAX) & QE_QEMAX);
}

/**
 * @brief Enables an interrupt.
 * @param QEx The QE instance.
 * @param IT The interrupt source. Can be one of QE_IT_TypeDef values.
 */
__STATIC_INLINE void QE_EnableIT(QE_TypeDef *QEx, QE_IT_TypeDef IT)
{
    SET_BIT(QEx->QESET, (1UL << IT));
}

/**
 * @brief Disables an interrupt.
 * @param QEx The QE instance.
 * @param IT The interrupt source. Can be one of QE_IT_TypeDef values.
 */
__STATIC_INLINE void QE_DisableIT(QE_TypeDef *QEx, QE_IT_TypeDef IT)
{
    CLEAR_BIT(QEx->QESET, (1UL << IT));
}

/**
 * @brief Checks if an interrupt is enabled.
 * @param QEx The QE instance.
 * @param IT The interrupt source. Can be one of QE_IT_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t QE_IsEnabledIT(QE_TypeDef *QEx, QE_IT_TypeDef IT)
{
    return ((READ_BIT(QEx->QESET, (1UL << IT)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears the pending interrupt.
 * @param QEx The QE instance.
 */
__STATIC_INLINE void QE_ClearIT(QE_TypeDef *QEx)
{
    SET_BIT(QEx->QESET, QE_QESET_INTRPT_CLR);
}

ErrorStatus QE_DeInit(QE_TypeDef *QEx);
ErrorStatus QE_Init(QE_TypeDef *QEx, QE_InitStruct_TypeDef *QE_InitStruct);
void QE_StructInit(QE_InitStruct_TypeDef *QE_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_QE_H */

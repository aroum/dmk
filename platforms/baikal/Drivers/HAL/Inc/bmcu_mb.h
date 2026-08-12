/**
 * *****************************************************************************
 *  @file       bmcu_mb.h
 *  @author     Baikal electronics SDK team
 *  @brief      Mailbox (MB) driver header file
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

#ifndef __BMCU_MB_H
#define __BMCU_MB_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (BE_U1000)

/**
 * @brief Writes a value in MB register.
 * @param INSTANCE The MB instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define MB_WriteReg(INSTANCE, REG, VALUE)       WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in MB register.
 * @param INSTANCE The MB instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define MB_ReadReg(INSTANCE, REG)                READ_REG(INSTANCE->REG)

/******************************************************************************/
/*                                CORE0, CORE1                                */
/******************************************************************************/

/**
 * @brief Writes data into DATA0 register.
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @param Data The data to write (32 bits).
 */
__STATIC_INLINE void C0_C1_MB_WriteData0(MB_TypeDef *MBx, uint32_t Data)
{
    WRITE_REG(MBx->DATA0, (uint32_t)(Data & MB_DATA0));
}

/**
 * @brief Reads data from DATA0 register.
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @returns The received data (32 bits).
 */
__STATIC_INLINE uint32_t C0_C1_MB_ReadData0(MB_TypeDef *MBx)
{
    return (uint32_t)READ_BIT(MBx->DATA0, MB_DATA0);
}

/**
 * @brief Sets FULL0 (DATA0 ready) flag.
 * @note This function can be called by CORE0 or CORE1.
 * @note The flag is set automatically on DATA0 register write.
 * @note The flag is cleared automatically on DATA0 register read from CORE2 side.
 * @param MBx The MB instance (System side).
 */
__STATIC_INLINE void C0_C1_MB_SetFlag_FULL0(MB_TypeDef *MBx)
{
    SET_BIT(MBx->FULL0, MB_FULL0);
}

/**
 * @brief Clears FULL0 (DATA0 ready) flag.
 * @note This function can be called by CORE0 or CORE1.
 * @note The flag is set automatically on DATA0 register write.
 * @note The flag is cleared automatically on DATA0 register read from CORE2 side.
 * @param MBx The MB instance (System side).
 */
__STATIC_INLINE void C0_C1_MB_ClearFlag_FULL0(MB_TypeDef *MBx)
{
    CLEAR_BIT(MBx->FULL0, MB_FULL0);
}

/**
 * @brief Checks if FULL0 (DATA0 ready) flag is set or not.
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C0_C1_MB_IsActiveFlag_FULL0(MB_TypeDef *MBx)
{
    return ((READ_BIT(MBx->FULL0, MB_FULL0) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if EMPTY0 (DATA0 empty) flag is set or not.
 * @note This function can be called by CORE0 or CORE1.
 * @note This flag is the inversion of FULL0 flag.
 * @param MBx The MB instance (System side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C0_C1_MB_IsActiveFlag_EMPTY0(MB_TypeDef *MBx)
{
    return ((READ_BIT(MBx->EMPTY0, MB_EMPTY0) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Reads data from DATA1 register.
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @returns The received data (32 bits).
 */
__STATIC_INLINE uint32_t C0_C1_MB_ReadData1(MB_TypeDef *MBx)
{
    return (uint32_t)READ_BIT(MBx->DATA1, MB_DATA1);
}

/**
 * @brief Checks if FULL1 (DATA1 ready) flag is set or not.
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C0_C1_MB_IsActiveFlag_FULL1(MB_TypeDef *MBx)
{
    return ((READ_BIT(MBx->FULL1, MB_FULL1) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if EMPTY1 (DATA1 empty) flag is set or not.
 * @note This function can be called by CORE0 or CORE1.
 * @note This flag is the inversion of FULL1 flag.
 * @param MBx The MB instance (System side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C0_C1_MB_IsActiveFlag_EMPTY1(MB_TypeDef *MBx)
{
    return ((READ_BIT(MBx->EMPTY1, MB_EMPTY1) != 0UL) ? 1UL : 0UL);
}

/******************************************************************************/
/*                                   CORE2                                    */
/******************************************************************************/

/**
 * @brief Reads data from DATA0 register.
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @returns The received data (32 bits).
 */
__STATIC_INLINE uint32_t C2_MB_ReadData0(MB_TypeDef *CORE2_MBx)
{
    return (uint32_t)READ_BIT(CORE2_MBx->DATA0, MB_DATA0);
}

/**
 * @brief Checks if FULL0 (DATA0 ready) flag is set or not.
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C2_MB_IsActiveFlag_FULL0(MB_TypeDef *CORE2_MBx)
{
    return ((READ_BIT(CORE2_MBx->FULL0, MB_FULL0) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if EMPTY0 (DATA0 empty) flag is set or not.
 * @note This function can be called only by CORE2.
 * @note This flag is the inversion of FULL0 flag.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C2_MB_IsActiveFlag_EMPTY0(MB_TypeDef *CORE2_MBx)
{
    return ((READ_BIT(CORE2_MBx->EMPTY0, MB_EMPTY0) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Writes data into DATA1 register.
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @param Data The data to write (32 bits).
 */
__STATIC_INLINE void C2_MB_WriteData1(MB_TypeDef *CORE2_MBx, uint32_t Data)
{
    WRITE_REG(CORE2_MBx->DATA1, (uint32_t)(Data & MB_DATA1));
}

/**
 * @brief Reads data from DATA1 register.
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @returns The received data (32 bits).
 */
__STATIC_INLINE uint32_t C2_MB_ReadData1(MB_TypeDef *CORE2_MBx)
{
    return (uint32_t)READ_BIT(CORE2_MBx->DATA1, MB_DATA1);
}

/**
 * @brief Sets FULL1 (DATA1 ready) flag.
 * @note This function can be called only by CORE2.
 * @note The flag is set automatically on DATA1 register write.
 * @note The flag is cleared automatically on DATA1 register read from system side.
 * @param CORE2_MBx The MB instance (CORE2 side).
 */
__STATIC_INLINE void C2_MB_SetFlag_FULL1(MB_TypeDef *CORE2_MBx)
{
    SET_BIT(CORE2_MBx->FULL1, MB_FULL1);
}

/**
 * @brief Clears FULL1 (DATA1 ready) flag.
 * @note This function can be called only by CORE2.
 * @note The flag is set automatically on DATA1 register write.
 * @note The flag is cleared automatically on DATA1 register read from system side.
 * @param CORE2_MBx The MB instance (CORE2 side).
 */
__STATIC_INLINE void C2_MB_ClearFlag_FULL1(MB_TypeDef *CORE2_MBx)
{
    CLEAR_BIT(CORE2_MBx->FULL1, MB_FULL1);
}

/**
 * @brief Checks if FULL1 (DATA1 ready) flag is set or not.
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C2_MB_IsActiveFlag_FULL1(MB_TypeDef *CORE2_MBx)
{
    return ((READ_BIT(CORE2_MBx->FULL1, MB_FULL1) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if EMPTY1 (DATA1 empty) flag is set or not.
 * @note This function can be called only by CORE2.
 * @note This flag is the inversion of FULL1 flag.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @retval 1 if the flag is set, otherwise 0.
 */
__STATIC_INLINE uint32_t C2_MB_IsActiveFlag_EMPTY1(MB_TypeDef *CORE2_MBx)
{
    return ((READ_BIT(CORE2_MBx->EMPTY1, MB_EMPTY1) != 0UL) ? 1UL : 0UL);
}

ErrorStatus C0_C1_MB_DeInit(MB_TypeDef *MBx);
ErrorStatus C2_MB_DeInit(MB_TypeDef *CORE2_MBx);

#ifdef __cplusplus
}
#endif

#endif

#endif /* __BMCU_MB_H */

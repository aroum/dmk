/**
 * *****************************************************************************
 *  @file       bmcu_mb.c
 *  @author     Baikal electronics SDK team
 *  @brief      Mailbox (MB) driver
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

#include "bmcu_mb.h"
#include "bmcu_def.h"

#if defined (BE_U1000)

/******************************************************************************/
/*                                CORE0, CORE1                                */
/******************************************************************************/

/**
 * @brief Deinitializes a MB instance (System side).
 * @note This function can be called by CORE0 or CORE1.
 * @param MBx The MB instance (System side).
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The MB instance have been deinitialized.
 *         - ERROR: The MB instance have not been deinitialized.
 */
ErrorStatus C0_C1_MB_DeInit(MB_TypeDef *MBx)
{
    /* Check parameters */
    assert(IS_MB_ALL_INSTANCE(MBx));
  
    WRITE_REG(MBx->DATA0, 0x0UL);
    CLEAR_BIT(MBx->FULL0, MB_FULL0);
  
    return SUCCESS;
}

/******************************************************************************/
/*                                   CORE2                                    */
/******************************************************************************/

/**
 * @brief Deinitializes a MB instance (CORE2 side).
 * @note This function can be called only by CORE2.
 * @param CORE2_MBx The MB instance (CORE2 side).
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The MB instance have been deinitialized.
 *         - ERROR: The MB instance have not been deinitialized.
 */
ErrorStatus C2_MB_DeInit(MB_TypeDef *CORE2_MBx)
{
    /* Check parameters */
    assert(IS_CORE2_MB_ALL_INSTANCE(CORE2_MBx));
  
    WRITE_REG(CORE2_MBx->DATA1, 0x0UL);
    CLEAR_BIT(CORE2_MBx->FULL1, MB_FULL1);
  
    return SUCCESS;
}

#endif

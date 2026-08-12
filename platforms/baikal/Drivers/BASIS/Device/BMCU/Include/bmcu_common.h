/**
 * *****************************************************************************
 *  @file       bmcu_common.h
 *  @author     Baikal electronics SDK team
 *  @brief      BASIS Baikal MCU common definitions
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

#ifndef BMCU_COMMON_H
#define BMCU_COMMON_H

#if defined (BMCU_U)
#include "bmcu_u.h"
#endif
#if defined (BE_U1000)
#include "be_u1000.h"
#endif

#if !defined (BMCU_U) && !defined (BE_U1000)
#error "Please select the target BMCU device."
#endif

#endif /* BMCU_COMMON_H */

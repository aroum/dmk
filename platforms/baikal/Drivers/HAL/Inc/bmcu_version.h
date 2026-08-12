/**
 * *****************************************************************************
 *  @file       bmcu_version.h
 *  @author     Baikal electronics SDK team
 *  @brief      SDK common definitions
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

#ifndef __BMCU_VERSION_H
#define __BMCU_VERSION_H

/* HAL version definitions */
#define __HAL_VERSION_MAJ        (2U)   /*!< [31:22] HAL major version */
#define __HAL_VERSION_MIN        (2U)   /*!< [21:12] HAL minor version */
#define __HAL_VERSION_PATCH      (0U) /*!< [11:0] HAL patch version */
#define __HAL_VERSION            ((__HAL_VERSION_MAJ << 22) | \
                                  (__HAL_VERSION_MIN << 12) | \
                                   __HAL_VERSION_PATCH)   /*!< HAL full version */

#endif /* __BMCU_VERSION_H */

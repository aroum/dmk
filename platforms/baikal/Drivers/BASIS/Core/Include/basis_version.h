/**
 * *****************************************************************************
 *  @file       basis_version.h
 *  @author     Baikal electronics SDK team
 *  @brief      BASIS component verion
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

#ifndef BASIS_VERSION_H
#define BASIS_VERSION_H

/* BASIS version definitions */
#define __BASIS_VERSION_MAJ        (2U)   /*!< [31:22] BASIS major version */
#define __BASIS_VERSION_MIN        (2U)   /*!< [21:12] BASIS minor version */
#define __BASIS_VERSION_PATCH      (0U) /*!< [11:0] BASIS patch version */
#define __BASIS_VERSION            ((__BASIS_VERSION_MAJ << 22) | \
                                    (__BASIS_VERSION_MIN << 12) | \
                                     __BASIS_VERSION_PATCH)   /*!< BASIS full version */

#endif /* BASIS_VERSION_H */

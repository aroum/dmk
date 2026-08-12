/**
 * *****************************************************************************
 *  @file       basis_compiler.h
 *  @author     Baikal electronics SDK team
 *  @brief      BASIS third-part compiler macros
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

#ifndef BASIS_COMPILER_H
#define BASIS_COMPILER_H

#include <stdint.h>

/* GNU Compiler */
#if defined (__GNUC__)

#include "basis_compiler_gcc.h"

#else
#error Unknown compiler.
#endif

#endif /* BASIS_COMPILER_H */

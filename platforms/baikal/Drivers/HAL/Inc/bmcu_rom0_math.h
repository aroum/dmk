/**
 * *****************************************************************************
 *  @file       bmcu_rom0_math.h
 *  @author     Baikal electronics SDK team
 *  @brief      ROM0 (math functions) header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * 
 *  File content:
 *      - typedef for built-in core math.h functions
 *      - setting adresses to names of functions
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_ROM0_MATH_H
#define __BMCU_ROM0_MATH_H

/* define functions, that would be used from rom lib 		 *
 * to prevent defining this names in original math.h library */
#define atanf	atanf_origin
#define asinf	asinf_origin
#define acosf	acosf_origin
#define atan2f	atan2f_origin
#define sinf	sinf_origin
#define cosf	cosf_origin
#define tanf	tanf_origin
#define expf	expf_origin
#define logf	logf_origin
#define log10f	log10f_origin
#define log2f	log2f_origin
#define cbrtf	cbrtf_origin
#define fmodf	fmodf_origin
#define nanf	nanf_origin
#define finitef	finitef_origin
#define scalbnf	scalbnf_origin
#define floorf	floorf_origin

/* undefining _MATH_H_ to include math.h there */
#undef _MATH_H_
#include <math.h>

/* undefining names back to use them for rom functions */
#undef atanf
#undef asinf
#undef acosf
#undef atan2f
#undef sinf
#undef cosf
#undef tanf
#undef expf
#undef logf
#undef log10f
#undef log2f
#undef cbrtf
#undef fmodf
#undef nanf
#undef finitef
#undef scalbnf
#undef floorf


typedef float (*math_atanf_addr)(float x);
typedef float (*math_asinf_addr)(float x);
typedef float (*math_acosf_addr)(float x);
typedef float (*math_atan2f_addr)(float x,float y);
typedef float (*math_sinf_addr)(float x);
typedef float (*math_cosf_addr)(float x);
typedef float (*math_tanf_addr)(float x);

typedef float (*math_expf_addr)(float x);

typedef float (*math_logf_addr)(float x);
typedef float (*math_log10f_addr)(float x);
typedef float (*math_log2f_addr)(float x);

typedef float (*math_cbrtf_addr)(float x);

typedef float (*math_fmodf_addr)(float x,float y);
typedef float (*math_nanf_addr)(const char * c);
typedef int (*math_finitef_addr)(float x);
typedef float (*math_scalbnf_addr)(float x, int i);
typedef float (*math_floorf_addr)(float x);


#if defined (BMCU_U)
	#define atanf(a) ((math_atanf_addr) (0x4000d404))(a)
	#define asinf(a) ((math_asinf_addr) (0x4000d8c4))(a)
	#define acosf(a) ((math_acosf_addr) (0x4000d5e8))(a)
	#define atan2f(a,b) ((math_atan2f_addr) (0x4000db5c))(a,b)
	#define sinf(a) ((math_sinf_addr) (0x4000dcd4))(a)
	#define cosf(a) ((math_cosf_addr) (0x4000e02e))(a)
	#define tanf(a) ((math_tanf_addr) (0x4000ddde))(a)

	#define expf(a) ((math_expf_addr) (0x4000e162))(a)
	#define logf(a) ((math_logf_addr) (0x4000e50e))(a)
	#define log10f(a) ((math_log10f_addr) (0x4000e750))(a)
	#define log2f(a) ((math_log2f_addr) (0x4000ea3e))(a)

	#define cbrtf(a) ((math_cbrtf_addr) (0x4000e96e))(a)

	#define fmodf(a,b) ((math_fmodf_addr) (0x4000e396))(a,b)

	#define nanf(a) ((math_nanf_addr) (0x4000f2d6))(a)
	#define finitef(a) ((math_finitef_addr) (0x4000f2e0))(a)
	#define scalbnf(a) ((math_scalbnf_addr) (0x4000f2ee))(a)
	#define floorf(a) ((math_floorf_addr) (0x4000f3c0))(a)
#endif

#if defined (BE_U1000)
	#define atanf(a) ((math_atanf_addr) (0x4000d604))(a)
	#define asinf(a) ((math_asinf_addr) (0x4000dac4))(a)
	#define acosf(a) ((math_acosf_addr) (0x4000d7e8))(a)
	#define atan2f(a,b) ((math_atan2f_addr) (0x4000dd5c))(a,b)
	#define sinf(a) ((math_sinf_addr) (0x4000ded4))(a)
	#define cosf(a) ((math_cosf_addr) (0x4000e22e))(a)
	#define tanf(a) ((math_tanf_addr) (0x4000dfde))(a)

	#define expf(a) ((math_expf_addr) (0x4000e362))(a)
	#define logf(a) ((math_logf_addr) (0x4000e70e))(a)
	#define log10f(a) ((math_log10f_addr) (0x4000e950))(a)
	#define log2f(a) ((math_log2f_addr) (0x4000ec3e))(a)

	#define cbrtf(a) ((math_cbrtf_addr) (0x4000eb6e))(a)

	#define fmodf(a,b) ((math_fmodf_addr) (0x4000e596))(a,b)

	#define nanf(a) ((math_nanf_addr) (0x4000f4d6))(a)
	#define finitef(a) ((math_finitef_addr) (0x4000f4e0))(a)
	#define scalbnf(a) ((math_scalbnf_addr) (0x4000f4ee))(a)
	#define floorf(a) ((math_floorf_addr) (0x4000f5c0))(a)
#endif

#endif //__BMCU_ROM0_MATH_H

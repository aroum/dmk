/**
 * *****************************************************************************
 *  @file       bmcu_rom1_dsp.h
 *  @author     Baikal electronics SDK team
 *  @brief      ROM1 (DSP functions) header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * 
 *  File content:
 *      - copy of bear_dsp_math_types.h to prevent dependence from dsp-lib code
 *      - typedef for built-in core math.h functions
 *      - setting adresses to names of functions
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *             Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *             Copyright (c) 2023 CloudBEAR LLC, all rights reserved.
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */
#ifndef __BMCU_ROM1_DSP_H
#define __BMCU_ROM1_DSP_H

    #include <limits.h>
    #include <stdint.h>

    /**
     * @brief 8-bit fractional data type in 1.7 format.
     */
    typedef int8_t q7_t;

    /**
     * @brief 16-bit fractional data type in 1.15 format.
     */
    typedef int16_t q15_t;

    /**
     * @brief 32-bit fractional data type in 1.31 format.
     */
    typedef int32_t q31_t;

    /**
     * @brief 64-bit fractional data type in 1.63 format.
     */
    typedef int64_t q63_t;

    /**
     * @brief 32-bit floating-point type definition.
     */
    typedef float float32_t;

    /**
     * @brief 64-bit floating-point type definition.
     */
    typedef double float64_t;

    #define F64_MAX ((float64_t)DBL_MAX)
    #define F32_MAX ((float32_t)FLT_MAX)

    #define F64_MIN (-DBL_MAX)
    #define F32_MIN (-FLT_MAX)

    #define F64_ABSMAX ((float64_t)DBL_MAX)
    #define F32_ABSMAX ((float32_t)FLT_MAX)

    #define F64_ABSMIN ((float64_t)0.0)
    #define F32_ABSMIN ((float32_t)0.0)

    #define Q31_MAX ((q31_t)(0x7FFFFFFFL))
    #define Q15_MAX ((q15_t)(0x7FFF))
    #define Q7_MAX ((q7_t)(0x7F))
    #define Q31_MIN ((q31_t)(0x80000000L))
    #define Q15_MIN ((q15_t)(0x8000))
    #define Q7_MIN ((q7_t)(0x80))

    #define Q31_ABSMAX ((q31_t)(0x7FFFFFFFL))
    #define Q15_ABSMAX ((q15_t)(0x7FFF))
    #define Q7_ABSMAX ((q7_t)(0x7F))
    #define Q31_ABSMIN ((q31_t)0)
    #define Q15_ABSMIN ((q15_t)0)
    #define Q7_ABSMIN ((q7_t)0)

    /* Dimension C vector space */
    #define CMPLX_DIM 2

    /**
     * @brief Error status returned by some functions in the library.
     */

    typedef enum {
        DSP_MATH_SUCCESS = 0,         /**< No error */
        DSP_MATH_ARGUMENT_ERROR = -1, /**< One or more arguments are incorrect */
        DSP_MATH_LENGTH_ERROR = -2,   /**< Length of data buffer is incorrect */
        DSP_MATH_SIZE_MISMATCH = -3,  /**< Size of matrices is not compatible with
                                            the operation */
        DSP_MATH_NANINF = -4,         /**< Not-a-number (NaN) or infinity is
                                            generated */
        DSP_MATH_SINGULAR = -5,       /**< Input matrix is singular and cannot be
                                            inverted */
        DSP_MATH_TEST_FAILURE = -6,   /**< Test Failed */
        DSP_MATH_DECOMPOSITION_FAILURE = -7 /**< Decomposition Failed */
    } dsp_status_t;

    /**
     * @brief Vector type of 8 signed 8-bit values.
     */
    typedef int64_t v8int8_t;

    /**
     * @brief Vector type of 4 signed 8-bit values.
     */
    typedef int32_t v4int8_t;

    /**
     * @brief Vector type of 4 signed 16-bit values.
     */
    typedef int64_t v4int16_t;

    /**
     * @brief Vector type of 2 signed 16-bit values.
     */
    typedef int32_t v2int16_t;

    /**
     * @brief Vector type of 2 signed 32-bit values.
     */
    typedef int64_t v2int32_t;

/**
 * @brief Computes the absolute value of a vector on an element-by-element basis.
 * 
 * pDst[n] = abs(pSrc[n]),   0 <= n < blockSize.
 * The functions support in-place computation allowing the source and destination pointers to reference the same memory buffer.
 * There are separate functions for Q7, Q15, and Q31 data types. 
 * 
 * Parameters
 * @param [in]    src    points to the input vector
 * @param [out]   dst    points to the output vector
 * @param [in]    block_size   number of samples in each vector 
 * @return        none
 */
typedef void (*dsp_abs_q7_addr)(const q7_t* src, q7_t* dst, uint32_t block_size); /*! <- The function uses saturating arithmetic. The Q7 value -1 (0x80) will be saturated to the maximum allowable positive value 0x7F. */
typedef void (*dsp_abs_q15_addr)(const q15_t* src, q15_t* dst, uint32_t block_size); /*! <- The function uses saturating arithmetic. The Q15 value -1 (0x8000) will be saturated to the maximum allowable positive value 0x7FFF. */
typedef void (*dsp_abs_q31_addr)(const q31_t* src, q31_t* dst, uint32_t block_size); /*! <- The function uses saturating arithmetic. The Q31 value -1 (0x80000000) will be saturated to the maximum allowable positive value 0x7FFFFFFF. */

/**
 * @brief Element-by-element addition of two vectors.
 * 
 *  dst[n] = src_a[n] + src_b[n],   0 <= n < blockSize.
 *  There are separate functions for Q7, Q15, and Q31 data types. 
 * 
 * Parameters
 * @param [in]  src_a   points to first input vector 
 * @param [in]  src_b   points to second input vector 
 * @param [out] dst    points to output vector 
 * @param [in]  block_size   number of samples in each vector 
 * @return      none
 */

typedef void (*dsp_add_q7_addr)(const q7_t* src_a, const q7_t* src_b, q7_t* dst,
                 uint32_t block_size); /*! <- The function uses saturating arithmetic. Results outside of the allowable Q7 range [0x80 0x7F] are saturated. */
typedef void (*dsp_add_q15_addr)(const q15_t* src_a, const q15_t* src_b, q15_t* dst,
                  uint32_t block_size); /*! <- The function uses saturating arithmetic. Results outside of the allowable Q15 range [0x8000 0x7FFF] are saturated. */
typedef void (*dsp_add_q31_addr)(const q31_t* src_a, const q31_t* src_b, q31_t* dst,
                  uint32_t block_size); /*! <- The function uses saturating arithmetic. Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] are saturated. */

/**
 * @brief Compute the Arc tangent of y/x:
 *  The sign of y and x are used to determine the right quadrant and compute the right angle. 
 * 
 * Computing Arc tangent only using the ratio y/x is not enough to determine the angle since there is an indeterminacy. Opposite quadrants are giving the same ratio.
 * ArcTan2 is not using y/x to compute the angle but y and x and use the sign of y and x to determine the quadrant. 
 * 
 * Parameters
 * @param  [in]    y   y coordinate
 * @param  [in]    x   x coordinate
 * @param  [out]   result 
 * @return         error status
 */
typedef dsp_status_t (*dsp_atan2_f32_addr)(float32_t y, float32_t x, float32_t* result); /*! <- Function for floating-point parameters*/
typedef dsp_status_t (*dsp_atan2_q15_addr)(q15_t y, q15_t x, q15_t* result); /*! <- function for Q15 parameters, Result in Q2.13 */
typedef dsp_status_t (*dsp_atan2_q31_addr)(q31_t y, q31_t x, q31_t* result); /*! <- function for Q31 parameters, Result in Q2.29 */

/**
 * @brief Element-by-element limitation of the range of vector values
 * 
 * Parameters
 * @param [IN]  src     points to the input vector
 * @param [OUT] dst     points to the output vector
 * @param [IN]  low     The lower limit of the saturation range
 * @param [IN]  high    The higher limit of the saturation range
 * @param [IN]  num_samples  number of samples in each vector
 * @return              none
 */
typedef void (*dsp_clip_q7_addr)(const q7_t* src, q7_t* dst, q7_t low, q7_t high,
                  uint32_t num_samples);
typedef void (*dsp_clip_q15_addr)(const q15_t* src, q15_t* dst, q15_t low, q15_t high,
                   uint32_t num_samples);
typedef void (*dsp_clip_q31_addr)(const q31_t* src, q31_t* dst, q31_t low, q31_t high,
                   uint32_t num_samples);

/**
 * @brief Computes the trigonometric cosine function using a combination of table lookup and linear interpolation. There are separate functions for Q15, Q31, and floating-point data types. The input to the floating-point version is in radians while the fixed-point Q15 and Q31 have a scaled input with the range [0 +0.9999] mapping to [0 2*pi). The fixed-point range is chosen so that a value of 2*pi wraps around to 0.
 * 
 * The implementation is based on table lookup using 512 values together with linear interpolation. The steps used are:
 *      Calculation of the nearest integer table index
 *      Compute the fractional portion (fract) of the table index.
 *      The final result equals (1.0f-fract)*a + fract*b;
 *  where
 *      a = Table[index];
 *      b = Table[index+1];
 * 
 * Parameters
 * @param [in]    x   input value in radians. 
 */
typedef float32_t (*dsp_cos_f32_addr)(float32_t x); /*! <- Floating point input&output value */
typedef q15_t (*dsp_cos_q15_addr)(q15_t x); /*! <- The Q15 input value is in the range [0 +0.9999] and is mapped to a radian value in the range [0 2*PI). */
typedef q31_t (*dsp_cos_q31_addr)(q31_t x); /*! <- The Q31 input value is in the range [0 +0.9999] and is mapped to a radian value in the range [0 2*PI). */

/**
 * @brief         Fixed point division
 * 
 * @param[in]     numerator    Numerator
 * @param[in]     denominator  Denominator
 * @param[out]    quotient     Quotient value normalized between -1.0 and 1.0
 * @param[out]    shift        Shift left value to get the unnormalized quotient
 * @return        error status
 *
 * When dividing by 0, an error BEAR_MATH_NANINF is returned. And the quotient is forced
 * to the saturated negative or positive value.
 */
typedef dsp_status_t (*dsp_divide_q15_addr)(q15_t numerator, q15_t denominator,
                              q15_t* quotient, int16_t* shift);
typedef dsp_status_t (*dsp_divide_q31_addr)(q31_t numerator, q31_t denominator,
                              q31_t* quotient, int16_t* shift);

/**
 * @brief         Dot product of vectors.
 * 
 * Computes the dot product of two vectors.
 * The vectors are multiplied element-by-element and then summed.
 * 
 * sum = pSrcA[0]*pSrcB[0] + pSrcA[1]*pSrcB[1] + ... + pSrcA[blockSize-1]*pSrcB[blockSize-1]
 * 
 * @param[in]     src_a       points to the first input vector
 * @param[in]     srcb        points to the second input vector
 * @param[in]     block_size  number of samples in each vector
 * @param[out]    result      output result returned here
 * @return        none
 * 
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 */
typedef void (*dsp_dot_prod_f32_addr)(const float32_t* src_a, const float32_t* src_b,
                       uint32_t block_size, float32_t* result); /*! <- floating-point vectors*/
typedef void (*dsp_dot_prod_q7_addr)(const q7_t* src_a, const q7_t* src_b, uint32_t block_size,
                      q31_t* result); /*! <-    @par           Scaling and Overflow Behavior
                                       *        The intermediate multiplications are in 1.7 x 1.7 = 2.14 format and these
                                       *        results are added to an accumulator in 18.14 format.
                                       *        Nonsaturating additions are used and there is no danger of wrap around as long as  
                                       *        he vectors are less than 2^18 elements long.
                                       *        The return result is in 18.14 format.*/
typedef void (*dsp_dot_prod_q15_addr)(const q15_t* src_a, const q15_t* src_b,
                       uint32_t block_size, q63_t* result); /*! <- @par           Scaling and Overflow Behavior
                                                             *     The intermediate multiplications are in 1.15 x 1.15 = 2.30 format and these
                                                             *     results are added to a 64-bit accumulator in 34.30 format.
                                                             *     Nonsaturating additions are used and given that there are 33 guard bits in the accumulator
                                                             *     there is no risk of overflow.
                                                             *     The return result is in 34.30 format. */

typedef void (*dsp_dot_prod_q31_addr)(const q31_t* src_a, const q31_t* src_b,
                       uint32_t block_size, q63_t* result); /*! <- @par           Scaling and Overflow Behavior
                                                             *     The intermediate multiplications are in 1.31 x 1.31 = 2.62 format and these
                                                             *     are truncated to 2.48 format by discarding the lower 14 bits.
                                                             *     The 2.48 result is then added without saturation to a 64-bit accumulator in 16.48 format.
                                                             *     There are 15 guard bits in the accumulator and there is no risk of overflow as long as
                                                             *     the length of the vectors is less than 2^16 elements.
                                                             *     The return result is in 16.48 format.
                                                             */

/**
 * @brief        Vector multiplication
 * 
 *  C = A * B 
 * 
 * @param[in]     src_a         points to the first input vector
 * @param[in]     src_b         points to the second input vector
 * @param[out]    dst           points to the output vector
 * @param[in]     block_size    number of samples in each vector
 * @return        none
 * 
 * @par           Scaling and Overflow Behavior
 *                Functions uses saturating arithmetic.
 *                Results outside of the allowable range are saturated.
 */
typedef void (*dsp_mult_q7_addr)(const q7_t* src_a, const q7_t* src_b, q7_t* dst,
                  uint32_t block_size); /*! <- Q7, range [0x80 0x7F] */
typedef void (*dsp_mult_q15_addr)(const q15_t* src_a, const q15_t* src_b, q15_t* dst,
                   uint32_t block_size); /*! <- Q15, range [0x8000 0x7FFF] */
typedef void (*dsp_mult_q31_addr)(const q31_t* src_a, const q31_t* src_b, q31_t* dst,
                   uint32_t block_size); /*! <- Q31, range [0x80000000 0x7FFFFFFF] */

/**
 * @brief         Negates the elements of a vector.
 * 
 * dst[n] = -src[n]
 * 
 * @param[in]     src          points to the input vector.
 * @param[out]    dst          points to the output vector.
 * @param[in]     block_size   number of samples in each vector.
 * @return        none
 * 
 * @par           Scaling and Overflow Behavior
 *                Functions uses saturating arithmetic.
 *                The value -1 is saturated to the maximum allowable positive value.
 */
typedef void (*dsp_negate_q7_addr)(const q7_t* src, q7_t* dst, uint32_t block_size); /*! <- Q7, range [0x80 0x7F] */
typedef void (*dsp_negate_q15_addr)(const q15_t* src, q15_t* dst, uint32_t block_size); /*! <- Q15, range [0x8000 0x7FFF] */
typedef void (*dsp_negate_q31_addr)(const q31_t* src, q31_t* dst, uint32_t block_size); /*! <- Q31, range [0x80000000 0x7FFFFFFF] */

/**
 * @brief         Adds a constant offset to each element of a vector.
 * 
 * dst[n] = src[n] + offset
 * 
 * The functions support in-place computation allowing the source and
 * destination pointers to reference the same memory buffer.
 * There are separate functions for floating-point, Q7, Q15, and Q31 data types.
 * 
 * @param[in]     src            points to the input vector
 * @param[in]     offset         is the offset to be added
 * @param[out]    dst            points to the output vector
 * @param[in]     block_size     number of samples in each vector
 * @return        none
 *
 * @par           Scaling and Overflow Behavior
 *                  In functions, that uses saturating arithmetic,
 *                  results outside of the allowable range are saturated.
 * 
 */
typedef void (*dsp_offset_f32_addr)(const float32_t* src, float32_t offset, float32_t* dst,
                     uint32_t block_size); /*! <- Function for floating-point parameters*/
typedef void (*dsp_offset_q7_addr)(const q7_t* src, q7_t offset, q7_t* dst,
                    uint32_t block_size); /*! <- Q7, range [0x80 0x7F] */
typedef void (*dsp_offset_q15_addr)(const q15_t* src, q15_t offset, q15_t* dst,
                     uint32_t block_size); /*! <- Q15, range [0x8000 0x7FFF] */
typedef void (*dsp_offset_q31_addr)(const q31_t* src, q31_t offset, q31_t* dst,
                     uint32_t block_size); /*! <- Q31, range [0x80000000 0x7FFFFFFF] */

/**
 * @brief         Multiplies each element of a vector by a scalar.
 * 
 * dst[n] = src[n] * scale_fract << shift
 * 
 * @param[in]     pSrc       points to the input vector
 * @param[in]     scaleFract fractional portion of the scale value
 * @param[in]     shift      number of bits to shift the result by
 * @param[out]    pDst       points to the output vector
 * @param[in]     blockSize  number of samples in each vector
 * @return        none
 */
typedef void (*dsp_scale_q7_addr)(const q7_t* src, q7_t scale_fract, int8_t shift, q7_t* dst,
                   uint32_t block_size); /*! <- @par           Scaling and Overflow Behavior
                                          *     The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.7 format.
                                          *     These are multiplied to yield a 2.14 intermediate result and this is shifted with saturation to 1.7 format. */
typedef void (*dsp_scale_q15_addr)(const q15_t* src, q15_t scale_fract, int8_t shift,
                    q15_t* dst, uint32_t block_size); /*! <- @par           Scaling and Overflow Behavior
                                          *     The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.15 format.
                                          *     These are multiplied to yield a 2.30 intermediate result and this is shifted with saturation to 1.15 format. */
typedef void (*dsp_scale_q31_addr)(const q31_t* src, q31_t scale_fract, int8_t shift,
                    q31_t* dst, uint32_t block_size); /*! <- @par           Scaling and Overflow Behavior
                                          *     The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.31 format.
                                          *     These are multiplied to yield a 2.62 intermediate result and this is shifted with saturation to 1.31 format. */

/**
 * @brief         Shifts the elements of a vector a specified number of bits
 * 
 *  C = A << shiftBits 
 * 
 * @param[in]     src         points to the input vector
 * @param[in]     shift_bits  number of bits to shift.  A positive value shifts left; a negative value shifts right.
 * @param[out]    dst         points to the output vector
 * @param[in]     block_size  number of samples in each vector
 * @return        none
 *
 * @par           onditions for optimum performance
 *                  Input and output buffers should be aligned by 32-bit
 * @par           Scaling and Overflow Behavior
 *                  The function uses saturating arithmetic.
 *                  Results outside of the allowable range are saturated.
 */
typedef void (*dsp_shift_q7_addr)(const q7_t* src, int8_t shift_bits, q7_t* dst,
                   uint32_t block_size); /*! <- Q7, range [0x80 0x7F] */
typedef void (*dsp_shift_q15_addr)(const q15_t* src, int8_t shift_bits, q15_t* dst,
                    uint32_t block_size); /*! <- Q15, range [0x8000 0x7FFF] */
typedef void (*dsp_shift_q31_addr)(const q31_t* src, int8_t shift_bits, q31_t* dst,
                    uint32_t block_size); /*! <- Q31, range [0x80000000 0x7FFFFFFF] */

/**
 * @brief         Fast approximation to the trigonometric sine function.
 * 
 * Computes the trigonometric sine function using a combination of table lookup
 * and linear interpolation.  There are separate functions for
 * Q15, Q31, and floating-point data types.
 * The input to the floating-point version is in radians while the
 * fixed-point Q15 and Q31 have a scaled input with the range
 * [0 +0.9999] mapping to [0 2*pi).  The fixed-point range is chosen so that a
 * value of 2*pi wraps around to 0.
 *
 * The implementation is based on table lookup using 512 values together with linear interpolation.
 * The steps used are:
 *  -# Calculation of the nearest integer table index
 *  -# Compute the fractional portion (fract) of the table index.
 *  -# The final result equals <code>(1.0f-fract)*a + fract*b;</code>
 *
 * where
 * <pre>
 *    b = Table[index];
 *    c = Table[index+1];
 * </pre>
 *
 * @param[in]     x  [Scaled] input value in radians
 * @return        sin(x)
 */
typedef float32_t (*dsp_sin_f32_addr)(float32_t x); /*! <- Function for floating-point parameters. Input value in radians.*/
typedef q15_t (*dsp_sin_q15_addr)(q15_t x); /*! <- The Q15 input value is in the range [0 +0.9999] and is mapped to a radian value in the range [0 2*PI). */
typedef q31_t (*dsp_sin_q31_addr)(q31_t x); /*! <- The Q31 input value is in the range [0 +0.9999] and is mapped to a radian value in the range [0 2*PI). */

/**
 * @brief         Counts square root.
 * @param[in]     in    input value. 
 * @param[out]    out   points to square root of input value
 * @return        execution status
                   - \ref BEAR_MATH_SUCCESS        : input value is positive
                   - \ref BEAR_MATH_ARGUMENT_ERROR : input value is negative; *pOut is set to 0
 */
typedef dsp_status_t (*dsp_sqrt_f32_addr)(const float32_t in, float32_t* pOut); /*! <- Function for floating-point parameters*/
typedef dsp_status_t (*dsp_sqrt_q15_addr)(q15_t in, q15_t* out); /*! <- Q15 function. The range of the input value is [0 +1) or 0x0000 to 0x7FFF*/
typedef dsp_status_t (*dsp_sqrt_q31_addr)(q31_t in, q31_t* out); /*! <- Q31 function. The range of the input value is [0 +1) or 0x00000000 to 0x7FFFFFFF*/

/**
 * @brief         Element-by-element vector subtraction.
 * 
 * dst[n] = src_a[n] - src_b[n]
 * 
 * @param[in]     src_a       points to the first input vector
 * @param[in]     src_b       points to the second input vector
 * @param[out]    dst         points to the output vector
 * @param[in]     block_size  number of samples in each vector
 * @return        none
 *
 * @par           Scaling and Overflow Behavior
 *                  The function uses saturating arithmetic.
 *                  Results outside of the allowable range will be saturated.
 */
typedef void (*dsp_sub_q7_addr)(const q7_t* src_a, const q7_t* src_b, q7_t* dst,
                 uint32_t block_size); /*! <- Q7, range [0x80 0x7F] */
typedef void (*dsp_sub_q15_addr)(const q15_t* src_a, const q15_t* src_b, q15_t* dst,
                  uint32_t block_size); /*! <- Q15, range [0x8000 0x7FFF] */
typedef void (*dsp_sub_q31_addr)(const q31_t* src_a, const q31_t* src_b, q31_t* dst,
                  uint32_t block_size); /*! <- Q31, range [0x80000000 0x7FFFFFFF] */

/**
 * @brief         Compute the exp values of a vector of samples.
 * 
 *  C = e ^ A 
 * 
 * @param[in]     src           points to the input vector
 * @param[out]    dst           points to the output vector
 * @param[in]     block_size    number of samples in each vector
 * @return        none
 */
typedef void (*dsp_vexp_f32_addr)(const float32_t* src, float32_t* dst, uint32_t block_size); /*! <- Floating-point vector of exp values. */

/**
 * @brief          Compute the log values of a vector of samples.
 * 
 * C = log(A)
 * 
 * @param[in]     src           points to the input vector
 * @param[out]    dst           points to the output vector
 * @param[in]     block_size    number of samples in each vector
 * @return        none
 */
typedef void (*dsp_vlog_f32_addr)(const float32_t* src, float32_t* dst, uint32_t block_size);
typedef void (*dsp_vlog_q15_addr)(const q15_t* src, q15_t* dst, uint32_t block_size);  /*! <- input vector in q15, output vector in q4.11*/
typedef void (*dsp_vlog_q31_addr)(const q31_t* src, q31_t* dst, uint32_t block_size);  /*! <- input vector in q31, output vector in q5.26*/


#define dsp_abs_q7(a,b,c) ((dsp_abs_q7_addr)(0x400089e8))(a,b,c)
#define dsp_abs_q15(a,b,c) ((dsp_abs_q15_addr)(0x40008a92))(a,b,c)
#define dsp_abs_q31(a,b,c) ((dsp_abs_q31_addr)(0x40008b88))(a,b,c)
#define dsp_add_q7(a,b,c,d) ((dsp_add_q7_addr)(0x4000952a))(a,b,c,d)
#define dsp_add_q15(a,b,c,d) ((dsp_add_q15_addr)(0x4000960a))(a,b,c,d)
#define dsp_add_q31(a,b,c,d) ((dsp_add_q31_addr)(0x4000974e))(a,b,c,d)
#define dsp_atan2_f32(a,b,c) ((dsp_atan2_f32_addr)(0x40009074))(a,b,c)
#define dsp_atan2_q15(a,b,c) ((dsp_atan2_q15_addr)(0x40008f5e))(a,b,c)
#define dsp_atan2_q31(a,b,c) ((dsp_atan2_q31_addr)(0x40008f62))(a,b,c)
#define dsp_clip_q7(a,b,c,d,e) ((dsp_clip_q7_addr)(0x40009808))(a,b,c,d,e)
#define dsp_clip_q15(a,b,c,d,e) ((dsp_clip_q15_addr)(0x400098f0))(a,b,c,d,e)
#define dsp_clip_q31(a,b,c,d,e) ((dsp_clip_q31_addr)(0x40009a22))(a,b,c,d,e)
#define dsp_cos_f32(a) ((dsp_cos_f32_addr)(0x4000b734))(a)
#define dsp_cos_q15(a) ((dsp_cos_q15_addr)(0x4000b694))(a)
#define dsp_cos_q31(a) ((dsp_cos_q31_addr)(0x4000b6ea))(a)
#define dsp_divide_q15(a,b,c,d) ((dsp_divide_q15_addr)(0x40008c1a))(a,b,c,d)
#define dsp_divide_q31(a,b,c,d) ((dsp_divide_q31_addr)(0x40008c1e))(a,b,c,d)
#define dsp_dot_prod_f32(a,b,c,d) ((dsp_dot_prod_f32_addr)(0x40009dda))(a,b,c,d)
#define dsp_dot_prod_q7(a,b,c,d) ((dsp_dot_prod_q7_addr)(0x40009ad8))(a,b,c,d)
#define dsp_dot_prod_q15(a,b,c,d) ((dsp_dot_prod_q15_addr)(0x40009b90))(a,b,c,d)
#define dsp_dot_prod_q31(a,b,c,d) ((dsp_dot_prod_q31_addr)(0x40009cb0))(a,b,c,d)
#define dsp_mult_q7(a,b,c,d) ((dsp_mult_q7_addr)(0x40008c88))(a,b,c,d)
#define dsp_mult_q15(a,b,c,d) ((dsp_mult_q15_addr)(0x40008d68))(a,b,c,d)
#define dsp_mult_q31(a,b,c,d) ((dsp_mult_q31_addr)(0x40008ea8))(a,b,c,d)
#define dsp_negate_q7(a,b,c) ((dsp_negate_q7_addr)(0x40009dde))(a,b,c)
#define dsp_negate_q15(a,b,c) ((dsp_negate_q15_addr)(0x40009ea6))(a,b,c)
#define dsp_negate_q31(a,b,c) ((dsp_negate_q31_addr)(0x40009fae))(a,b,c)
#define dsp_offset_f32(a,b,c,d) ((dsp_offset_f32_addr)(0x400092dc))(a,b,c,d)
#define dsp_offset_q7(a,b,c,d) ((dsp_offset_q7_addr)(0x40009078))(a,b,c,d)
#define dsp_offset_q15(a,b,c,d) ((dsp_offset_q15_addr)(0x4000913a))(a,b,c,d)
#define dsp_offset_q31(a,b,c,d) ((dsp_offset_q31_addr)(0x40009242))(a,b,c,d)
#define dsp_scale_q7(a,b,c,d,e) ((dsp_scale_q7_addr)(0x4000a04e))(a,b,c,d,e)
#define dsp_scale_q15(a,b,c,d,e) ((dsp_scale_q15_addr)(0x4000a1b8))(a,b,c,d,e)
#define dsp_scale_q31(a,b,c,d,e) ((dsp_scale_q31_addr)(0x4000a2f8))(a,b,-(c),d,e)
#define dsp_shift_q7(a,b,c,d) ((dsp_shift_q7_addr)(0x400092e0))(a,b,c,d)
#define dsp_shift_q15(a,b,c,d) ((dsp_shift_q15_addr)(0x4000938e))(a,b,c,d)
#define dsp_shift_q31(a,b,c,d) ((dsp_shift_q31_addr)(0x40009490))(a,b,c,d)
#define dsp_sin_f32(a) ((dsp_sin_f32_addr)(0x4000b844))(a)
#define dsp_sin_q15(a) ((dsp_sin_q15_addr)(0x4000b7b0))(a)
#define dsp_sin_q31(a) ((dsp_sin_q31_addr)(0x4000b800))(a)
#define dsp_sqrt_f32(a,b) ((dsp_sqrt_f32_addr)(0x4000b4e4))(a,b)
#define dsp_sqrt_q15(a,b) ((dsp_sqrt_q15_addr)(0x4000b342))(a,b)
#define dsp_sqrt_q31(a,b) ((dsp_sqrt_q31_addr)(0x4000b426))(a,b)
#define dsp_sub_q7(a,b,c,d) ((dsp_sub_q7_addr)(0x4000a3b0))(a,b,c,d)
#define dsp_sub_q15(a,b,c,d) ((dsp_sub_q15_addr)(0x4000a490))(a,b,c,d)
#define dsp_sub_q31(a,b,c,d) ((dsp_sub_q31_addr)(0x4000a5d4))(a,b,c,d)
#define dsp_vexp_f32(a,b,c) ((dsp_vexp_f32_addr)(0x4000b50e))(a,b,c)
#define dsp_vlog_f32(a,b,c) ((dsp_vlog_f32_addr)(0x4000b664))(a,b,c)
#define dsp_vlog_q15(a,b,c) ((dsp_vlog_q15_addr)(0x4000b53e))(a,b,c)
#define dsp_vlog_q31(a,b,c) ((dsp_vlog_q31_addr)(0x4000b5d4))(a,b,c)

#endif //__BMCU_ROM1_DSP_H

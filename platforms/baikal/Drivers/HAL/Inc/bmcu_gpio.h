/**
 * *****************************************************************************
 *  @file       bmcu_gpio.h
 *  @author     Baikal electronics SDK team
 *  @brief      General-Purpose Input/Output (GPIO) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * 
 *  File content:
 *      - GPIO related type definitions and enumerations
 *      - Macros for GPIO control and status check
 *      - Macros for GPIO interrupts control and status check
 *      - GPIO module export functions prototypes
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_GPIO_H
#define __BMCU_GPIO_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief GPIO pins */
#define GPIO_PIN_0                      0x0001U     /*!< Pin 0  */
#define GPIO_PIN_1                      0x0002U     /*!< Pin 1  */
#define GPIO_PIN_2                      0x0004U     /*!< Pin 2  */
#define GPIO_PIN_3                      0x0008U     /*!< Pin 3  */
#define GPIO_PIN_4                      0x0010U     /*!< Pin 4  */
#define GPIO_PIN_5                      0x0020U     /*!< Pin 5  */
#define GPIO_PIN_6                      0x0040U     /*!< Pin 6  */
#define GPIO_PIN_7                      0x0080U     /*!< Pin 7  */
#define GPIO_PIN_8                      0x0100U     /*!< Pin 8  */
#define GPIO_PIN_9                      0x0200U     /*!< Pin 9  */
#define GPIO_PIN_10                     0x0400U     /*!< Pin 10 */
#define GPIO_PIN_11                     0x0800U     /*!< Pin 11 */
#define GPIO_PIN_12                     0x1000U     /*!< Pin 12 */
#define GPIO_PIN_13                     0x2000U     /*!< Pin 13 */
#define GPIO_PIN_14                     0x4000U     /*!< Pin 14 */
#define GPIO_PIN_15                     0x8000U     /*!< Pin 15 */
#define GPIO_PIN_ALL                    (GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | \
                                         GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_5  | \
                                         GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_8  | \
                                         GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | \
                                         GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | \
                                         GPIO_PIN_15)   /*!< All pins */

/**
 * @typedef
 * @brief Pin input/output mode types enumeration
 *        Used for control the direction of data pin in GPIO port
 * @note  Value after reset: GPIO_MODE_INPUT
 * @see   GPIO_SetPinMode(), GPIO_GetPinMode()
 */
typedef enum {
    GPIO_MODE_INPUT          = 0,   ///< Pin configured as INPUT
    GPIO_MODE_OUTPUT         = 1    ///< Pin configured as OUTPUT
} GPIO_Mode_TypeDef;

/**
 * @typedef
 * @brief GPIO initialization structure type definition
 * Used in GPIO_Init()
 */
typedef struct {
    uint16_t PinMask;           ///< Pin or set of pins to be initialized
    GPIO_Mode_TypeDef Mode;     ///< Pin mode: @ref GPIO_Mode_TypeDef
} GPIO_InitStruct_TypeDef;

/**
 * @typedef
 * @brief Pin interrupt enable or disable states enumeration
 *        Used for pin interrupt enable control.
 * @note  Value after reset: GPIO_IT_DISABLE
 * @see   GPIO_SetPinIT(), GPIO_IsEnabledPinIT()
 */
typedef enum {
    GPIO_IT_DISABLE          = 0,   ///< Disable pin interrupt generation
    GPIO_IT_ENABLE           = 1    ///< Enable pin interrupt generation
} GPIO_ITEn_TypeDef;

/**
 * @typedef
 * @brief Pin interrupt mask states enumeration
 *        Used to control if GPIO pin interrupts can create an interupt for MCU 
 *        interrupt controller
 * @note  By default all interrupts bits are unmasket, i.e. have
 *        GPIO_IT_UNMASKED value
 * @see   GPIO_SetPinITMsk(), GPIO_IsEnabledPinITMsk()
 */
typedef enum {
    GPIO_IT_UNMASKED         = 0,   ///< Interrupt bit from GPIO pin is unmasked
    GPIO_IT_MASKED           = 1    ///< Interrupt bit from GPIO pin is masked
} GPIO_ITMsk_TypeDef;


/**
 * @typedef
 * @brief Pin interrupt types (by level or by edge) enumeration
 * @note  Value after reset: GPIO_IT_TYPE_LEVEL
 * @see   GPIO_SetPinITType(), GPIO_GetPinITType()
 */
typedef enum {
    GPIO_IT_TYPE_LEVEL       = 0,   ///< Interrupt is level sensitive
    GPIO_IT_TYPE_EDGE        = 1    ///< Interrupt is edge sensitive
} GPIO_ITType_TypeDef;

/**
 * @typedef
 * @brief Pin interrupts polarity types enumeration
 *            Polarity of edge or level sensitivity that can
 *            occur on input of GPIO port.
 *            GPIO_IT_POL_LOW: interrupt type to falling-edge or
 *            active-low sensitive
 *            GPIO_IT_POL_HIGH: interrupt is rising-edge or active-high
 *            sensitive
 * @note      Value after reset: GPIO_IT_POL_LOW
 * @see       GPIO_SetPinITPolarity(), GPIO_GetPinITPolarity()
 */
typedef enum {
    GPIO_IT_POL_LOW          = 0,   ///< Active Low polarity
    GPIO_IT_POL_HIGH         = 1    ///< Active High polarity
} GPIO_ITPol_TypeDef;

/**
 * @typedef
 * @brief Pin interrupt state enumeration
 * @note  Value after reset: GPIO_IT_INACTIVE
 * @see   GPIO_IsActivePinIT()
 */
typedef enum {
    GPIO_IT_INACTIVE         = 0,   ///< Interrupt is disabled
    GPIO_IT_ACTIVE           = 1    ///< Interrupt is enabled
} GPIO_ITStat_TypeDef;

/**
 * @typedef
 * @brief Pin interrupt raw (premasking) states enumeration
 * @note  Value after reset: GPIO_IT_RAW_INACTIVE
 * @see   GPIO_IsActivePinITRaw()
 */
typedef enum {
    GPIO_IT_RAW_INACTIVE     = 0,   ///< Interrupt is inactive
    GPIO_IT_RAW_ACTIVE       = 1    ///< Interrupt is active
} GPIO_ITRawStat_TypeDef;

/**
 * @typedef
 * @brief Pin external signal interrupts debounce states enumeration
 *        If an external signal that is the source of an
 *        interrupt needs to be debounced to remove any spurious
 *        glitches.
 *        GPIO_DEBOUNCE_DISABLE: No debounce
 *        GPIO_DEBOUNCE_ENABLE: Enable debounce
 * @note  Value after reset: GPIO_DEBOUNCE_DISABLE
 * @see   GPIO_SetPinDebounce(), GPIO_IsEnabledPinDebounce()
 */
typedef enum {
    GPIO_IT_DEBOUNCE_DISABLE = 0,   ///< No debounce
    GPIO_IT_DEBOUNCE_ENABLE  = 1    ///< Enable debounce logic
} GPIO_ITDebounce_TypeDef;

/**
 * @typedef
 * @brief GPIO port sycnronization level states enumeration
 *        All level-sensitive interrupts being synchronized to clock signal
 * @note  Value after reset: GPIO_IT_SYNC_DISABLE
 * @see   GPIO_SetSyncIT(), GPIO_IsEnabledSyncIT()
 */
typedef enum {
    GPIO_IT_SYNC_DISABLE     = 0,    ///< No synchronization to clock signal 
    GPIO_IT_SYNC_ENABLE      = GPIO_SYNC     ///< Synchronize to clock signal
} GPIO_ITSync_TypeDef;

/**
 * @typedef
 * @brief Pin interrupts single/both edges types enumeration
 *        Edge type of interrupt that can occur on GPIO port.
 *        GPIO_IT_BOTHEDGE_DISABLE: interrupt type depends on the value of
 *        the corresponding bits in the INTLVL and INTPOLARITY registers
 *        GPIO_IT_BOTHEDGE_ENABLE: enables the generation of interrupts on
 *        both the rising edge and the falling edge of an external input
 *        signal corresponding to that bit on GPIO port
 * @note  Value after reset: GPIO_IT_BOTHEDGE_DISABLE
 * @see   GPIO_SetPinBothEdgeIT(), GPIO_IsEnabledPinBothEdgeIT()
 */
typedef enum {
    GPIO_IT_BOTHEDGE_DISABLE = 0,   ///< Single edge sensitive
    GPIO_IT_BOTHEDGE_ENABLE  = 1    ///< Both edge sensitive
} GPIO_ITBothEdge_TypeDef;

/**
 * @brief Writes a value in GPIO register.
 * @param INSTANCE The GPIO instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define GPIO_WriteReg(INSTANCE, REG, VALUE)     WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in GPIO register.
 * @param INSTANCE The GPIO instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define GPIO_ReadReg(INSTANCE, REG)             READ_REG(INSTANCE->REG)

/**
 * @brief  Configure GPIO mode for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask The GPIO pin. Can be a combination of CRU_PIN_x values
 * @param  Mode selects pin mode. Can be one of @ref GPIO_Mode_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinMode (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_Mode_TypeDef Mode) {
    MODIFY_REG(GPIOx->DDR, PinMask, (PinMask * Mode));
}

/**
 * @brief  Get GPIO mode configuration for the dedicated pin on the dedicated port
 * @param  GPIOx GPIO Port
 * @param  Pin Requested GPIO port pin. Can be one of GPIO_PIN_x values
 * @retval Requested pin direction configuration. Can be one of @ref GPIO_Mode_TypeDef
 */
__STATIC_INLINE GPIO_Mode_TypeDef
GPIO_GetPinMode (GPIO_TypeDef *GPIOx, uint16_t Pin) {
    return ((GPIOx->DDR & Pin) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
}

/**
 * @brief  Write data to the dedicated GPIO port output data register (DR)
 * @param  GPIOx GPIO Port
 * @param  PortValue Value to be written to 16-bit output GPIO port register.
           Value range: 0x0000 .. 0xFFFF
 * @retval none
 */
__STATIC_INLINE void
GPIO_WriteOutputPort (GPIO_TypeDef *GPIOx, uint16_t PortValue) {
    WRITE_REG(GPIOx->DR, PortValue);
}

/**
 * @brief  Read data from the dedicated GPIO port output register (DR)
 * @param  GPIOx GPIO Port
 * @retval 16-bit value, read from GPIO port output register
 */
__STATIC_INLINE uint16_t
GPIO_ReadOutputPort (GPIO_TypeDef *GPIOx) {
    return (uint16_t)(READ_REG(GPIOx->DR) & GPIO_PIN_ALL);
}

/**
 * @brief  Read data from the dedicated GPIO port input register (EXT)
 * @param  GPIOx GPIO Port
 * @retval 16-bit value, read from GPIO port input register
 */
__STATIC_INLINE uint16_t
GPIO_ReadInputPort (GPIO_TypeDef *GPIOx) {
    return (uint16_t)(READ_REG(GPIOx->EXT) & GPIO_PIN_ALL);
}

/**
 * @brief  Set pins to high level for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetOutputPin (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    MODIFY_REG(GPIOx->DR, 0, PinMask);
}

/**
 * @brief  Set pins to low level for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @retval none
 */
__STATIC_INLINE void
GPIO_ResetOutputPin (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    MODIFY_REG(GPIOx->DR, PinMask, 0);
}

/**
 * @brief  Check if all pins of dedicated pin set on the dedicated GPIO port are set to high level
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if all dedicated requested pins are set to high level,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsOutputPinSet (GPIO_TypeDef *GPIOx,  uint16_t PinMask) {
    return (PinMask == READ_BIT(GPIOx->DR, PinMask));
}

/**
 * @brief  Toggle level for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @retval none
 */
__STATIC_INLINE void
GPIO_ToggleOutputPin (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    WRITE_REG(GPIOx->DR, READ_REG(GPIOx->DR) ^ PinMask);
}

/**
 * @brief  Check if all pins of dedicated pin set on the dedicated GPIO port
 *         are read as high
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be read. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if all dedicated requested pins are in the high level,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsInputPinSet (GPIO_TypeDef *GPIOx,  uint16_t PinMask) {
    return (PinMask == READ_BIT(GPIOx->EXT, PinMask));
}

/**
 * @brief  Configure interrupt for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @param  ITState Interrupt state value. Can be one of @ref GPIO_ITEn_TypeDef valeus
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinIT (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITEn_TypeDef ITState) {
    MODIFY_REG(GPIOx->INTEN, PinMask, (ITState & 1) ? PinMask : 0);
}

/**
 * @brief  Check if interrupts are enabled for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if interrupts are enabled for all of requested pins,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsEnabledPinIT (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_ENABLE & 1) ? PinMask : 0) == READ_BIT(GPIOx->INTEN, PinMask));
}

/**
 * @brief  Configure interrupt masking for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @param  ITMskState Interrupt mask state value. Can be one of @ref GPIO_ITMsk_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinITMsk (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITMsk_TypeDef ITMskState) {
    MODIFY_REG(GPIOx->INTMSK, PinMask, (ITMskState & 1) ? PinMask : 0);
}

/**
 * @brief  Check if interrupts are masked for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if interrupts are masked for all of requested pins,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsEnabledPinITMsk (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_MASKED & 1) ? PinMask : 0) == READ_BIT(GPIOx->INTMSK, PinMask));
}

/**
 * @brief  Configure interrupt triggering type for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @param  ITType Interrupt triggering type. Can be one of @ref GPIO_ITType_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinITType (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITType_TypeDef ITType) {
    MODIFY_REG(GPIOx->INTLVL, PinMask, (ITType & 1) ? PinMask : 0);
}

/**
 * @brief  Get interrput triggering type for the dedicated pin on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  Pin Requested GPIO port pin. Can be one of GPIO_PIN_x values
 * @retval Interrupt triggering type. Can be one of @ref GPIO_ITType_TypeDef values
 */
__STATIC_INLINE GPIO_ITType_TypeDef
GPIO_GetPinITType (GPIO_TypeDef *GPIOx, uint16_t Pin) {
    return ((GPIO_IT_TYPE_LEVEL == (READ_BIT(GPIOx->INTLVL, Pin) >> POSITION_VAL(Pin)) ?
                                     GPIO_IT_TYPE_LEVEL : GPIO_IT_TYPE_EDGE));
}

/**
 * @brief  Configure interrupt polarity for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @param  ITPolarity Interrupt polarity value. Can be one of @ref GPIO_ITPol_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinITPolarity (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITPol_TypeDef ITPolarity) {
    MODIFY_REG(GPIOx->INTPOLARITY, PinMask, (ITPolarity & 1) ? PinMask : 0);
}

/**
 * @brief  Get interrput polarity type for the dedicated pin on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  Pin Requested GPIO port pin. Can be one of GPIO_PIN_x values
 * @retval Interrupt polarity value. Can be one of @ref GPIO_ITPol_TypeDef values
 */
__STATIC_INLINE GPIO_ITPol_TypeDef
GPIO_GetPinITPolarity (GPIO_TypeDef *GPIOx, uint16_t Pin) {
    return ((GPIO_IT_POL_LOW == (READ_BIT(GPIOx->INTPOLARITY, Pin) >> POSITION_VAL(Pin)) ?
                                     GPIO_IT_POL_LOW : GPIO_IT_POL_HIGH));
}

/**
 * @brief  Check if interrupt status is active for all of the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if interrupt status for all of selected pins is active,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsActivePinIT (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_ACTIVE & 1) ? PinMask : 0) == READ_BIT(GPIOx->INTSTAT, PinMask));
}

/**
 * @brief  Check if raw interrupt status is active for all of the dedicated pin set on the dedicated GPIO port
 * @note   Raw interrupt status means interrupt status berfore interrupt masking
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested.
                   Can be a combination of GPIO_PIN_x values
 * @retval 1 if raw interrupt status for all of selected pins is active,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsActivePinITRaw (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_RAW_ACTIVE & 1) ? PinMask : 0) == READ_BIT(GPIOx->INTSTATRAW, PinMask));
}

/**
 * @brief  Configure interrupt debounce logic for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
* @param  Debounce Specifies debounce configuration value.
                   Can be one of @ref GPIO_ITDebounce_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinDebounce (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITDebounce_TypeDef Debounce) {
    MODIFY_REG(GPIOx->DEBOUNCE, PinMask, (Debounce & 1) ? PinMask : 0);
}

/**
 * @brief  Check debounce configureation for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if debounce logic is enabled for all of selected pins is active,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsEnabledPinDebounce (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_DEBOUNCE_ENABLE & 1) ? PinMask : 0) == READ_BIT(GPIOx->DEBOUNCE, PinMask));
}

/**
 * @brief  Clear interrupt state for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
 * @retval none
 */
__STATIC_INLINE void
GPIO_ClearPinIT (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    SET_BIT(GPIOx->EOI, PinMask);
}

/**
 * @brief  Configure synchronization for level-sensitive interrupts on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  Sync Syncronization configureation value.
                Can be one of @ref GPIO_ITSync_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetSyncIT (GPIO_TypeDef *GPIOx, GPIO_ITSync_TypeDef Sync) {
    WRITE_REG(GPIOx->SYNC, Sync);
}

/**
 * @brief  Check interrupt synhcronization configuration on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @retval 1 if synhcronization on dedicated GPIO port is enabled ,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsEnabledSyncIT (GPIO_TypeDef *GPIOx) {
    return (GPIO_IT_SYNC_ENABLE == READ_REG(GPIOx->SYNC));
}

/**
 * @brief  Configure interrupt edge triggering type for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be configured. Can be a combination
                   of GPIO_PIN_x values
* @param  EdgeType Interrupts single/both edges type value.
                    Can be one of @ref GPIO_ITBothEdge_TypeDef values
 * @retval none
 */
__STATIC_INLINE void
GPIO_SetPinBothEdgeIT (GPIO_TypeDef *GPIOx, uint16_t PinMask, GPIO_ITBothEdge_TypeDef EdgeType) {
    MODIFY_REG(GPIOx->BOTHEDGE, PinMask, (EdgeType & 1) ? PinMask : 0);
}

/**
 * @brief  Check interrupt edge triggering type for the dedicated pin set on the dedicated GPIO port
 * @param  GPIOx GPIO Port
 * @param  PinMask Set of GPIO pins to be requested. Can be a combination
                   of GPIO_PIN_x values
 * @retval 1 if interrupts for both edges for all dedicated pins are enabled,
           otherwise 0
 */
__STATIC_INLINE uint8_t
GPIO_IsEnabledPinBothEdgeIT (GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    return (((GPIO_IT_BOTHEDGE_ENABLE & 1) ? PinMask : 0) == READ_BIT(GPIOx->BOTHEDGE, PinMask));
}

ErrorStatus GPIO_DeInit(GPIO_TypeDef *GPIOx, GPIO_InitStruct_TypeDef *GPIO_InitStruct);
ErrorStatus GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitStruct_TypeDef *GPIO_InitStruct);
void GPIO_StructInit(GPIO_InitStruct_TypeDef *GPIO_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /*__BMCU_GPIO_H */

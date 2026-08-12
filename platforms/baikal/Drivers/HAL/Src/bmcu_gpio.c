/**
 * *****************************************************************************
 *  @file       bmcu_gpio.c
 *  @author     Baikal electronics SDK team
 *  @brief      General-Purpose Input/Output (GPIO) module driver
 *  @version    2.2.0
 *  @date       2026.04.02
 * 
 *  File content:
 *      - GPIO Init/Deinit functions implementation
 *      - Definitions for GPIO function assertions check
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#include "bmcu_gpio.h"
#include "bmcu_def.h"

#define IS_GPIO_PIN(__VALUE__)                  ((__VALUE__) != 0U)
#define IS_GPIO_MODE(__VALUE__)                 (((__VALUE__) == GPIO_MODE_INPUT) || \
                                                 ((__VALUE__) == GPIO_MODE_OUTPUT))

/**
 * @brief Deinitializes pin set on the dedicated GPIO port.
 * @param GPIOx The GPIO instance.
 * @param GPIO_InitStruct The pointer to GPIO_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The GPIO instance have been deinitialized.
 *         - ERROR: The GPIO instance have not been deinitialized.
 */
ErrorStatus GPIO_DeInit(GPIO_TypeDef *GPIOx, GPIO_InitStruct_TypeDef *GPIO_InitStruct)
{
    ErrorStatus status = SUCCESS;
  
    /* Check parameters */
    assert(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert(IS_GPIO_PIN(GPIO_InitStruct->PinMask));

    /* Disable interrupt generation */
    GPIO_SetPinIT(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_DISABLE);
    /* Clear interrupt state */
    GPIO_ClearPinIT(GPIOx, GPIO_InitStruct->PinMask);
    /* Unmask GPIO interrupt */
    GPIO_SetPinITMsk(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_UNMASKED);
    /* Interrupt as level sensitive */
    GPIO_SetPinITType(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_TYPE_LEVEL);
    /* Low interrupt polarity */
    GPIO_SetPinITPolarity(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_POL_LOW);
    /* No debounce logic */
    GPIO_SetPinDebounce(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_DEBOUNCE_DISABLE);
    /* Disable synchronization for level-sensitive interrupts */
    GPIO_SetSyncIT(GPIOx, GPIO_IT_SYNC_DISABLE);
    /* Single edge sensitive interrupts */
    GPIO_SetPinBothEdgeIT(GPIOx, GPIO_InitStruct->PinMask, GPIO_IT_BOTHEDGE_DISABLE);

    /* Configure pins as INPUT */
    GPIO_SetPinMode(GPIOx, GPIO_InitStruct->PinMask, GPIO_MODE_INPUT);
    /* Set pins to low level */
    GPIO_ResetOutputPin(GPIOx, GPIO_InitStruct->PinMask);
    
    return status;
}

/**
 * @brief Initializes a GPIO instance.
 * @note GPIO_InitStruct structure should be initialized prior to calling this function.
 * @param GPIOx The GPIO instance.
 * @param GPIO_InitStruct The pointer to GPIO_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The GPIO instance have been initialized.
 *         - ERROR: The GPIO instance have not been initialized.
 */
ErrorStatus GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitStruct_TypeDef *GPIO_InitStruct)
{
    ErrorStatus status = SUCCESS;
  
    /* Check parameters */
    assert(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert(IS_GPIO_PIN(GPIO_InitStruct->PinMask));
    assert(IS_GPIO_MODE(GPIO_InitStruct->Mode));

    GPIO_SetPinMode(GPIOx, GPIO_InitStruct->PinMask, GPIO_InitStruct->Mode);

    return status;
}

/**
 * @brief Sets each field of GPIO_InitStruct_TypeDef structure to default value.
 * @param GPIO_InitStruct The pointer to GPIO_InitStruct_TypeDef structure.
 */
void GPIO_StructInit(GPIO_InitStruct_TypeDef *GPIO_InitStruct)
{
    /* Set GPIO_InitStruct fields to default values */
    GPIO_InitStruct->PinMask = GPIO_PIN_ALL;
    GPIO_InitStruct->Mode = GPIO_MODE_INPUT;
}

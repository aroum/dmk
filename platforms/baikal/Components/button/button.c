/**
 * *****************************************************************************
 *  @file       BSP/Components/button/button.c
 *  @author     Baikal electronics SDK team
 *  @brief      User button driver source file
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

/* Includes ------------------------------------------------------------------*/
#if defined(EVU_BA_1_2)
#include "../../EVU_BA_1_2/bsp.h"
#elif defined(EVU_BA_2_0)
#include "../../EVU_BA_2_0/bsp.h"
#elif defined(EVU_BA_2_1)
#include "../../EVU_BA_2_1/bsp.h"
#elif defined(EVU_BA_2_3)
#include "../../EVU_BA_2_3/bsp.h"
#elif defined(EVU_BA_2_5)
#include "../../EVU_BA_2_5/bsp.h"
#elif defined(EVU_LI_2_0)
#include "../../EVU_LI_2_0/bsp.h"
#else
#error "The selected development board does not support user button"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void bsp_btn_init(void) {
    CRU_PIN_InitStruct_TypeDef CRU_PIN_InitStruct;
    GPIO_InitStruct_TypeDef GPIO_InitStruct;

    /* 1. Initialize CRU settings of the user button pin */
    CRU_PIN_StructInit(&CRU_PIN_InitStruct);
    CRU_PIN_InitStruct.Port = BSP_BTN_CRU_PORT;
    CRU_PIN_InitStruct.Pin = BSP_BTN_CRU_PIN;
    CRU_PIN_InitStruct.Pull = CRU_PIN_PULL_UP;
    CRU_PIN_InitStruct.InputCtrl = ENABLE;
    CRU_PIN_InitStruct.DriveStrength = CRU_PIN_DRIVE_STRENGTH_0;
    CRU_PIN_InitStruct.Alternate = CRU_PIN_AF_0;
    CRU_PIN_Init(&CRU_PIN_InitStruct);

    /* 2. Initialize GPIO settings of the user button pin */
#if defined(BMCU_U)
    /* Enable clock of the user button GPIO port */
    BSP_BTN_CLK_EN_FN(BSP_BTN_CLK_PERIPH);
#endif

    /* Configure the user button pin as input */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.PinMask = BSP_BTN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_Init(BSP_BTN_GPIO_PORT, &GPIO_InitStruct);
}

void bsp_btn_deinit(void) {
    CRU_PIN_InitStruct_TypeDef CRU_PIN_InitStruct;
    GPIO_InitStruct_TypeDef GPIO_InitStruct;

    /* 1. Set default CRU settings of the user button pin */
    CRU_PIN_StructInit(&CRU_PIN_InitStruct);
    CRU_PIN_InitStruct.Port = BSP_BTN_CRU_PORT;
    CRU_PIN_InitStruct.Pin = BSP_BTN_CRU_PIN;
    CRU_PIN_Init(&CRU_PIN_InitStruct);

    /* 2. Set default GPIO settings of the user button pin */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.PinMask = BSP_BTN_GPIO_PIN;
    GPIO_DeInit(BSP_BTN_GPIO_PORT, &GPIO_InitStruct);
}

uint32_t bsp_btn_ispressed(void) {
    return (((GPIO_ReadInputPort(BSP_BTN_GPIO_PORT) & BSP_BTN_GPIO_PIN) == 0UL) ? 1UL : 0UL);
}

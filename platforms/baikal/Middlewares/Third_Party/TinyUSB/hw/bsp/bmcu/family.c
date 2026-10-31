/**
 * *****************************************************************************
 *  @file       family.c
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB driver
 *  @version    2.4.0
 *  @date       2026-10-06
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#include "board.h"
#include "basis_core_riscv.h"
#if defined (BE_U1000)
#include "bmcu_cru.h"
#endif

size_t board_get_unique_id(uint8_t id[], size_t max_len)
{
    (void)max_len;

    /* Fixed serial string is 01234567889ABCDEF */
    uint32_t* uid32 = (uint32_t*)(uintptr_t)id;
    uid32[0] = 0x67452301;
    uid32[1] = 0xEFCDAB89;

    return 8UL;
}

void board_vbus_init(void)
{
#if defined (BE_U1000)
    CRU->USBCR |= CRU_USBCR_DRVVBUSVAL;

    CRU_PIN_InitStruct_TypeDef CRU_PIN_InitStruct;
    CRU_PIN_StructInit(&CRU_PIN_InitStruct);
    CRU_PIN_InitStruct.Port = USB_DRVVBUS_CRU_PORT;
    CRU_PIN_InitStruct.Pin = USB_DRVVBUS_CRU_PIN;
    CRU_PIN_InitStruct.Pull = CRU_PIN_PULL_NO;
    CRU_PIN_InitStruct.Alternate = USB_DRVVBUS_CRU_PIN_AF;
    CRU_PIN_Init(&CRU_PIN_InitStruct);
#endif
}

uint32_t tusb_time_millis_api(void)
{
    return (uint32_t)((__get_time_u() / 1000U) & 0xFFFFFFFFUL);
}

/**
 * *****************************************************************************
 *  @file       board.h
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB driver
 *  @version    2.4.0
 *  @date       2026-10-06
 * *****************************************************************************
 *  @copyright Copyright (c) 2026 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BOARD_H
#define __BOARD_H

#include "bmcu_common.h"

#ifndef USB_INT_LEVEL
#define USB_INT_LEVEL               (1U)    /*!< USB interrupt level */
#endif /* USB_INT_LEVEL */

#ifndef USB_INT_PRIORITY
#define USB_INT_PRIORITY            (1U)    /*!< USB interrupt priority */
#endif /* USB_INT_PRIORITY */

#ifndef USB_DMA_INT_LEVEL
#define USB_DMA_INT_LEVEL           (1U)    /*!< USB DMA interrupt level */
#endif /* USB_DMA_INT_LEVEL */

#ifndef USB_DMA_INT_PRIORITY
#define USB_DMA_INT_PRIORITY        (1U)    /*!< USB DMA interrupt priority */
#endif /* USB_DMA_INT_PRIORITY */

#if defined (__BSP_H) && defined (__HCD_BMCU_H)
#define USB_DRVVBUS_CRU_PIN_AF      BSP_USB_DRVVBUS_CRU_PIN_AF
#define USB_DRVVBUS_CRU_PORT        BSP_USB_DRVVBUS_CRU_PORT
#define USB_DRVVBUS_CRU_PIN         BSP_USB_DRVVBUS_CRU_PIN
#else
#define USB_DRVVBUS_CRU_PIN_AF      (CRU_PIN_AF_5)
#define USB_DRVVBUS_CRU_PORT        (CRU_PORT_C)
#define USB_DRVVBUS_CRU_PIN         (CRU_PIN_15)
#endif /* __BSP_H */

void board_vbus_init(void);

#endif /* __BOARD_H */

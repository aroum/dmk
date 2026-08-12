/**
 ******************************************************************************
 * @file    MDR32FxQI_usb_handlers.h
 * @author  Milandr Application Team
 * @version V2.0.0i
 * @date    10/03/2022
 * @brief   This file contains all the functions prototypes for the USB handlers.
 ******************************************************************************
 * <br><br>
 *
 * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
 * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDR32FxQI_USB_HANDLERS_H
#define __MDR32FxQI_USB_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_usb_default_handlers.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
 * @{
 */

/** @addtogroup  USB_Virtual_COM_Port_Echo USB Virtual COM Port Echo
 * @{
 */

/* Configuration file MDR32FxQI_config.h should be included before */
#ifndef __MDR32FxQI_CONFIG_H
#error "Configuration file MDR32FxQI_config.h should be included before"
#endif /* #ifndef __MDR32FxQI_CONFIG_H */

#include "app_usb_hid.h"

#undef USB_DEVICE_HANDLE_RESET
#define USB_DEVICE_HANDLE_RESET USB_HID_Reset()

#undef USB_DEVICE_HANDLE_GET_DESCRIPTOR
#define USB_DEVICE_HANDLE_GET_DESCRIPTOR(wVALUE, wINDEX, wLENGTH) USB_HID_GetDescriptor(wVALUE, wINDEX, wLENGTH)

#undef USB_DEVICE_HANDLE_CLASS_REQUEST
#define USB_DEVICE_HANDLE_CLASS_REQUEST USB_HID_ClassRequest()

#undef USB_DEVICE_HANDLE_SET_CONFIGURATION
#define USB_DEVICE_HANDLE_SET_CONFIGURATION(wVALUE) USB_HID_SetConfiguration(wVALUE)

/** @defgroup USB_Virtual_COM_Port_Echo_Handler_Functions USB Virtual COM Port Echo Handler Functions
 * @{
 */

USB_Result USB_CDC_RecieveData(uint8_t *Buffer, uint32_t Length);

#ifdef USB_CDC_LINE_CODING_SUPPORTED
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef *DATA);
USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef *DATA);
#endif /* USB_CDC_LINE_CODING_SUPPORTED */

#ifdef USB_VCOM_SYNC
USB_Result USB_CDC_DataSent(void);
#endif /* USB_VCOM_SYNC */

#ifdef USB_DEBUG_PROTO
USB_Result USB_DeviceSetupPacket_Debug(USB_EP_TypeDef EPx, const USB_SetupPacket_TypeDef *USB_SetupPacket);
#endif /* USB_DEBUG_PROTO */

/** @} */ /* End of group USB_Virtual_COM_Port_Echo_Handler_Functions */

/** @} */ /* End of group USB_Virtual_COM_Port_Echo */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_USB_HANDLERS_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
 *
 * END OF FILE MDR32FxQI_usb_handlers.h */

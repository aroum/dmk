#include "board_usb.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_usb.h"
#include "MDR32FxQI_usb_handlers.h"

static USB_Clock_TypeDef USB_Clock_InitStruct;
static USB_DeviceBUSParam_TypeDef USB_DeviceBUSParam;

void Board_USB_Init(bool freertos_irq_priority)
{
  RST_CLK_PCLKcmd(RST_CLK_PCLK_USB, ENABLE);

  USB_Clock_InitStruct.USB_USBC1_Source = USB_C1HSEdiv1;
  USB_Clock_InitStruct.USB_PLLUSBMUL = USB_PLLUSBMUL6;

  USB_DeviceBUSParam.MODE = USB_SC_SCFSP_Full;
  USB_DeviceBUSParam.SPEED = USB_SC_SCFSR_12Mb;
  USB_DeviceBUSParam.PULL = USB_HSCR_DP_PULLUP_Set;

  USB_DeviceInit(&USB_Clock_InitStruct, &USB_DeviceBUSParam);
  USB_SetSIM(USB_SIS_Msk);
  USB_DevicePowerOn();

#ifdef USB_INT_HANDLE_REQUIRED
  if (freertos_irq_priority) {
    NVIC_SetPriority(USB_IRQn, 6);
  }
  NVIC_EnableIRQ(USB_IRQn);
#endif

  USB_DEVICE_HANDLE_RESET;
}

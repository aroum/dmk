#include "app_usb_hid.h"

// Undefine conflicting definitions before including TinyUSB headers
#undef KEYBOARD_MODIFIER_LEFTCTRL
#undef KEYBOARD_MODIFIER_LEFTSHIFT
#undef KEYBOARD_MODIFIER_LEFTALT
#undef KEYBOARD_MODIFIER_LEFTGUI
#undef KEYBOARD_MODIFIER_RIGHTCTRL
#undef KEYBOARD_MODIFIER_RIGHTSHIFT
#undef KEYBOARD_MODIFIER_RIGHTALT
#undef KEYBOARD_MODIFIER_RIGHTGUI

#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

#if defined(MCU_rp2350)
#include "hardware/irq.h"
#elif defined(MCU_nrf52840)
#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrfx_power.h"

void USBD_IRQHandler(void) {
    tud_int_handler(0);
}

static void power_event_handler(nrfx_power_usb_evt_t event) {
    extern void tusb_hal_nrf_power_event(uint32_t event);
    tusb_hal_nrf_power_event((uint32_t)event);
}
#elif defined(MCU_milandr)
#include "MDR32FxQI_config.h"

void USB_IRQHandler(void) {
    tud_int_handler(0);
}
#endif

static TaskHandle_t s_usb_task_handle = NULL;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
static StaticTask_t s_usb_task_tcb;
static StackType_t s_usb_task_stack[512];
#endif

void tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
    (void)rhport;
    (void)eventid;
    if (s_usb_task_handle == NULL) {
        return;
    }
    if (in_isr) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(s_usb_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        xTaskNotifyGive(s_usb_task_handle);
    }
}

static void usb_device_task(void *pvParameters) {
    (void)pvParameters;
    s_usb_task_handle = xTaskGetCurrentTaskHandle();
    while (1) {
        tud_task();
#ifdef VIAL
        extern void vial_flush_pending_report(void);
        vial_flush_pending_report();
#endif
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    }
}

USB_Result USB_HID_Init(void) {
#if defined(MCU_nrf52840)
    NVIC_SetPriority(USBD_IRQn, 2);

    ret_code_t clk_ret = nrf_drv_clock_init();
    if (clk_ret == NRF_SUCCESS || clk_ret == NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        nrf_drv_clock_lfclk_request(NULL);
    }

    const nrfx_power_config_t pwr_cfg = {0};
    nrfx_power_init(&pwr_cfg);

    const nrfx_power_usbevt_config_t usbevt_cfg = {.handler = power_event_handler};
    nrfx_power_usbevt_init(&usbevt_cfg);
    nrfx_power_usbevt_enable();

    extern void tusb_hal_nrf_power_event(uint32_t event);
    uint32_t usb_reg = NRF_POWER->USBREGSTATUS;
    if (usb_reg & POWER_USBREGSTATUS_VBUSDETECT_Msk) {
        tusb_hal_nrf_power_event(0); // USB_EVT_DETECTED
    }
    if (usb_reg & POWER_USBREGSTATUS_OUTPUTRDY_Msk) {
        tusb_hal_nrf_power_event(2); // USB_EVT_READY
    }
#endif

    tusb_init();
#if defined(MCU_rp2350)
    irq_set_priority(USBCTRL_IRQ, 0x80);
#elif defined(MCU_milandr)
    NVIC_SetPriority(USB_IRQn, 6);
#endif
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    s_usb_task_handle = xTaskCreateStatic(usb_device_task, "usbd", 512, NULL, 3, s_usb_task_stack, &s_usb_task_tcb);
#else
    xTaskCreate(usb_device_task, "usbd", 512, NULL, 3, &s_usb_task_handle);
#endif
    return USB_SUCCESS;
}

USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_keyboard_report(1, report->Modifier, (uint8_t *)report->Keycodes)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_SendConsumerReport(uint16_t usage) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_report(2, &usage, sizeof(usage))) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_SendMouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_mouse_report(3, buttons, x, y, wheel, pan)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat,
                                     uint32_t buttons) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_gamepad_report(4, x, y, z, rz, rx, ry, hat, buttons)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_Reset(void) {
    return USB_SUCCESS;
}

USB_Result USB_HID_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH) {
    (void)wVALUE;
    (void)wINDEX;
    (void)wLENGTH;
    return USB_SUCCESS;
}

USB_Result USB_HID_ClassRequest(void) {
    return USB_SUCCESS;
}

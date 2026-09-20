#include "app_usb_hid.h"
#include "config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "task_internal.h"
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

static void usb_device_task(void *param) {
    (void)param;
    s_usb_task_handle = xTaskGetCurrentTaskHandle();
    while (1) {
        tud_task();
#if defined(MIDI_USB) && !defined(MCU_milandr)
        extern void tud_midi_rx_cb(uint8_t itf);
        while (tud_midi_available()) {
            tud_midi_rx_cb(0);
        }
#endif
#ifdef VIAL
        extern void vial_flush_pending_report(void);
        vial_flush_pending_report();
#endif
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2));
    }
}

bool USB_HID_Init(void) {
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
    s_usb_task_handle = xTaskCreateStatic(usb_device_task, "usbd", 512, NULL, TASK_PRIO_USBD, s_usb_task_stack, &s_usb_task_tcb);
#else
    xTaskCreate(usb_device_task, "usbd", 512, NULL, TASK_PRIO_USBD, &s_usb_task_handle);
#endif
    return (s_usb_task_handle != NULL);
}

bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    if (!tud_mounted() || tud_suspended()) {
        return false;
    }
    int timeout = 5; // Reduced from 50ms to 5ms to prevent blocking keyboard scanning
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_keyboard_report(1, report->Modifier, (uint8_t *)report->Keycodes)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return false;
}

bool USB_HID_SendConsumerReport(uint16_t usage) {
    if (!tud_mounted() || tud_suspended()) {
        return false;
    }
    int timeout = 5; // Reduced from 50ms to 5ms
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_report(2, &usage, sizeof(usage))) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return false;
}

bool USB_HID_SendMouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan) {
    if (!tud_mounted() || tud_suspended()) {
        return false;
    }
    int timeout = 2; // Reduced from 50ms to 2ms for non-blocking mouse updates
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_mouse_report(3, buttons, x, y, wheel, pan)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return false;
}

bool USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat,
                               uint32_t buttons) {
    if (!tud_mounted() || tud_suspended()) {
        return false;
    }
    int timeout = 5; // Reduced from 50ms to 5ms
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_gamepad_report(4, x, y, z, rz, rx, ry, hat, buttons)) {
                if (s_usb_task_handle) {
                    xTaskNotifyGive(s_usb_task_handle);
                }
                return true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return false;
}

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
#if !defined(MCU_baikal)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
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
        extern void vial_eeprom_task(void);
        vial_flush_pending_report();
        vial_eeprom_task();
#endif
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2));
    }
}

bool USB_HID_Init(void) {
#if defined(MCU_nrf52840)
    NVIC_SetPriority(USBD_IRQn, 5);

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

#if defined(MCU_baikal)
    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(0, &dev_init);
#else
    tusb_init();
#endif
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

static bool usb_hid_wait_ready(int timeout_ms) {
    if (!tud_mounted() || tud_suspended()) {
        return false;
    }
    while (timeout_ms > 0) {
        if (tud_hid_ready()) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout_ms--;
    }
    return tud_hid_ready();
}

static bool usb_hid_complete_send(bool sent) {
    if (sent && s_usb_task_handle) {
        xTaskNotifyGive(s_usb_task_handle);
    }
    return sent;
}

bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    if (!usb_hid_wait_ready(5)) {
        return false;
    }
    return usb_hid_complete_send(tud_hid_keyboard_report(1, report->Modifier, (uint8_t *)report->Keycodes));
}

bool USB_HID_SendConsumerReport(uint16_t usage) {
    if (!usb_hid_wait_ready(5)) {
        return false;
    }
    return usb_hid_complete_send(tud_hid_report(2, &usage, sizeof(usage)));
}

bool USB_HID_SendMouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan) {
    if (!usb_hid_wait_ready(2)) {
        return false;
    }
    return usb_hid_complete_send(tud_hid_mouse_report(3, buttons, x, y, wheel, pan));
}

bool USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat,
                               uint32_t buttons) {
    if (!usb_hid_wait_ready(5)) {
        return false;
    }
    return usb_hid_complete_send(tud_hid_gamepad_report(4, x, y, z, rz, rx, ry, hat, buttons));
}

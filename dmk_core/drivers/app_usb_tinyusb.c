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
#endif

static void usb_device_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        tud_task();
#ifdef VIAL
        extern void vial_flush_pending_report(void);
        vial_flush_pending_report();
#endif
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

USB_Result USB_HID_Init(void) {
    tusb_init();
#if defined(MCU_rp2350)
    irq_set_priority(USBCTRL_IRQ, 0x80);
#endif
    xTaskCreate(usb_device_task, "usbd", 512, NULL, 3, NULL);
    return USB_SUCCESS;
}

USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_keyboard_report(1, report->Modifier, (uint8_t *)report->Keycodes)) {
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
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat, uint32_t buttons) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_gamepad_report(4, x, y, z, rz, rx, ry, hat, buttons)) {
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

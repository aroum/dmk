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

static void usb_device_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

USB_Result USB_HID_Init(void) {
    tusb_init();
    xTaskCreate(usb_device_task, "usbd", 512, NULL, 3, NULL);
    return USB_SUCCESS;
}

USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
#ifdef EXTRAKEY_ENABLE
            if (tud_hid_keyboard_report(1, report->Modifier, (uint8_t *)report->Keycodes)) {
                return USB_SUCCESS;
            }
#else
            if (tud_hid_keyboard_report(0, report->Modifier, (uint8_t *)report->Keycodes)) {
                return USB_SUCCESS;
            }
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
}

USB_Result USB_HID_SendConsumerReport(uint16_t usage) {
#ifdef EXTRAKEY_ENABLE
    int timeout = 50; // 50ms timeout
    while (timeout > 0) {
        if (tud_hid_ready()) {
            if (tud_hid_report(0, 2, &usage, sizeof(usage))) {
                return USB_SUCCESS;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    return USB_ERR_BUSY;
#else
    (void)usage;
    return USB_SUCCESS;
#endif
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

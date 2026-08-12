// USB HID keyboard driver for nRF52840 using nRF5 SDK app_usbd_hid_kbd
#include "app_usb_hid.h"
#include "hal_gpio.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_kbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"

#include "FreeRTOS.h"
#include "config.h"
#include "task.h"

#ifdef VIAL
#include "app_usbd_hid_generic.h"
#include "vial.h"
#endif

extern void led_set_hid_state(uint8_t state);

// Forward declaration of the keyboard event handler
static void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);

// HID keyboard instance on interface 0, endpoint IN1
APP_USBD_HID_KBD_GLOBAL_DEF(m_app_hid_kbd, 0, NRF_DRV_USBD_EPIN1, hid_kbd_user_ev_handler, APP_USBD_HID_SUBCLASS_BOOT);

#ifdef VIAL
// Define the Vial Raw HID report descriptor (32 bytes IN, 32 bytes OUT)
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(vial_desc, {
                                                         0x06, 0x60, 0xFF, // Usage Page (Vendor Defined 0xFF60)
                                                         0x09, 0x61,       // Usage (Vendor Defined 0x61)
                                                         0xA1, 0x01,       // Collection (Application)
                                                         0x09, 0x62,       // Usage (Vendor Defined 0x62)
                                                         0x15, 0x00,       // Logical Minimum (0)
                                                         0x26, 0xFF, 0x00, // Logical Maximum (255)
                                                         0x75, 0x08,       // Report Size (8)
                                                         0x95, 0x20,       // Report Count (32)
                                                         0x81, 0x02,       // Input (Data, Var, Abs)
                                                         0x09, 0x63,       // Usage (Vendor Defined 0x63)
                                                         0x15, 0x00,       // Logical Minimum (0)
                                                         0x26, 0xFF, 0x00, // Logical Maximum (255)
                                                         0x75, 0x08,       // Report Size (8)
                                                         0x95, 0x20,       // Report Count (32)
                                                         0x91, 0x02,       // Output (Data, Var, Abs)
                                                         0xC0              // End Collection
                                                     });

static const app_usbd_hid_subclass_desc_t *vial_reps[] = {&vial_desc};

#define VIAL_ENDPOINT_LIST() (NRF_DRV_USBD_EPIN2, NRF_DRV_USBD_EPOUT2)

static void hid_vial_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);

APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_vial,
                                1, // Interface 1
                                hid_vial_user_ev_handler, VIAL_ENDPOINT_LIST(), vial_reps,
                                2,  // IN Queue size
                                32, // OUT max report size
                                0,  // Feature max report size
                                APP_USBD_HID_SUBCLASS_NONE, APP_USBD_HID_PROTO_GENERIC);

static void hid_vial_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event) {
    switch (event) {
    case APP_USBD_HID_USER_EVT_OUT_REPORT_READY: {
        size_t size = 0;
        const void *p_report = app_usbd_hid_generic_out_report_get(&m_app_hid_vial, &size);
        if (p_report && size >= 32) {
            uint8_t response[32];
            vial_process_packet(p_report, response);
            app_usbd_hid_generic_in_report_set(&m_app_hid_vial, response, 32);
        }
        break;
    }
    default:
        break;
    }
}
#endif

// Tracks whether the USB stack is ready to accept reports
static volatile bool s_usb_ready = false;
extern volatile bool usb_mounted;

// Tracks which keys were previously pressed to send release events
static uint8_t s_prev_keys[6] = {0};

static bool key_array_contains(const uint8_t *arr, uint8_t val) {
    for (int i = 0; i < 6; i++) {
        if (arr[i] == val) {
            return true;
        }
    }
    return false;
}

static void update_leds(void) {
    uint8_t state = 0;
    if (app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_NUM_LOCK)) {
        state |= 0x01;
    }
    if (app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_CAPS_LOCK)) {
        state |= 0x02;
    }
    if (app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_SCROLL_LOCK)) {
        state |= 0x04;
    }
    if (app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_COMPOSE)) {
        state |= 0x08;
    }
    if (app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_KANA)) {
        state |= 0x10;
    }
    led_set_hid_state(state);
}

static void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event) {
    (void)p_inst;
    switch (event) {
    case APP_USBD_EVT_DRV_RESET:
        for (int i = 0; i < 6; i++) {
            s_prev_keys[i] = 0;
        }
        led_set_hid_state(0);
        break;
    case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
        update_leds();
        break;
    default:
        break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event) {
    switch (event) {
    case APP_USBD_EVT_DRV_SUSPEND:
        app_usbd_suspend_req();
        s_usb_ready = false;
        usb_mounted = false;
        for (int i = 0; i < 6; i++) {
            s_prev_keys[i] = 0;
        }
        led_set_hid_state(0);
        break;
    case APP_USBD_EVT_DRV_RESUME:
        s_usb_ready = true;
        break;
    case APP_USBD_EVT_STARTED:
        s_usb_ready = true;
        break;
    case APP_USBD_EVT_STOPPED:
        app_usbd_disable();
        s_usb_ready = false;
        usb_mounted = false;
        for (int i = 0; i < 6; i++) {
            s_prev_keys[i] = 0;
        }
        led_set_hid_state(0);
        break;
    case APP_USBD_EVT_POWER_DETECTED:
        if (!nrf_drv_usbd_is_enabled()) {
            app_usbd_enable();
        }
        break;
    case APP_USBD_EVT_POWER_REMOVED:
        app_usbd_stop();
        usb_mounted = false;
        for (int i = 0; i < 6; i++) {
            s_prev_keys[i] = 0;
        }
        led_set_hid_state(0);
        break;
    case APP_USBD_EVT_POWER_READY:
        app_usbd_start();
        break;
    case APP_USBD_EVT_STATE_CHANGED:
        if (app_usbd_core_state_get() == APP_USBD_STATE_Configured) {
            usb_mounted = true;
        } else {
            usb_mounted = false;
        }
        break;
    default:
        break;
    }
}

static TaskHandle_t m_usbd_thread = NULL;

static void usb_new_event_isr_handler(app_usbd_internal_evt_t const *const p_event, bool queued) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    (void)p_event;
    (void)queued;
    if (m_usbd_thread != NULL) {
        vTaskNotifyGiveFromISR(m_usbd_thread, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// FreeRTOS task that drives the USB event queue
static void usb_device_task(void *pvParameters) {
    (void)pvParameters;
    // Set the first event to make sure that USB queue is processed after it is started
    xTaskNotifyGive(xTaskGetCurrentTaskHandle());
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        while (app_usbd_event_queue_process()) { /* drain queue */
        }
    }
}

USB_Result USB_HID_Init(void) {
    ret_code_t ret;

    ret = nrf_drv_power_init(NULL);
    if (ret != NRF_SUCCESS && ret != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        return USB_ERROR;
    }

    ret = nrf_drv_clock_init();
    if (ret != NRF_SUCCESS && ret != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        return USB_ERROR;
    }

    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running()) { /* wait for LFCLK */
    }

    static const app_usbd_config_t usbd_config = {
        .ev_isr_handler = usb_new_event_isr_handler,
        .ev_state_proc = usbd_user_ev_handler,
    };

    ret = app_usbd_init(&usbd_config);
    if (ret != NRF_SUCCESS) {
        return USB_ERROR;
    }

    app_usbd_class_inst_t const *kbd_inst = app_usbd_hid_kbd_class_inst_get(&m_app_hid_kbd);
    ret = app_usbd_class_append(kbd_inst);
    if (ret != NRF_SUCCESS) {
        return USB_ERROR;
    }

#ifdef VIAL
    app_usbd_class_inst_t const *vial_inst = app_usbd_hid_generic_class_inst_get(&m_app_hid_vial);
    ret = app_usbd_class_append(vial_inst);
    if (ret != NRF_SUCCESS) {
        return USB_ERROR;
    }
#endif

    ret = app_usbd_power_events_enable();
    if (ret != NRF_SUCCESS) {
        return USB_ERROR;
    }

    xTaskCreate(usb_device_task, "usbd", 512, NULL, 3, &m_usbd_thread);

    return USB_SUCCESS;
}

USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    if (!usb_mounted || !s_usb_ready) {
        for (int i = 0; i < 6; i++) {
            s_prev_keys[i] = 0;
        }
        return USB_SUCCESS;
    }

    ret_code_t err_code;

    // Send modifier keys
    uint8_t mod = report->Modifier;
    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_LEFT_CTRL,
                                                   (mod & KEYBOARD_MODIFIER_LEFTCTRL) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_LEFT_SHIFT,
                                                   (mod & KEYBOARD_MODIFIER_LEFTSHIFT) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_LEFT_ALT,
                                                   (mod & KEYBOARD_MODIFIER_LEFTALT) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_LEFT_UI,
                                                   (mod & KEYBOARD_MODIFIER_LEFTGUI) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_RIGHT_CTRL,
                                                   (mod & KEYBOARD_MODIFIER_RIGHTCTRL) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_RIGHT_SHIFT,
                                                   (mod & KEYBOARD_MODIFIER_RIGHTSHIFT) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_RIGHT_ALT,
                                                   (mod & KEYBOARD_MODIFIER_RIGHTALT) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    err_code = app_usbd_hid_kbd_modifier_state_set(&m_app_hid_kbd, APP_USBD_HID_KBD_MODIFIER_RIGHT_UI,
                                                   (mod & KEYBOARD_MODIFIER_RIGHTGUI) != 0);
    if (err_code != NRF_SUCCESS)
        goto blink_err;

    // Release keys that are no longer pressed
    for (int i = 0; i < 6; i++) {
        uint8_t prev_key = s_prev_keys[i];
        if (prev_key != 0 && !key_array_contains(report->Keycodes, prev_key)) {
            err_code = app_usbd_hid_kbd_key_control(&m_app_hid_kbd, (app_usbd_hid_kbd_codes_t)prev_key, false);
            if (err_code != NRF_SUCCESS)
                goto blink_err;
        }
    }

    // Press keys that are newly pressed
    for (int i = 0; i < 6; i++) {
        uint8_t new_key = report->Keycodes[i];
        if (new_key != 0 && !key_array_contains(s_prev_keys, new_key)) {
            err_code = app_usbd_hid_kbd_key_control(&m_app_hid_kbd, (app_usbd_hid_kbd_codes_t)new_key, true);
            if (err_code != NRF_SUCCESS)
                goto blink_err;
        }
    }

    // Update previous keys state
    for (int i = 0; i < 6; i++) {
        s_prev_keys[i] = report->Keycodes[i];
    }

    return USB_SUCCESS;

blink_err:
    return USB_ERROR;
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

USB_Result USB_HID_SendConsumerReport(uint16_t usage) {
    (void)usage;
    return USB_SUCCESS;
}

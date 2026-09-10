/**
 * @file    app_usb_hid.c
 * @brief   USB HID Keyboard driver implementation using Milandr SPL.
 */

#include "app_usb_hid.h"
#include "config.h"
#include "led.h"
#include <stdbool.h>
#include <string.h>

#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif

#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x4010
#endif

#ifdef VIAL
#include "vial.h"
static uint8_t vial_rx_buffer[32];
static uint8_t vial_tx_buffer[32];
static volatile bool vial_tx_busy = false;

static USB_Result VIAL_OnDataReceived(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length);
static USB_Result VIAL_OnDataSent(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length);
#endif

/* Context and state management */
static volatile USB_Result USB_HID_SendDataStatus = USB_SUCCESS;
static uint8_t USB_HID_IdleRate = 0;
static uint8_t USB_HID_Protocol = 1; /* 0 = Boot, 1 = Report */

static const uint8_t Usb_HID_Report_Descriptor[] = {
    0x05, 0x01, /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06, /* USAGE (Keyboard) */
    0xA1, 0x01, /* COLLECTION (Application) */
    0x85, 0x01, /*   REPORT_ID (1) */
    0x05, 0x07, /*   USAGE_PAGE (Keyboard) */
    0x19, 0xE0, /*   USAGE_MINIMUM (224 / Left Control) */
    0x29, 0xE7, /*   USAGE_MAXIMUM (231 / Right GUI) */
    0x15, 0x00, /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01, /*   LOGICAL_MAXIMUM (1) */
    0x75, 0x01, /*   REPORT_SIZE (1) */
    0x95, 0x08, /*   REPORT_COUNT (8) */
    0x81, 0x02, /*   INPUT (Data,Var,Abs) - Modifier byte */
    0x95, 0x01, /*   REPORT_COUNT (1) */
    0x75, 0x08, /*   REPORT_SIZE (8) */
    0x81, 0x03, /*   INPUT (Constant) - Reserved byte */
    0x95, 0x05, /*   REPORT_COUNT (5) */
    0x75, 0x01, /*   REPORT_SIZE (1) */
    0x05, 0x08, /*   USAGE_PAGE (LEDs) */
    0x19, 0x01, /*   USAGE_MINIMUM (Num Lock) */
    0x29, 0x05, /*   USAGE_MAXIMUM (Kana) */
    0x91, 0x02, /*   OUTPUT (Data,Var,Abs) - LED report */
    0x95, 0x01, /*   REPORT_COUNT (1) */
    0x75, 0x03, /*   REPORT_SIZE (3) */
    0x91, 0x03, /*   OUTPUT (Constant) - LED padding */
    0x95, 0x06, /*   REPORT_COUNT (6) */
    0x75, 0x08, /*   REPORT_SIZE (8) */
    0x15, 0x00, /*   LOGICAL_MINIMUM (0) */
    0x25, 0xFF, /*   LOGICAL_MAXIMUM (255) */
    0x05, 0x07, /*   USAGE_PAGE (Keyboard) */
    0x19, 0x00, /*   USAGE_MINIMUM (0) */
    0x29, 0xFF, /*   USAGE_MAXIMUM (255) */
    0x81, 0x00, /*   INPUT (Data,Ary,Abs) - 6 keycodes */
    0xC0,       /* END_COLLECTION */

    0x05, 0x0C,       /* USAGE_PAGE (Consumer Page) */
    0x09, 0x01,       /* USAGE (Consumer Control) */
    0xA1, 0x01,       /* COLLECTION (Application) */
    0x85, 0x02,       /*   REPORT_ID (2) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x26, 0x3C, 0x02, /*   LOGICAL_MAXIMUM (572) */
    0x19, 0x00,       /*   USAGE_MINIMUM (Unassigned) */
    0x2A, 0x3C, 0x02, /*   USAGE_MAXIMUM (AC Format) */
    0x75, 0x10,       /*   REPORT_SIZE (16) */
    0x95, 0x01,       /*   REPORT_COUNT (1) */
    0x81, 0x00,       /*   INPUT (Data,Ary,Abs) */
    0xC0,             /* END_COLLECTION */

    0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x02,       /* USAGE (Mouse) */
    0xA1, 0x01,       /* COLLECTION (Application) */
    0x85, 0x03,       /*   REPORT_ID (3) */
    0x09, 0x01,       /*   USAGE (Pointer) */
    0xA1, 0x00,       /*   COLLECTION (Physical) */
    0x05, 0x09,       /*     USAGE_PAGE (Button) */
    0x19, 0x01,       /*     USAGE_MINIMUM (Button 1) */
    0x29, 0x05,       /*     USAGE_MAXIMUM (Button 5) */
    0x15, 0x00,       /*     LOGICAL_MINIMUM (0) */
    0x25, 0x01,       /*     LOGICAL_MAXIMUM (1) */
    0x75, 0x01,       /*     REPORT_SIZE (1) */
    0x95, 0x05,       /*     REPORT_COUNT (5) */
    0x81, 0x02,       /*     INPUT (Data,Var,Abs) - 5 buttons */
    0x75, 0x03,       /*     REPORT_SIZE (3) */
    0x95, 0x01,       /*     REPORT_COUNT (1) */
    0x81, 0x03,       /*     INPUT (Constant) - Padding */
    0x05, 0x01,       /*     USAGE_PAGE (Generic Desktop) */
    0x09, 0x30,       /*     USAGE (X) */
    0x09, 0x31,       /*     USAGE (Y) */
    0x15, 0x81,       /*     LOGICAL_MINIMUM (-127) */
    0x25, 0x7F,       /*     LOGICAL_MAXIMUM (127) */
    0x75, 0x08,       /*     REPORT_SIZE (8) */
    0x95, 0x02,       /*     REPORT_COUNT (2) */
    0x81, 0x06,       /*     INPUT (Data,Var,Rel) */
    0x09, 0x38,       /*     USAGE (Wheel) */
    0x15, 0x81,       /*     LOGICAL_MINIMUM (-127) */
    0x25, 0x7F,       /*     LOGICAL_MAXIMUM (127) */
    0x75, 0x08,       /*     REPORT_SIZE (8) */
    0x95, 0x01,       /*     REPORT_COUNT (1) */
    0x81, 0x06,       /*     INPUT (Data,Var,Rel) */
    0x05, 0x0C,       /*     USAGE_PAGE (Consumer) */
    0x0A, 0x38, 0x02, /*     USAGE (AC Pan) */
    0x15, 0x81,       /*     LOGICAL_MINIMUM (-127) */
    0x25, 0x7F,       /*     LOGICAL_MAXIMUM (127) */
    0x75, 0x08,       /*     REPORT_SIZE (8) */
    0x95, 0x01,       /*     REPORT_COUNT (1) */
    0x81, 0x06,       /*     INPUT (Data,Var,Rel) */
    0xC0,             /*   END_COLLECTION */
    0xC0,             /* END_COLLECTION */

    0x05, 0x01,       /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x05,       /* USAGE (Gamepad) */
    0xA1, 0x01,       /* COLLECTION (Application) */
    0x85, 0x04,       /*   REPORT_ID (4) */
    0x05, 0x01,       /*   USAGE_PAGE (Generic Desktop) */
    0x09, 0x30,       /*   USAGE (X) */
    0x09, 0x31,       /*   USAGE (Y) */
    0x09, 0x32,       /*   USAGE (Z) */
    0x09, 0x35,       /*   USAGE (Rz) */
    0x09, 0x33,       /*   USAGE (Rx) */
    0x09, 0x34,       /*   USAGE (Ry) */
    0x15, 0x81,       /*   LOGICAL_MINIMUM (-127) */
    0x25, 0x7F,       /*   LOGICAL_MAXIMUM (127) */
    0x95, 0x06,       /*   REPORT_COUNT (6) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x81, 0x02,       /*   INPUT (Data,Var,Abs) */
    0x05, 0x01,       /*   USAGE_PAGE (Generic Desktop) */
    0x09, 0x39,       /*   USAGE (Hat Switch) */
    0x15, 0x01,       /*   LOGICAL_MINIMUM (1) */
    0x25, 0x08,       /*   LOGICAL_MAXIMUM (8) */
    0x35, 0x00,       /*   PHYSICAL_MINIMUM (0) */
    0x46, 0x3B, 0x01, /*   PHYSICAL_MAXIMUM (315) */
    0x95, 0x01,       /*   REPORT_COUNT (1) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x81, 0x02,       /*   INPUT (Data,Var,Abs) */
    0x05, 0x09,       /*   USAGE_PAGE (Button) */
    0x19, 0x01,       /*   USAGE_MINIMUM (Button 1) */
    0x29, 0x20,       /*   USAGE_MAXIMUM (Button 32) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,       /*   LOGICAL_MAXIMUM (1) */
    0x95, 0x20,       /*   REPORT_COUNT (32) */
    0x75, 0x01,       /*   REPORT_SIZE (1) */
    0x81, 0x02,       /*   INPUT (Data,Var,Abs) */
    0xC0              /* END_COLLECTION */
};

/* Standard Device Descriptor */
static const uint8_t Usb_HID_Device_Descriptor[18] = {
    0x12, /* bLength */
    0x01, /* bDescriptorType (Device) */
    0x00,
    0x02, /* bcdUSB (2.0) */
    0x00, /* bDeviceClass (Defined at interface level) */
    0x00, /* bDeviceSubClass */
    0x00, /* bDeviceProtocol */
    64,   /* bMaxPacketSize0 */
#ifdef VIAL
    (uint8_t)(VIAL_VENDOR_ID & 0xFF),
    (uint8_t)((VIAL_VENDOR_ID >> 8) & 0xFF), /* idVendor */
    (uint8_t)(VIAL_PRODUCT_ID & 0xFF),
    (uint8_t)((VIAL_PRODUCT_ID >> 8) & 0xFF), /* idProduct */
#else
    0x83, 0x04, /* idVendor (Milandr 0x0483) */
    0x11, 0x57, /* idProduct (Custom HID keyboard 0x5711) */
#endif
    0x00,
    0x01, /* bcdDevice (1.0) */
    0x01, /* iManufacturer (String 1) */
    0x02, /* iProduct (String 2) */
    0x03, /* iSerialNumber (String 3) */
    0x01  /* bNumConfigurations */
};

#ifdef VIAL
/* Standard HID Vial Report Descriptor (34 bytes) */
static const uint8_t Usb_Vial_Report_Descriptor[] = {
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
};
#endif

#ifdef VIAL
#if defined(MIDI_USB)
#define VIAL_RX_EP USB_EP2
#define TOTAL_CONFIG_LEN 154
#define TOTAL_NUM_INTERFACES 4
#define MIDI_IF_AC 0x02
#define MIDI_IF_MS 0x03
#else
#define VIAL_RX_EP USB_EP3
#define TOTAL_CONFIG_LEN 66
#define TOTAL_NUM_INTERFACES 2
#endif
#else
#if defined(MIDI_USB)
#define TOTAL_CONFIG_LEN 122
#define TOTAL_NUM_INTERFACES 3
#define MIDI_IF_AC 0x01
#define MIDI_IF_MS 0x02
#else
#define TOTAL_CONFIG_LEN 34
#define TOTAL_NUM_INTERFACES 1
#endif
#endif

static const uint8_t Usb_HID_Configuration_Descriptor[] = {
    /* Configuration Descriptor (9 bytes) */
    0x09, 0x02, (uint8_t)(TOTAL_CONFIG_LEN & 0xFF), (uint8_t)((TOTAL_CONFIG_LEN >> 8) & 0xFF),
    TOTAL_NUM_INTERFACES, 0x01, 0x00, 0xA0, 50,

    /* Interface 0 Descriptor - Keyboard (9 bytes) */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,
    /* HID Descriptor (9 bytes) */
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22,
    (uint8_t)(sizeof(Usb_HID_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_HID_Report_Descriptor) >> 8) & 0xFF),
    /* Endpoint 1 IN Descriptor - Keyboard (7 bytes) */
    0x07, 0x05, 0x81, 0x03, 0x10, 0x00, 10,

#ifdef VIAL
    /* Interface 1 Descriptor - Vial Raw HID (9 bytes) */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    /* HID Descriptor for Vial (9 bytes) */
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22,
    (uint8_t)(sizeof(Usb_Vial_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_Vial_Report_Descriptor) >> 8) & 0xFF),
    /* Endpoint 2 IN Descriptor - Vial IN (7 bytes) */
    0x07, 0x05, 0x82, 0x03, 32, 0x00, 1,
    /* Endpoint OUT Descriptor - Vial OUT (7 bytes) */
#if defined(MIDI_USB)
    0x07, 0x05, 0x02, 0x03, 32, 0x00, 1,
#else
    0x07, 0x05, 0x03, 0x03, 32, 0x00, 1,
#endif
#endif

#ifdef MIDI_USB
    /* Interface Descriptor - Audio Control (9 bytes) */
    0x09, 0x04, MIDI_IF_AC, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    /* Class-Specific Audio Control Interface Descriptor (9 bytes) */
    0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, MIDI_IF_MS,

    /* Interface Descriptor - MIDI Streaming (9 bytes) */
    0x09, 0x04, MIDI_IF_MS, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00,
    /* Class-Specific MIDI Streaming Interface Descriptor (7 bytes) */
    0x07, 0x24, 0x01, 0x00, 0x01, 37, 0x00,
    /* MIDI IN Jack Descriptor (Embedded) (6 bytes) */
    0x06, 0x24, 0x02, 0x01, 0x01, 0x00,
    /* MIDI IN Jack Descriptor (External) (6 bytes) */
    0x06, 0x24, 0x02, 0x02, 0x02, 0x00,
    /* MIDI OUT Jack Descriptor (Embedded) (9 bytes) */
    0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00,
    /* MIDI OUT Jack Descriptor (External) (9 bytes) */
    0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00,
    /* Endpoint 3 OUT Descriptor - MIDI Bulk OUT (7 bytes) */
    0x07, 0x05, 0x03, 0x02, 64, 0x00, 0,
    /* Class-Specific MIDI Bulk OUT Endpoint Descriptor (5 bytes) */
    0x05, 0x25, 0x01, 0x01, 0x01,
    /* Endpoint 3 IN Descriptor - MIDI Bulk IN (7 bytes) */
    0x07, 0x05, 0x83, 0x02, 64, 0x00, 0,
    /* Class-Specific MIDI Bulk IN Endpoint Descriptor (5 bytes) */
    0x05, 0x25, 0x01, 0x01, 0x03
#endif
};

/* String Descriptor 0 (Language ID) */
static const uint8_t Usb_HID_String_LangID[4] = {
    0x04,      /* bLength */
    0x03,      /* bDescriptorType (String) */
    0x09, 0x04 /* wLANGID (US English 0x0409) */
};

/* String Descriptor 1 (Manufacturer) - "Milandr" in UTF-16LE */
static const uint8_t Usb_HID_String_Manuf[16] = {16, 0x03, 'M', 0, 'i', 0, 'l', 0, 'a', 0, 'n', 0, 'd', 0, 'r', 0};

/* String Descriptor 2 (Product) - "HID Keyboard" in UTF-16LE */
static const uint8_t Usb_HID_String_Prod[26] = {26, 0x03, 'H', 0,   'I', 0,   'D', 0,   ' ', 0,   'K', 0,   'e',
                                                0,  'y',  0,   'b', 0,   'o', 0,   'a', 0,   'r', 0,   'd', 0};

#ifdef VIAL
/* String Descriptor 3 (Serial) - "vial:f64c2b3c" in UTF-16LE */
static const uint8_t Usb_HID_String_Serial[28] = {28,  0x03, 'v', 0, 'i', 0, 'a', 0, 'l', 0, ':', 0, 'f', 0,
                                                  '6', 0,    '4', 0, 'c', 0, '2', 0, 'b', 0, '3', 0, 'c', 0};
#else
/* String Descriptor 3 (Serial) - "12345678" in UTF-16LE */
static const uint8_t Usb_HID_String_Serial[18] = {18, 0x03, '1', 0,   '2', 0,   '3', 0,   '4',
                                                  0,  '5',  0,   '6', 0,   '7', 0,   '8', 0};
#endif

/* ---------------------- Private Callbacks --------------------------------- */

/**
 * @brief  Callback triggered when host completes reading the keyboard report.
 */
static USB_Result USB_HID_OnDataSent(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length) {
    (void)EPx;
    (void)Buffer;
    (void)Length;

    /* Release send status to allow next report transmission */
    USB_HID_SendDataStatus = USB_SUCCESS;
    return USB_SUCCESS;
}

/**
 * @brief  Callback triggered during control transfer DATA stage for class requests.
 */
static USB_Result USB_HID_DoDataOut(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length) {
    (void)EPx;

    if (Length > 0 && Buffer != NULL) {
        led_set_hid_state(Buffer[0]);
    }

    /* Currently we only receive SET_REPORT reports (LED status) which we ACK */
    return USB_SUCCESS;
}

#ifdef VIAL
/**
 * @brief  Callback triggered when host sends a Vial raw HID packet.
 */
static USB_Result VIAL_OnDataReceived(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length) {
    (void)EPx;
    if (Length >= 32 && Buffer != NULL) {
        vial_process_packet(Buffer, vial_tx_buffer);
        vial_tx_busy = true;
        USB_EP_doDataIn(USB_EP2, vial_tx_buffer, 32, VIAL_OnDataSent);
    } else {
        /* Re-arm receiver if packet was invalid */
        USB_EP_doDataOut(VIAL_RX_EP, vial_rx_buffer, 32, VIAL_OnDataReceived);
    }
    return USB_SUCCESS;
}

/**
 * @brief  Callback triggered when the device finishes sending the Vial response packet.
 */
static USB_Result VIAL_OnDataSent(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length) {
    (void)EPx;
    (void)Buffer;
    (void)Length;

    vial_tx_busy = false;
    /* Arm receiver for next host packet */
    USB_EP_doDataOut(VIAL_RX_EP, vial_rx_buffer, 32, VIAL_OnDataReceived);
    return USB_SUCCESS;
}
#endif

#ifdef MIDI_USB
static volatile USB_Result USB_MIDI_SendDataStatus = USB_SUCCESS;
static uint8_t USB_MIDI_TxBuffer[4];

static USB_Result USB_MIDI_OnDataSent(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length) {
    (void)EPx;
    (void)Buffer;
    (void)Length;
    USB_MIDI_SendDataStatus = USB_SUCCESS;
    return USB_SUCCESS;
}

USB_Result USB_MIDI_SendPacket(const uint8_t *packet) {
    if (USB_DeviceContext.USB_DeviceState != USB_DEV_STATE_CONFIGURED) {
        return USB_ERR_BUSY;
    }
    if (USB_MIDI_SendDataStatus != USB_SUCCESS) {
        return USB_ERR_BUSY;
    }
    memcpy(USB_MIDI_TxBuffer, packet, 4);
    USB_MIDI_SendDataStatus = USB_ERR_BUSY;

    USB_Result result = USB_EP_doDataIn(USB_EP3, USB_MIDI_TxBuffer, 4, USB_MIDI_OnDataSent);
    if (result != USB_SUCCESS) {
        USB_MIDI_SendDataStatus = USB_SUCCESS;
    }
    return result;
}
#endif

/* ---------------------- Public API ---------------------------------------- */

extern volatile bool usb_mounted;

USB_Result USB_HID_Init(void) {
    USB_HID_SendDataStatus = USB_SUCCESS;
    USB_HID_IdleRate = 0;
    USB_HID_Protocol = 1;
#ifdef MIDI_USB
    USB_MIDI_SendDataStatus = USB_SUCCESS;
#endif
    return USB_SUCCESS;
}

USB_Result USB_HID_Reset(void) {
    USB_Result result;

    /* Call the device framework reset to reset hardware registers */
    result = USB_DeviceReset();

    if (result == USB_SUCCESS) {
        /* Initialize Interrupt Endpoint 1 (IN) to send reports */
        USB_EP_Init(USB_HID_EP_SEND, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);

#ifdef VIAL
        /* Initialize Interrupt Endpoint 2 (IN) and RX endpoint for Vial raw HID */
        USB_EP_Init(USB_EP2, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
        if (VIAL_RX_EP != USB_EP2) {
            USB_EP_Init(VIAL_RX_EP, USB_SEPx_CTRL_EPEN_Enable, 0);
        }
        vial_tx_busy = false;
        USB_EP_doDataOut(VIAL_RX_EP, vial_rx_buffer, 32, VIAL_OnDataReceived);
#endif

#ifdef MIDI_USB
        /* Initialize Bulk Endpoint 3 (IN/OUT) for MIDI */
        USB_EP_Init(USB_EP3, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
#endif

        /* Reset context variables */
        USB_HID_SendDataStatus = USB_SUCCESS;
#ifdef MIDI_USB
        USB_MIDI_SendDataStatus = USB_SUCCESS;
#endif
        usb_mounted = false;
    }

    return result;
}

USB_Result USB_HID_SetConfiguration(uint16_t wValue) {
    if (wValue != 0) {
        usb_mounted = true;
    } else {
        usb_mounted = false;
    }
    return USB_SUCCESS;
}

USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report) {
    USB_Result result = USB_HID_SendDataStatus;

    /* Try to initiate transaction only if endpoint is idle */
    if (result == USB_SUCCESS) {
        USB_HID_SendDataStatus = USB_ERR_BUSY;
        result = USB_EP_doDataIn(USB_HID_EP_SEND, (uint8_t *)report, sizeof(USB_HID_KeyboardReport_TypeDef),
                                 USB_HID_OnDataSent);
    }

    return result;
}

USB_Result USB_HID_SendConsumerReport(uint16_t usage) {
    static uint8_t consumer_report[3];
    consumer_report[0] = 2; // Report ID 2
    consumer_report[1] = usage & 0xFF;
    consumer_report[2] = (usage >> 8) & 0xFF;

    USB_Result result = USB_HID_SendDataStatus;

    /* Try to initiate transaction only if endpoint is idle */
    if (result == USB_SUCCESS) {
        USB_HID_SendDataStatus = USB_ERR_BUSY;
        result = USB_EP_doDataIn(USB_HID_EP_SEND, consumer_report, 3, USB_HID_OnDataSent);
    }

    return result;
}

USB_Result USB_HID_SendMouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan) {
    static uint8_t mouse_report[6];
    mouse_report[0] = 3; // Report ID 3
    mouse_report[1] = buttons;
    mouse_report[2] = (uint8_t)x;
    mouse_report[3] = (uint8_t)y;
    mouse_report[4] = (uint8_t)wheel;
    mouse_report[5] = (uint8_t)pan;

    USB_Result result = USB_HID_SendDataStatus;

    /* Try to initiate transaction only if endpoint is idle */
    if (result == USB_SUCCESS) {
        USB_HID_SendDataStatus = USB_ERR_BUSY;
        result = USB_EP_doDataIn(USB_HID_EP_SEND, mouse_report, sizeof(mouse_report), USB_HID_OnDataSent);
    }

    return result;
}

USB_Result USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat, uint32_t buttons) {
    static uint8_t gamepad_report[12];
    gamepad_report[0] = 4; // Report ID 4
    gamepad_report[1] = (uint8_t)x;
    gamepad_report[2] = (uint8_t)y;
    gamepad_report[3] = (uint8_t)z;
    gamepad_report[4] = (uint8_t)rz;
    gamepad_report[5] = (uint8_t)rx;
    gamepad_report[6] = (uint8_t)ry;
    gamepad_report[7] = hat;
    gamepad_report[8] = (uint8_t)(buttons & 0xFF);
    gamepad_report[9] = (uint8_t)((buttons >> 8) & 0xFF);
    gamepad_report[10] = (uint8_t)((buttons >> 16) & 0xFF);
    gamepad_report[11] = (uint8_t)((buttons >> 24) & 0xFF);

    USB_Result result = USB_HID_SendDataStatus;

    /* Try to initiate transaction only if endpoint is idle */
    if (result == USB_SUCCESS) {
        USB_HID_SendDataStatus = USB_ERR_BUSY;
        result = USB_EP_doDataIn(USB_HID_EP_SEND, gamepad_report, sizeof(gamepad_report), USB_HID_OnDataSent);
    }

    return result;
}

USB_Result USB_HID_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH) {
    const uint8_t *pDescr = 0;
    uint32_t length = 0;
    USB_Result result = USB_SUCCESS;
    uint8_t descType = (uint8_t)(wVALUE >> 8);
    uint8_t descIndex = (uint8_t)(wVALUE & 0xFF);

    switch (descType) {
    case USB_DEVICE:
        pDescr = Usb_HID_Device_Descriptor;
        length = sizeof(Usb_HID_Device_Descriptor);
        break;

    case USB_CONFIGURATION:
        pDescr = Usb_HID_Configuration_Descriptor;
        length = sizeof(Usb_HID_Configuration_Descriptor);
        break;

    case USB_STRING:
        switch (descIndex) {
        case 0:
            pDescr = Usb_HID_String_LangID;
            length = sizeof(Usb_HID_String_LangID);
            break;
        case 1:
            pDescr = Usb_HID_String_Manuf;
            length = sizeof(Usb_HID_String_Manuf);
            break;
        case 2:
            pDescr = Usb_HID_String_Prod;
            length = sizeof(Usb_HID_String_Prod);
            break;
        case 3:
            pDescr = Usb_HID_String_Serial;
            length = sizeof(Usb_HID_String_Serial);
            break;
        default:
            result = USB_ERROR;
            break;
        }
        break;

    case USB_HID_DESCRIPTOR_HID:
#ifdef VIAL
        if ((wINDEX & 0xFF) == 1) {
            pDescr = &Usb_HID_Configuration_Descriptor[43];
        } else {
            pDescr = &Usb_HID_Configuration_Descriptor[18];
        }
#else
        /* HID descriptor is embedded inside configuration descriptor starting at byte 18 */
        pDescr = &Usb_HID_Configuration_Descriptor[18];
#endif
        length = 9;
        break;

    case USB_HID_DESCRIPTOR_REPORT:
#ifdef VIAL
        if ((wINDEX & 0xFF) == 1) {
            pDescr = Usb_Vial_Report_Descriptor;
            length = sizeof(Usb_Vial_Report_Descriptor);
        } else {
            pDescr = Usb_HID_Report_Descriptor;
            length = sizeof(Usb_HID_Report_Descriptor);
        }
#else
        pDescr = Usb_HID_Report_Descriptor;
        length = sizeof(Usb_HID_Report_Descriptor);
#endif
        break;

    default:
        result = USB_ERROR;
        break;
    }

    if (result == USB_SUCCESS && pDescr != 0) {
        if (length > wLENGTH) {
            length = wLENGTH;
        }
        result = USB_EP_doDataIn(USB_EP0, (uint8_t *)pDescr, length, USB_DeviceDoStatusOutAck);
    }

    return result;
}

USB_Result USB_HID_ClassRequest(void) {
    USB_Result result = USB_SUCCESS;
    uint16_t wValue = USB_CurrentSetupPacket.wValue;
    uint16_t wLength = USB_CurrentSetupPacket.wLength;
    static uint8_t tempByte = 0;

    switch (USB_CurrentSetupPacket.bRequest) {
    case USB_HID_SET_IDLE:
        USB_HID_IdleRate = (uint8_t)(wValue >> 8);
        result = USB_SUCCESS;
        break;

    case USB_HID_GET_IDLE:
        tempByte = USB_HID_IdleRate;
        result = USB_EP_doDataIn(USB_EP0, &tempByte, 1, USB_DeviceDoStatusOutAck);
        break;

    case USB_HID_SET_PROTOCOL:
        USB_HID_Protocol = (uint8_t)wValue;
        result = USB_SUCCESS;
        break;

    case USB_HID_GET_PROTOCOL:
        tempByte = USB_HID_Protocol;
        result = USB_EP_doDataIn(USB_EP0, &tempByte, 1, USB_DeviceDoStatusOutAck);
        break;

    case USB_HID_SET_REPORT:
        /* Accept report host sends via data stage */
        if (wLength > 0) {
            result = USB_EP_doDataOut(USB_EP0, &tempByte, wLength, USB_HID_DoDataOut);
        } else {
            result = USB_ERR_INV_REQ;
        }
        break;

    default:
        result = USB_ERROR;
        break;
    }

    /* If no data stage is scheduled, trigger standard status stage */
    if (result == USB_SUCCESS && wLength == 0) {
        result = (USB_CurrentSetupPacket.mRequestTypeData & 0x80) == USB_DEVICE_TO_HOST
                     ? USB_EP_doDataOut(USB_EP0, 0, 0, 0)
                     : USB_EP_doDataIn(USB_EP0, 0, 0, 0);
    }

    return result;
}

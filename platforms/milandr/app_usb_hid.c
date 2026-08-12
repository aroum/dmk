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

#ifdef EXTRAKEY_ENABLE
static const uint8_t Usb_HID_Report_Descriptor[90] = {
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
    0xC0              /* END_COLLECTION */
};
#else
static const uint8_t Usb_HID_Report_Descriptor[63] = {
    0x05, 0x01, /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06, /* USAGE (Keyboard) */
    0xA1, 0x01, /* COLLECTION (Application) */
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
    0xC0        /* END_COLLECTION */
};
#endif

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

#ifdef MIDI_USB
/* Composite Configuration Descriptor: Keyboard HID (Interface 0) + Vial Raw HID (Interface 1) + MIDI AC (Interface 2) +
 * MIDI MS (Interface 3) */
static const uint8_t Usb_HID_Configuration_Descriptor[154] = {
    /* Configuration Descriptor (9 bytes) */
    0x09,      /* bLength */
    0x02,      /* bDescriptorType (Configuration) */
    154, 0x00, /* wTotalLength (154 bytes) */
    0x04,      /* bNumInterfaces (Keyboard, Vial, MIDI AC, MIDI MS) */
    0x01,      /* bConfigurationValue */
    0x00,      /* iConfiguration */
    0xA0,      /* bmAttributes (Bus Powered, Remote Wakeup) */
    50,        /* bMaxPower (100 mA) */

    /* Interface 0 Descriptor - Keyboard (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x03, /* bInterfaceClass (HID) */
    0x01, /* bInterfaceSubClass (Boot) */
    0x01, /* bInterfaceProtocol (Keyboard) */
    0x00, /* iInterface */

    /* HID Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_HID_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_HID_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint 1 IN Descriptor - Keyboard (7 bytes) */
    0x07,       /* bLength */
    0x05,       /* bDescriptorType (Endpoint) */
    0x81,       /* bEndpointAddress (IN EP1) */
    0x03,       /* bmAttributes (Interrupt) */
    0x08, 0x00, /* wMaxPacketSize (8 bytes) */
    10,         /* bInterval (10 ms) */

    /* Interface 1 Descriptor - Vial Raw HID (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x01, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x02, /* bNumEndpoints (1 IN + 1 OUT) */
    0x03, /* bInterfaceClass (HID) */
    0x00, /* bInterfaceSubClass (None) */
    0x00, /* bInterfaceProtocol (None) */
    0x00, /* iInterface */

    /* HID Descriptor for Vial (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_Vial_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_Vial_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint 2 IN Descriptor - Vial IN (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x82,     /* bEndpointAddress (IN EP2) */
    0x03,     /* bmAttributes (Interrupt) */
    32, 0x00, /* wMaxPacketSize (32 bytes) */
    1,        /* bInterval (1 ms) */

    /* Endpoint 2 OUT Descriptor - Vial OUT (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x02,     /* bEndpointAddress (OUT EP2) */
    0x03,     /* bmAttributes (Interrupt) */
    32, 0x00, /* wMaxPacketSize (32 bytes) */
    1,        /* bInterval (1 ms) */

    /* Interface 2 Descriptor - Audio Control (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x02, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass (Audio) */
    0x01, /* bInterfaceSubClass (Audio Control) */
    0x00, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Class-Specific Audio Control Interface Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x24,       /* bDescriptorType (CS Interface) */
    0x01,       /* bDescriptorSubtype (Header) */
    0x00, 0x01, /* bcdADC (1.00) */
    0x09, 0x00, /* wTotalLength (9 bytes) */
    0x01,       /* bInCollection */
    0x03,       /* baInterfaceNr (Interface 3 is MIDI Streaming) */

    /* Interface 3 Descriptor - MIDI Streaming (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x03, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x02, /* bNumEndpoints */
    0x01, /* bInterfaceClass (Audio) */
    0x03, /* bInterfaceSubClass (MIDI Streaming) */
    0x00, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Class-Specific MIDI Streaming Interface Descriptor (7 bytes) */
    0x07,       /* bLength */
    0x24,       /* bDescriptorType (CS Interface) */
    0x01,       /* bDescriptorSubtype (MS Header) */
    0x00, 0x01, /* bcdMSC (1.00) */
    37, 0x00,   /* wTotalLength (37 bytes) */

    /* MIDI IN Jack Descriptor (Embedded) (6 bytes) */
    0x06, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x02, /* bDescriptorSubtype (MIDI_IN_JACK) */
    0x01, /* bJackType (Embedded) */
    0x01, /* bJackID */
    0x00, /* iJack */

    /* MIDI IN Jack Descriptor (External) (6 bytes) */
    0x06, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x02, /* bDescriptorSubtype (MIDI_IN_JACK) */
    0x02, /* bJackType (External) */
    0x02, /* bJackID */
    0x00, /* iJack */

    /* MIDI OUT Jack Descriptor (Embedded) (9 bytes) */
    0x09, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x03, /* bDescriptorSubtype (MIDI_OUT_JACK) */
    0x01, /* bJackType (Embedded) */
    0x03, /* bJackID */
    0x01, /* bNrInputPins */
    0x02, /* baSourceID (from External MIDI IN Jack 2) */
    0x01, /* baSourcePin */
    0x00, /* iJack */

    /* MIDI OUT Jack Descriptor (External) (9 bytes) */
    0x09, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x03, /* bDescriptorSubtype (MIDI_OUT_JACK) */
    0x02, /* bJackType (External) */
    0x04, /* bJackID */
    0x01, /* bNrInputPins */
    0x01, /* baSourceID (from Embedded MIDI IN Jack 1) */
    0x01, /* baSourcePin */
    0x00, /* iJack */

    /* Endpoint 3 OUT Descriptor - MIDI Bulk OUT (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x03,     /* bEndpointAddress (OUT EP3) */
    0x02,     /* bmAttributes (Bulk) */
    64, 0x00, /* wMaxPacketSize (64 bytes) */
    0,        /* bInterval */

    /* Class-Specific MIDI Bulk OUT Endpoint Descriptor (5 bytes) */
    0x05, /* bLength */
    0x25, /* bDescriptorType (CS Endpoint) */
    0x01, /* bDescriptorSubtype (MS General) */
    0x01, /* bNumEmbMIDIJack */
    0x01, /* baAssocJackID (Embedded MIDI IN Jack 1) */

    /* Endpoint 3 IN Descriptor - MIDI Bulk IN (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x83,     /* bEndpointAddress (IN EP3) */
    0x02,     /* bmAttributes (Bulk) */
    64, 0x00, /* wMaxPacketSize (64 bytes) */
    0,        /* bInterval */

    /* Class-Specific MIDI Bulk IN Endpoint Descriptor (5 bytes) */
    0x05, /* bLength */
    0x25, /* bDescriptorType (CS Endpoint) */
    0x01, /* bDescriptorSubtype (MS General) */
    0x01, /* bNumEmbMIDIJack */
    0x03  /* baAssocJackID (Embedded MIDI OUT Jack 3) */
};
#else
/* Standard Configuration Descriptor for VIAL (Configuration + 2 Interfaces) */
static const uint8_t Usb_HID_Configuration_Descriptor[66] = {
    /* Configuration Descriptor (9 bytes) */
    0x09,     /* bLength */
    0x02,     /* bDescriptorType (Configuration) */
    66, 0x00, /* wTotalLength (66 bytes) */
    0x02,     /* bNumInterfaces */
    0x01,     /* bConfigurationValue */
    0x00,     /* iConfiguration */
    0xA0,     /* bmAttributes (Bus Powered, Remote Wakeup) */
    50,       /* bMaxPower (100 mA) */

    /* Interface 0 Descriptor - Keyboard (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x03, /* bInterfaceClass (HID) */
    0x01, /* bInterfaceSubClass (Boot) */
    0x01, /* bInterfaceProtocol (Keyboard) */
    0x00, /* iInterface */

    /* HID Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_HID_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_HID_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint 1 IN Descriptor - Keyboard (7 bytes) */
    0x07,       /* bLength */
    0x05,       /* bDescriptorType (Endpoint) */
    0x81,       /* bEndpointAddress (IN EP1) */
    0x03,       /* bmAttributes (Interrupt) */
    0x08, 0x00, /* wMaxPacketSize (8 bytes) */
    10,         /* bInterval (10 ms) */

    /* Interface 1 Descriptor - Vial Raw HID (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x01, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x02, /* bNumEndpoints (1 IN + 1 OUT) */
    0x03, /* bInterfaceClass (HID) */
    0x00, /* bInterfaceSubClass (None) */
    0x00, /* bInterfaceProtocol (None) */
    0x00, /* iInterface */

    /* HID Descriptor for Vial (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_Vial_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_Vial_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint 2 IN Descriptor - Vial IN (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x82,     /* bEndpointAddress (IN EP2) */
    0x03,     /* bmAttributes (Interrupt) */
    32, 0x00, /* wMaxPacketSize (32 bytes) */
    1,        /* bInterval (1 ms) */

    /* Endpoint 3 OUT Descriptor - Vial OUT (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x03,     /* bEndpointAddress (OUT EP3) */
    0x03,     /* bmAttributes (Interrupt) */
    32, 0x00, /* wMaxPacketSize (32 bytes) */
    1         /* bInterval (1 ms) */
};
#endif
#else
#ifdef MIDI_USB
/* Configuration Descriptor: Keyboard HID (Interface 0) + MIDI AC (Interface 1) + MIDI MS (Interface 2) */
static const uint8_t Usb_HID_Configuration_Descriptor[122] = {
    /* Configuration Descriptor (9 bytes) */
    0x09,      /* bLength */
    0x02,      /* bDescriptorType (Configuration) */
    122, 0x00, /* wTotalLength (122 bytes) */
    0x03,      /* bNumInterfaces (Keyboard, MIDI AC, MIDI MS) */
    0x01,      /* bConfigurationValue */
    0x00,      /* iConfiguration */
    0xA0,      /* bmAttributes (Bus Powered, Remote Wakeup) */
    50,        /* bMaxPower (100 mA) */

    /* Interface Descriptor (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x03, /* bInterfaceClass (HID) */
    0x01, /* bInterfaceSubClass (Boot) */
    0x01, /* bInterfaceProtocol (Keyboard) */
    0x00, /* iInterface */

    /* HID Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_HID_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_HID_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint Descriptor (7 bytes) */
    0x07,       /* bLength */
    0x05,       /* bDescriptorType (Endpoint) */
    0x81,       /* bEndpointAddress (IN EP1) */
    0x03,       /* bmAttributes (Interrupt) */
    0x08, 0x00, /* wMaxPacketSize (8 bytes) */
    10,         /* bInterval (10 ms) */

    /* Interface 1 Descriptor - Audio Control (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x01, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass (Audio) */
    0x01, /* bInterfaceSubClass (Audio Control) */
    0x00, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Class-Specific Audio Control Interface Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x24,       /* bDescriptorType (CS Interface) */
    0x01,       /* bDescriptorSubtype (Header) */
    0x00, 0x01, /* bcdADC (1.00) */
    0x09, 0x00, /* wTotalLength (9 bytes) */
    0x01,       /* bInCollection */
    0x02,       /* baInterfaceNr (Interface 2 is MIDI Streaming) */

    /* Interface 2 Descriptor - MIDI Streaming (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x02, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x02, /* bNumEndpoints */
    0x01, /* bInterfaceClass (Audio) */
    0x03, /* bInterfaceSubClass (MIDI Streaming) */
    0x00, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Class-Specific MIDI Streaming Interface Descriptor (7 bytes) */
    0x07,       /* bLength */
    0x24,       /* bDescriptorType (CS Interface) */
    0x01,       /* bDescriptorSubtype (MS Header) */
    0x00, 0x01, /* bcdMSC (1.00) */
    37, 0x00,   /* wTotalLength (37 bytes) */

    /* MIDI IN Jack Descriptor (Embedded) (6 bytes) */
    0x06, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x02, /* bDescriptorSubtype (MIDI_IN_JACK) */
    0x01, /* bJackType (Embedded) */
    0x01, /* bJackID */
    0x00, /* iJack */

    /* MIDI IN Jack Descriptor (External) (6 bytes) */
    0x06, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x02, /* bDescriptorSubtype (MIDI_IN_JACK) */
    0x02, /* bJackType (External) */
    0x02, /* bJackID */
    0x00, /* iJack */

    /* MIDI OUT Jack Descriptor (Embedded) (9 bytes) */
    0x09, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x03, /* bDescriptorSubtype (MIDI_OUT_JACK) */
    0x01, /* bJackType (Embedded) */
    0x03, /* bJackID */
    0x01, /* bNrInputPins */
    0x02, /* baSourceID (from External MIDI IN Jack 2) */
    0x01, /* baSourcePin */
    0x00, /* iJack */

    /* MIDI OUT Jack Descriptor (External) (9 bytes) */
    0x09, /* bLength */
    0x24, /* bDescriptorType (CS Interface) */
    0x03, /* bDescriptorSubtype (MIDI_OUT_JACK) */
    0x02, /* bJackType (External) */
    0x04, /* bJackID */
    0x01, /* bNrInputPins */
    0x01, /* baSourceID (from Embedded MIDI IN Jack 1) */
    0x01, /* baSourcePin */
    0x00, /* iJack */

    /* Endpoint 3 OUT Descriptor - MIDI Bulk OUT (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x03,     /* bEndpointAddress (OUT EP3) */
    0x02,     /* bmAttributes (Bulk) */
    64, 0x00, /* wMaxPacketSize (64 bytes) */
    0,        /* bInterval */

    /* Class-Specific MIDI Bulk OUT Endpoint Descriptor (5 bytes) */
    0x05, /* bLength */
    0x25, /* bDescriptorType (CS Endpoint) */
    0x01, /* bDescriptorSubtype (MS General) */
    0x01, /* bNumEmbMIDIJack */
    0x01, /* baAssocJackID (Embedded MIDI IN Jack 1) */

    /* Endpoint 3 IN Descriptor - MIDI Bulk IN (7 bytes) */
    0x07,     /* bLength */
    0x05,     /* bDescriptorType (Endpoint) */
    0x83,     /* bEndpointAddress (IN EP3) */
    0x02,     /* bmAttributes (Bulk) */
    64, 0x00, /* wMaxPacketSize (64 bytes) */
    0,        /* bInterval */

    /* Class-Specific MIDI Bulk IN Endpoint Descriptor (5 bytes) */
    0x05, /* bLength */
    0x25, /* bDescriptorType (CS Endpoint) */
    0x01, /* bDescriptorSubtype (MS General) */
    0x01, /* bNumEmbMIDIJack */
    0x03  /* baAssocJackID (Embedded MIDI OUT Jack 3) */
};
#else
/* Standard Configuration Descriptor (Configuration + Interface + HID + Endpoint) */
static const uint8_t Usb_HID_Configuration_Descriptor[34] = {
    /* Configuration Descriptor (9 bytes) */
    0x09,     /* bLength */
    0x02,     /* bDescriptorType (Configuration) */
    34, 0x00, /* wTotalLength (34 bytes) */
    0x01,     /* bNumInterfaces */
    0x01,     /* bConfigurationValue */
    0x00,     /* iConfiguration */
    0xA0,     /* bmAttributes (Bus Powered, Remote Wakeup) */
    50,       /* bMaxPower (100 mA) */

    /* Interface Descriptor (9 bytes) */
    0x09, /* bLength */
    0x04, /* bDescriptorType (Interface) */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x03, /* bInterfaceClass (HID) */
    0x01, /* bInterfaceSubClass (Boot) */
    0x01, /* bInterfaceProtocol (Keyboard) */
    0x00, /* iInterface */

    /* HID Descriptor (9 bytes) */
    0x09,       /* bLength */
    0x21,       /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID (1.11) */
    0x00,       /* bCountryCode */
    0x01,       /* bNumDescriptors */
    0x22,       /* bDescriptorType (Report) */
    (uint8_t)(sizeof(Usb_HID_Report_Descriptor) & 0xFF),
    (uint8_t)((sizeof(Usb_HID_Report_Descriptor) >> 8) & 0xFF), /* wDescriptorLength */

    /* Endpoint Descriptor (7 bytes) */
    0x07,       /* bLength */
    0x05,       /* bDescriptorType (Endpoint) */
    0x81,       /* bEndpointAddress (IN EP1) */
    0x03,       /* bmAttributes (Interrupt) */
    0x08, 0x00, /* wMaxPacketSize (8 bytes) */
    10          /* bInterval (10 ms) */
};
#endif
#endif

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
#if defined(MIDI_USB)
        USB_EP_doDataOut(USB_EP2, vial_rx_buffer, 32, VIAL_OnDataReceived);
#else
        USB_EP_doDataOut(USB_EP3, vial_rx_buffer, 32, VIAL_OnDataReceived);
#endif
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
#if defined(MIDI_USB)
    USB_EP_doDataOut(USB_EP2, vial_rx_buffer, 32, VIAL_OnDataReceived);
#else
    USB_EP_doDataOut(USB_EP3, vial_rx_buffer, 32, VIAL_OnDataReceived);
#endif
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
#if defined(MIDI_USB)
        /* Initialize Interrupt Endpoint 2 (IN/OUT) for Vial raw HID */
        USB_EP_Init(USB_EP2, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
        vial_tx_busy = false;
        USB_EP_doDataOut(USB_EP2, vial_rx_buffer, 32, VIAL_OnDataReceived);

        /* Initialize Bulk Endpoint 3 (IN/OUT) for MIDI */
        USB_EP_Init(USB_EP3, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
#else
        /* Initialize Interrupt Endpoint 2 (IN) and Endpoint 3 (OUT) for Vial raw HID */
        USB_EP_Init(USB_EP2, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
        USB_EP_Init(USB_EP3, USB_SEPx_CTRL_EPEN_Enable, 0);
        vial_tx_busy = false;
        USB_EP_doDataOut(USB_EP3, vial_rx_buffer, 32, VIAL_OnDataReceived);
#endif
#else
#if defined(MIDI_USB)
        /* Initialize Bulk Endpoint 3 (IN/OUT) for MIDI */
        USB_EP_Init(USB_EP3, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1, 0);
#endif
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
#ifdef EXTRAKEY_ENABLE
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
#else
    (void)usage;
    return USB_SUCCESS;
#endif
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

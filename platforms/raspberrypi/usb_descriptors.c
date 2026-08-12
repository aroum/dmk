#include "config.h"
#include "tusb.h"
#include "vial.h"

#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif

#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x4010
#endif

#ifndef VIAL_KEYBOARD_NAME
#define VIAL_KEYBOARD_NAME "Pncateho"
#endif

// HID Report Descriptor
uint8_t const desc_hid_report[] = {
#ifdef EXTRAKEY_ENABLE
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)), TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))
#else
    TUD_HID_REPORT_DESC_KEYBOARD()
#endif
};

#ifdef VIAL
// 32-byte Raw HID / Vial descriptor
uint8_t const desc_vial_report[] = {
    0x06, 0x60, 0xFF, // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,       // Usage (Vendor Defined 0x61)
    0xa1, 0x01,       // Collection (Application)
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
    0xc0              // End Collection
};
#endif

// Device descriptor
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = 0x00, // Defined at interface level
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = VIAL_VENDOR_ID,   // Vendor ID
    .idProduct = VIAL_PRODUCT_ID, // Product ID
    .bcdDevice = 0x0100,          // Device release number

    .iManufacturer = 0x01, // Manufacturer string index
    .iProduct = 0x02,      // Product string index
    .iSerialNumber = 0x03, // Serial number string index

    .bNumConfigurations = 0x01 // Number of configurations
};

#ifdef MIDI_USB
#ifdef VIAL
#define ITF_NUM_MIDI_AC 2
#define ITF_NUM_MIDI_MS 3
#else
#define ITF_NUM_MIDI_AC 1
#define ITF_NUM_MIDI_MS 2
#endif
#define EPNUM_MIDI_OUT 0x03
#define EPNUM_MIDI_IN 0x83
#endif

#if defined(VIAL) && defined(MIDI_USB)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 2 * TUD_HID_DESC_LEN + TUD_MIDI_DESC_LEN)
#define TOTAL_INTERFACES 4
#elif defined(VIAL)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 2 * TUD_HID_DESC_LEN)
#define TOTAL_INTERFACES 2
#elif defined(MIDI_USB)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_MIDI_DESC_LEN)
#define TOTAL_INTERFACES 3
#else
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define TOTAL_INTERFACES 1
#endif

// Configuration descriptor
uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(1,                // Configuration number
                          TOTAL_INTERFACES, // Number of interfaces
                          0,                // Configuration string index
                          CONFIG_TOTAL_LEN, // Total length of data for this configuration
                          0,                // Attributes (bus-powered)
                          100),             // Maximum power consumption in 2mA units (100 * 2mA = 200mA)

    // Interface 0: Keyboard HID Descriptor
    TUD_HID_DESCRIPTOR(0,                         // Interface number
                       0,                         // String index
                       HID_ITF_PROTOCOL_KEYBOARD, // Protocol code
                       sizeof(desc_hid_report),   // HID report descriptor length
                       0x81,                      // Endpoint address (IN endpoint)
                       CFG_TUD_HID_EP_BUFSIZE,    // Endpoint size
                       10),                       // Polling interval in milliseconds

#ifdef VIAL
    // Interface 1: Vial Raw HID Descriptor
    TUD_HID_DESCRIPTOR(1,                        // Interface number
                       0,                        // String index
                       HID_ITF_PROTOCOL_NONE,    // Protocol code
                       sizeof(desc_vial_report), // HID report descriptor length
                       0x82,                     // Endpoint address (IN endpoint)
                       32,                       // Endpoint size (32 bytes)
                       1),                       // Polling interval in milliseconds
#endif

#ifdef MIDI_USB
    // Interface 2 & 3 (or 1 & 2): MIDI descriptor
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI_AC, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
#endif
};

// String Descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // Supported language (English - 0x0409)
    "dmk",                      // Manufacturer string
    VIAL_KEYBOARD_NAME,         // Product string
#ifdef VIAL
    "vial:f64c2b3c", // Serial number string for Vial detection
#else
    "123456", // Serial number string
#endif
};

uint16_t _desc_str[32];

// Device descriptor callback
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

// Configuration descriptor callback
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// HID report descriptor callback
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
#ifdef VIAL
    if (instance == 1) {
        return desc_vial_report;
    }
#endif
    (void)instance;
    return desc_hid_report;
}

extern volatile bool usb_mounted;

// Invoked when device is mounted
void tud_mount_cb(void) {
    usb_mounted = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    usb_mounted = false;
}

// This callback will be invoked when the host sends a SET_PROTOCOL request to switch between these modes.
void tud_hid_boot_mode_cb(uint8_t instance, bool boot_mode) {
    (void)instance;
    (void)boot_mode;
}

// This callback is invoked when the host sends a GET_REPORT request.
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

extern void led_set_hid_state(uint8_t state);

// This callback is invoked when the host sends a SET_REPORT request.
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)report_id;
#ifdef VIAL
    if (instance == 1) {
        if (bufsize >= 32) {
            uint8_t response[32];
            vial_process_packet(buffer, response);
            tud_hid_n_report(1, 0, response, 32);
        }
        return;
    }
#endif
    (void)instance;
    (void)bufsize;
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        led_set_hid_state(buffer[0]);
    }
}

// String descriptor callback
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        _desc_str[0] = (TUSB_DESC_STRING << 8) | 4;
    } else {
        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char *str = string_desc_arr[index];
        uint8_t chr_count = strlen(str);
        if (chr_count > 31)
            chr_count = 31;
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    }
    return _desc_str;
}

#include "config.h"
#include "led.h"
#include "tusb.h"

#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif

#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x4000
#endif

#ifndef VIAL_KEYBOARD_NAME
#define VIAL_KEYBOARD_NAME "Selenium S"
#endif

// HID Report Descriptor
uint8_t const desc_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD()};

// Device descriptor
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = TUSB_CLASS_HID,               // HID class
    .bDeviceSubClass = HID_SUBCLASS_BOOT,         // Boot subclass
    .bDeviceProtocol = HID_ITF_PROTOCOL_KEYBOARD, // Keyboard protocol
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = VIAL_VENDOR_ID,   // Vendor ID
    .idProduct = VIAL_PRODUCT_ID, // Product ID
    .bcdDevice = 0x0100,          // Device release number

    .iManufacturer = 0x01, // Manufacturer string index
    .iProduct = 0x02,      // Product string index
    .iSerialNumber = 0x03, // Serial number string index

    .bNumConfigurations = 0x01 // Number of configurations
};

// Configuration descriptor
uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(1,                                        // Configuration number
                          1,                                        // Number of interfaces
                          0,                                        // Configuration string index
                          (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN), // Total length of data for this configuration
                          0,                                        // Attributes (bus-powered)
                          100), // Maximum power consumption in 2mA units (100 * 2mA = 200mA)

    // HID Descriptor
    TUD_HID_DESCRIPTOR(0,                         // Interface number
                       0,                         // String index
                       HID_ITF_PROTOCOL_KEYBOARD, // Protocol code
                       sizeof(desc_hid_report),   // HID report descriptor length
                       0x81,                      // Endpoint address (IN endpoint)
                       CFG_TUD_HID_EP_BUFSIZE,    // Endpoint size
                       10)                        // Polling interval in milliseconds
};

// String Descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // Supported language (English - 0x0409)
    "Tknbr",                    // Manufacturer string
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
    (void)instance;
    return desc_hid_report;
}

// This callback will be invoked when the host sends a SET_PROTOCOL request to switch between these modes.
void tud_hid_boot_mode_cb(uint8_t instance, bool boot_mode) {
    if (boot_mode) {
        // Prepare to operate in boot mode
        // Example: Simplify the input report format to the 8-byte standard for keyboards
        // Modify your report structure or internal state as needed
    } else {
        // Switch to normal HID report protocol
        // Example: Enable additional keys or features in your reports
        // Restore or adjust the report structure for full HID functionality
    }
}

/** This callback is invoked when the host sends a GET_REPORT request. It is used to handle report requests from the
 * host. instance: The HID interface instance. report_id: The ID of the report (if the device supports multiple report
 * IDs). report_type: The type of report (Input, Output, Feature). buffer: The buffer where the report data should be
 * stored. reqlen: The length of the requested report. Returns: The actual length of the report.
 */
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Return the current state of the keyboard LEDs (e.g., Num Lock, Caps Lock)
        buffer[0] = 0x02; // Example: Caps Lock is on (0x02)
        return 1;         // Report length for LEDs (1 byte)
    }
    return 0;
}

/** This callback is invoked when the host sends a SET_REPORT request to the HID interface. It is used to handle report
 * requests from the host. instance: The HID interface instance. report_id: The ID of the report (if the device supports
 * multiple report IDs). report_type: The type of report (Input, Output, Feature). buffer: The buffer where the report
 * data should be stored. bufsize: The size of the report data
 */
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        if (bufsize > 0 && buffer != NULL) {
            led_set_hid_state(buffer[0]);
        }
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

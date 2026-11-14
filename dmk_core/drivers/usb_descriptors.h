#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

// HID Report Descriptor
extern uint8_t const desc_hid_report[];

// Device descriptor callback
uint8_t const *tud_descriptor_device_cb(void);

// Configuration descriptor callback
uint8_t const *tud_descriptor_configuration_cb(uint8_t index);

// HID report descriptor callback
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance);

// String descriptor callback
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);

// Debug console output
void debug_console_write(const char *buf, size_t len);
void debug_console_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void debug_console_flush(void);

#endif // USB_DESCRIPTORS_H_

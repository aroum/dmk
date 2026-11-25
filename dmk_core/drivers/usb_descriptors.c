#include "usb_report_diag.h"
#include "config.h"
#include "led.h"
#include "tusb.h"
#include "vial.h"
#include "dmk_fmt.h"
#include <stdarg.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

extern void usb_notify_task(void);
void debug_console_flush(void);
void debug_console_printf(const char *fmt, ...);

#include "rgb.h"

#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif

#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x4010
#endif

#ifndef VIAL_KEYBOARD_NAME
#define VIAL_KEYBOARD_NAME "DMK Keyboard"
#endif

// HID Report Descriptor: Composite Keyboard (ID 1), Consumer (ID 2), Mouse (ID 3), Gamepad (ID 4)
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2)),
#ifndef NO_MOUSE
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(3)),
#endif
#ifndef NO_GAMEPAD
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(4)),
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

#ifdef DEBUG_CONSOLE_ENABLE
// QMK / VIA Debug Console descriptor (Usage Page 0xFF31, Usage 0x0074)
uint8_t const desc_console_report[] = {
    0x06, 0x31, 0xFF, // Usage Page (Vendor Defined 0xFF31)
    0x09, 0x74,       // Usage (Vendor Defined 0x0074)
    0xa1, 0x01,       // Collection (Application)
    0x09, 0x75,       //   Usage (Vendor Defined 0x0075)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x20,       //   Report Count (32)
    0x81, 0x02,       //   Input (Data, Var, Abs)
    0xc0              // End Collection
};
#endif

enum {
    ITF_NUM_KEYBOARD = 0,
#ifdef VIAL
    ITF_NUM_VIAL,
#endif
#ifdef DEBUG_CONSOLE_ENABLE
    ITF_NUM_CONSOLE,
#endif
    ITF_TOTAL
};

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

#if defined(DEBUG_CONSOLE_ENABLE) && defined(VIAL) && defined(MIDI_USB)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN + TUD_MIDI_DESC_LEN)
#define TOTAL_INTERFACES 5
#elif defined(DEBUG_CONSOLE_ENABLE) && defined(VIAL)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN)
#define TOTAL_INTERFACES 3
#elif defined(VIAL) && defined(MIDI_USB)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_MIDI_DESC_LEN)
#define TOTAL_INTERFACES 4
#elif defined(VIAL)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#define TOTAL_INTERFACES 2
#elif defined(MIDI_USB)
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_MIDI_DESC_LEN)
#define TOTAL_INTERFACES 3
#else
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define TOTAL_INTERFACES 1
#endif

#ifndef USB_REMOTE_WAKEUP_ENABLED
#define USB_REMOTE_WAKEUP_ENABLED 1
#endif

#if USB_REMOTE_WAKEUP_ENABLED
#define USB_CONFIG_ATTRIBUTES TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP
#else
#define USB_CONFIG_ATTRIBUTES 0
#endif

// Configuration descriptor
uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(1,                      // Configuration number
                          TOTAL_INTERFACES,       // Number of interfaces
                          0,                      // Configuration string index
                          CONFIG_TOTAL_LEN,       // Total length of data for this configuration
                          USB_CONFIG_ATTRIBUTES,  // Attributes (bus-powered + optional remote wakeup)
                          100),                   // Maximum power consumption in 2mA units (100 * 2mA = 200mA)

#ifndef HID_POLL_INTERVAL_MS
#define HID_POLL_INTERVAL_MS 1
#endif

    // Interface 0: Composite Keyboard/Consumer/Mouse HID Descriptor
    TUD_HID_DESCRIPTOR(0,                       // Interface number
                       0,                       // String index
                       HID_ITF_PROTOCOL_NONE,   // Protocol code (composite with Report IDs)
                       sizeof(desc_hid_report), // HID report descriptor length
                       0x81,                    // Endpoint address (IN endpoint)
                       CFG_TUD_HID_EP_BUFSIZE,  // Endpoint size
                       HID_POLL_INTERVAL_MS),   // Polling interval in milliseconds (1ms = 1000Hz)

#ifndef EPNUM_VIAL_OUT
#define EPNUM_VIAL_OUT 0x04
#endif
#ifndef EPNUM_VIAL_IN
#define EPNUM_VIAL_IN 0x82
#endif

#ifndef TUD_HID_INOUT_ASYM_DESCRIPTOR
#define TUD_HID_INOUT_ASYM_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epout, _epout_size, _epin, _epin_size, _ep_interval) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_HID, (uint8_t)((_boot_protocol) ? (uint8_t)HID_SUBCLASS_BOOT : 0), _boot_protocol, _stridx,\
  /* HID descriptor */\
  9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0111), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(_report_desc_len),\
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epout_size), _ep_interval, \
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epin_size), _ep_interval
#endif

#ifdef VIAL
    // Interface 1: Vial Raw HID Descriptor (OUT: 64 bytes, IN: 32 bytes)
    TUD_HID_INOUT_ASYM_DESCRIPTOR(ITF_NUM_VIAL,             // Interface number
                                  0,                        // String index
                                  HID_ITF_PROTOCOL_NONE,    // Protocol code
                                  sizeof(desc_vial_report), // HID report descriptor length
                                  EPNUM_VIAL_OUT,           // Endpoint address (OUT endpoint)
                                  64,                       // Endpoint OUT size (64 bytes for short-packet completion)
                                  EPNUM_VIAL_IN,            // Endpoint address (IN endpoint)
                                  32,                       // Endpoint IN size (32 bytes)
                                  1),                       // Polling interval in milliseconds
#endif

#ifdef DEBUG_CONSOLE_ENABLE
#ifndef EPNUM_CONSOLE_IN
#define EPNUM_CONSOLE_IN 0x83
#endif
    // Interface 2: QMK / VIA Debug Console HID Descriptor (IN endpoint)
    TUD_HID_DESCRIPTOR(ITF_NUM_CONSOLE,
                       0,
                       HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_console_report),
                       EPNUM_CONSOLE_IN,
                       32,
                       1),
#endif

#ifdef MIDI_USB
    // Interface 2 & 3 (or 1 & 2): MIDI descriptor
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI_AC, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
#endif
};

#ifndef DMK_FW_VERSION
#define DMK_FW_VERSION "unknown"
#endif

// String Descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // Supported language (English - 0x0409)
    "dmk",                      // Manufacturer string
    VIAL_KEYBOARD_NAME,         // Product string
#if defined(VIA_V3) || defined(USE_VIA_V3)
    DMK_FW_VERSION,             // Firmware version string for VIA builds
#elif defined(VIAL)
    "vial:f64c2b3c",            // Serial number string for Vial detection
#else
    DMK_FW_VERSION,             // Serial number string
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

#if TUD_OPT_HIGH_SPEED
static tusb_desc_device_qualifier_t const desc_device_qualifier = {
    .bLength            = sizeof(tusb_desc_device_qualifier_t),
    .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .bNumConfigurations = 0x01,
    .bReserved          = 0x00
};

static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

uint8_t const *tud_descriptor_device_qualifier_cb(void) {
    return (uint8_t const *)&desc_device_qualifier;
}

uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index) {
    (void)index;
    memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
    desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;
    return desc_other_speed_config;
}
#endif

// HID report descriptor callback
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
#ifdef VIAL
    if (instance == ITF_NUM_VIAL) {
        return desc_vial_report;
    }
#endif
#ifdef DEBUG_CONSOLE_ENABLE
    if (instance == ITF_NUM_CONSOLE) {
        return desc_console_report;
    }
#endif
    (void)instance;
    return desc_hid_report;
}

volatile bool usb_mounted = false;

// Invoked when device is mounted
void tud_mount_cb(void) {
    extern void USB_HID_ResyncKeyboard(void);
    usb_mounted = true;
    led_resume();
    rgb_resume();
    USB_HID_ResyncKeyboard();
#ifdef DEBUG_CONSOLE_ENABLE
    debug_console_printf("[BOOT] HID console ready\r\n");
#endif
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    usb_mounted = false;
}

// Invoked when device is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    led_suspend();
    rgb_suspend();
}

// Re-send the current keyboard state if it changed while USB was suspended.
void tud_resume_cb(void) {
    led_resume();
    rgb_resume();
    extern void USB_HID_ResyncKeyboard(void);
    USB_HID_ResyncKeyboard();
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

#ifdef VIAL
static uint8_t pending_vial_response[32];
static volatile bool has_pending_vial_response = false;

volatile uint32_t vial_rx_count = 0;
volatile uint32_t vial_tx_submit_count = 0;
volatile uint32_t vial_tx_complete_count = 0;
volatile uint32_t vial_tx_fail_count = 0;

void vial_flush_pending_report(void) {
    if (has_pending_vial_response && tud_hid_n_ready(ITF_NUM_VIAL)) {
        if (tud_hid_n_report(ITF_NUM_VIAL, 0, pending_vial_response, 32)) {
            has_pending_vial_response = false;
            vial_tx_submit_count++;
        } else {
            vial_tx_fail_count++;
        }
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
#ifdef HHKB_USB_REPORT_DIAGNOSTICS
    usb_report_diag_complete(instance, report, len);
#endif
    (void)report;
    (void)len;
#ifdef VIAL
    if (instance == ITF_NUM_VIAL) {
        vial_tx_complete_count++;
        vial_flush_pending_report();
    }
#endif
#ifdef DEBUG_CONSOLE_ENABLE
    if (instance == ITF_NUM_CONSOLE) {
        debug_console_flush();
    }
#endif
}
#endif

#ifdef DEBUG_CONSOLE_ENABLE
__attribute__((weak)) void hhkb_calc_rates(uint32_t elapsed_ms) { (void)elapsed_ms; }
__attribute__((weak)) uint32_t hhkb_get_matrix_rate_hz(void) { return 0; }
__attribute__((weak)) bool hhkb_get_debug_flag(uint8_t flag) {
    (void)flag;
    return false;
}
__attribute__((weak)) uint8_t hhkb_get_scan_rate(void) { return 4; }
__attribute__((weak)) uint8_t hhkb_get_effective_scan_rate(void) { return hhkb_get_scan_rate(); }
__attribute__((weak)) void hhkb_get_scan_diagnostics(uint32_t *scan_us, uint32_t *scan_max_us, uint32_t *work_max_us, uint32_t *missed, uint32_t *q_full) {
    if (scan_us) *scan_us = 0;
    if (scan_max_us) *scan_max_us = 0;
    if (work_max_us) *work_max_us = 0;
    if (missed) *missed = 0;
    if (q_full) *q_full = 0;
}
__attribute__((weak)) void hhkb_reset_scan_diagnostics_peaks(void) {}
__attribute__((weak)) void hhkb_get_rt_rebuild(uint32_t *ms, uint32_t *count) {
    if (ms) *ms = 0;
    if (count) *count = 0;
}
__attribute__((weak)) void hhkb_get_latency_diagnostics(uint32_t *avg_us, uint32_t *max_us, int32_t *err_us,
                                                       uint32_t *locked, uint32_t *lead_us, int32_t *in_us) {
    if (avg_us) *avg_us = 0;
    if (max_us) *max_us = 0;
    if (err_us) *err_us = 0;
    if (locked) *locked = 0;
    if (lead_us) *lead_us = 0;
    if (in_us) *in_us = 0;
}

static uint32_t s_last_diag_tick = 0;
static uint32_t s_usb_loop_total = 0;
static uint32_t s_usb_loop_prev = 0;

void vial_diag_task(void) {
    s_usb_loop_total++;
    uint32_t now = xTaskGetTickCount();
    uint32_t elapsed_ticks = now - s_last_diag_tick;
    if (elapsed_ticks >= pdMS_TO_TICKS(1000)) {
        s_last_diag_tick = now;
        uint32_t elapsed_ms = (uint32_t)(elapsed_ticks * portTICK_PERIOD_MS);
        if (elapsed_ms == 0) elapsed_ms = 1000;

        uint32_t usb_diff = s_usb_loop_total - s_usb_loop_prev;
        s_usb_loop_prev = s_usb_loop_total;
        // 32-bit is exact here: usb_diff would need > 4.29M USB loops/s to overflow.
        uint32_t usb_rate_hz = (usb_diff * 1000U + (elapsed_ms / 2)) / elapsed_ms;

        hhkb_calc_rates(elapsed_ms);
#ifdef HHKB_SCAN_DIAGNOSTICS
        {
            // RT cache rebuilds are recorded in the matrix task and logged
            // here; its 1 KB stack cannot carry the printf chain itself.
            static uint32_t s_rt_rebuild_logged;
            uint32_t rebuild_ms = 0, rebuild_count = 0;
            hhkb_get_rt_rebuild(&rebuild_ms, &rebuild_count);
            if (rebuild_count != s_rt_rebuild_logged) {
                debug_console_printf("[CFG] RT rebuild=%lu ms (x%lu)\r\n",
                                     (unsigned long)rebuild_ms,
                                     (unsigned long)(rebuild_count - s_rt_rebuild_logged));
                s_rt_rebuild_logged = rebuild_count;
            }
        }
#endif
#ifdef HHKB_USB_REPORT_DIAGNOSTICS
        if (hhkb_get_debug_flag(1U << 4) && tud_mounted() && !tud_suspended()) {
            usb_report_diag_t d;
            usb_report_diag_snapshot(&d);
            debug_console_printf("[USB] sub=%lu done=%lu busy=%lu drop=%lu resync=%lu q=%lu\r\n",
                (unsigned long)d.submitted, (unsigned long)d.completed, (unsigned long)d.busy,
                (unsigned long)d.dropped, (unsigned long)d.resync, (unsigned long)d.queued);
        }
#endif


        if (hhkb_get_debug_flag(1U << 1)) { // DEBUG_FLAG_VIA
            debug_console_printf("[VIA_DIAG] rx=%lu tx_sub=%lu tx_cpl=%lu tx_fail=%lu pend=%d\r\n",
                                 (unsigned long)vial_rx_count, (unsigned long)vial_tx_submit_count,
                                 (unsigned long)vial_tx_complete_count, (unsigned long)vial_tx_fail_count,
                                 has_pending_vial_response ? 1 : 0);
        }

        if (hhkb_get_debug_flag(1U << 2)) { // DEBUG_FLAG_RATE
            static const uint16_t s_target_rates[] = {50, 125, 250, 500, 1000, 200, 91};
            uint8_t r_idx = hhkb_get_effective_scan_rate();
            uint16_t target_hz = (r_idx < sizeof(s_target_rates) / sizeof(s_target_rates[0])) ? s_target_rates[r_idx] : 1000;
#ifdef HHKB_SCAN_DIAGNOSTICS
            uint32_t scan_us = 0, scan_max_us = 0, work_max_us = 0, missed = 0, q_full = 0;
            hhkb_get_scan_diagnostics(&scan_us, &scan_max_us, &work_max_us, &missed, &q_full);
            debug_console_printf("[SCAN] target=%u actual=%lu Hz | raw=%lu us (max %lu) work_max=%lu us | miss=%lu q_full=%lu (usb=%lu)\r\n",
                                 target_hz, (unsigned long)hhkb_get_matrix_rate_hz(),
                                 (unsigned long)scan_us, (unsigned long)scan_max_us,
                                 (unsigned long)work_max_us, (unsigned long)missed, (unsigned long)q_full,
                                 (unsigned long)usb_rate_hz);
            {
                uint32_t age_avg = 0, age_max = 0, lock = 0, lead = 0;
                int32_t err = 0, in_off = 0;
                hhkb_get_latency_diagnostics(&age_avg, &age_max, &err, &lock, &lead, &in_off);
                debug_console_printf("[LAT] age_avg=%lu age_max=%lu us err=%ld us lock=%lu lead=%lu us in=%ld us\r\n",
                                     (unsigned long)age_avg, (unsigned long)age_max, (long)err,
                                     (unsigned long)lock, (unsigned long)lead, (long)in_off);
            }
            hhkb_reset_scan_diagnostics_peaks();
#else
            debug_console_printf("[RATE] matrix=%lu Hz (target: %u Hz), usb_loop=%lu Hz\r\n",
                                 (unsigned long)hhkb_get_matrix_rate_hz(), target_hz,
                                 (unsigned long)usb_rate_hz);
#endif
        }
    }
}
#endif

// This callback is invoked when the host sends a SET_REPORT request.
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)report_id;
#ifdef VIAL
    if (instance == ITF_NUM_VIAL) {
        if (bufsize >= 32) {
            vial_rx_count++;
            uint8_t response[32];
            vial_process_packet(buffer, response);
            if (tud_hid_n_report(ITF_NUM_VIAL, 0, response, 32)) {
                vial_tx_submit_count++;
                has_pending_vial_response = false;
            } else {
                vial_tx_fail_count++;
                memcpy(pending_vial_response, response, 32);
                has_pending_vial_response = true;
            }
        }
        return;
    }
#endif
    (void)instance;
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // A zero-length SET_REPORT(Output) would otherwise consume a stale byte.
        if (bufsize >= 1) led_set_hid_state(buffer[0]);
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

#ifndef CONSOLE_BUF_SIZE
#define CONSOLE_BUF_SIZE 256
#endif
static char s_console_buf[CONSOLE_BUF_SIZE];
static volatile uint16_t s_console_head = 0;
static volatile uint16_t s_console_tail = 0;

void debug_console_write(const char *buf, size_t len) {
    if (!buf || len == 0) return;
    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (running) taskENTER_CRITICAL();

    uint16_t head = s_console_head;
    uint16_t tail = s_console_tail;
    size_t free_space = (tail > head) ? (tail - head - 1) : (CONSOLE_BUF_SIZE - 1 - (head - tail));

    if (len <= free_space) {
        size_t first = CONSOLE_BUF_SIZE - head;
        if (first > len) first = len;
        memcpy(s_console_buf + head, buf, first);
        if (len > first) memcpy(s_console_buf, buf + first, len - first);
        s_console_head = (head + len) % CONSOLE_BUF_SIZE;
    }
    // Drop entire message if buffer is full to prevent tearing and corrupted lines

    if (running) {
        taskEXIT_CRITICAL();
        if (head == tail && len <= free_space) usb_notify_task();
    }
}

static void fmt_emit(char *buf, size_t size, size_t *pos, char c) {
    if (*pos + 1 < size) buf[*pos] = c;
    (*pos)++;
}

static void fmt_uint(char *buf, size_t size, size_t *pos, unsigned long val,
                     unsigned base, bool upper, unsigned width, char pad, bool neg) {
    char digits[12];
    unsigned n = 0;
    do {
        unsigned d = (unsigned)(val % base);
        digits[n++] = (char)(d < 10 ? '0' + d : (upper ? 'A' : 'a') + (d - 10));
        val /= base;
    } while (val != 0);

    unsigned total = n + (neg ? 1u : 0u);
    if (neg && pad == '0') fmt_emit(buf, size, pos, '-');
    while (total < width) {
        fmt_emit(buf, size, pos, pad);
        total++;
    }
    if (neg && pad != '0') fmt_emit(buf, size, pos, '-');
    while (n > 0) fmt_emit(buf, size, pos, digits[--n]);
}

int dmk_vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    size_t pos = 0;
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            fmt_emit(buf, size, &pos, *fmt);
            continue;
        }
        fmt++;
        if (*fmt == '%') {
            fmt_emit(buf, size, &pos, '%');
            continue;
        }
        char pad = ' ';
        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }
        unsigned width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10u + (unsigned)(*fmt - '0');
            fmt++;
        }
        bool is_long = false;
        if (*fmt == 'l') {
            is_long = true;
            fmt++;
        }
        if (!*fmt) break;
        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                while (*s) fmt_emit(buf, size, &pos, *s++);
                break;
            }
            case 'c':
                fmt_emit(buf, size, &pos, (char)va_arg(args, int));
                break;
            case 'd': {
                long v = is_long ? va_arg(args, long) : (long)va_arg(args, int);
                bool neg = v < 0;
                unsigned long u = neg ? (unsigned long)(-(v + 1)) + 1UL : (unsigned long)v;
                fmt_uint(buf, size, &pos, u, 10u, false, width, pad, neg);
                break;
            }
            case 'u': {
                unsigned long u = is_long ? va_arg(args, unsigned long) : (unsigned long)va_arg(args, unsigned int);
                fmt_uint(buf, size, &pos, u, 10u, false, width, pad, false);
                break;
            }
            case 'x':
            case 'X': {
                unsigned long u = is_long ? va_arg(args, unsigned long) : (unsigned long)va_arg(args, unsigned int);
                fmt_uint(buf, size, &pos, u, 16u, *fmt == 'X', width, pad, false);
                break;
            }
            default:
                fmt_emit(buf, size, &pos, '%');
                fmt_emit(buf, size, &pos, *fmt);
                break;
        }
    }
    if (size > 0) buf[pos < size ? pos : size - 1] = '\0';
    return (int)pos;
}

int dmk_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = dmk_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return len;
}

void debug_console_printf(const char *fmt, ...) {
    char tmp[128];
    va_list args;
    va_start(args, fmt);
    int len = dmk_vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    if (len > 0) {
        if ((size_t)len > sizeof(tmp) - 1) len = sizeof(tmp) - 1;
        debug_console_write(tmp, len);
    }
}

// Keyboards with matrix-task-produced [CAL] events format them here, in the
// USB task; the matrix task stack cannot carry the printf chain.
__attribute__((weak)) void hhkb_cal_log_flush(void) {}

void debug_console_flush(void) {
#ifdef DEBUG_CONSOLE_ENABLE
    if (!tud_mounted() || tud_suspended()) return;
    if (!tud_hid_n_ready(ITF_NUM_CONSOLE)) return;

#ifdef HHKB_COMPACT_STORAGE
    extern void hhkb_debug_flush_sample(void);
    if (s_console_head == s_console_tail) hhkb_debug_flush_sample();
#endif
    if (s_console_head == s_console_tail) hhkb_cal_log_flush();
    if (s_console_head == s_console_tail) return;

    uint8_t packet[32];
    memset(packet, 0, sizeof(packet));

    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (running) taskENTER_CRITICAL();

    size_t count = 0;
    while (s_console_tail != s_console_head && count < sizeof(packet)) {
        packet[count++] = (uint8_t)s_console_buf[s_console_tail];
        s_console_tail = (s_console_tail + 1) % CONSOLE_BUF_SIZE;
    }

    if (running) taskEXIT_CRITICAL();

    if (count > 0) {
        tud_hid_n_report(ITF_NUM_CONSOLE, 0, packet, sizeof(packet));
    }
#endif
}

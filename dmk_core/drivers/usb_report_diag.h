#ifndef DMK_USB_REPORT_DIAG_H
#define DMK_USB_REPORT_DIAG_H
#include <stdint.h>
typedef struct {
    uint32_t submitted, completed, busy, dropped, resync;
    uint32_t queued;
} usb_report_diag_t;
#ifdef HHKB_USB_REPORT_DIAGNOSTICS
void usb_report_diag_complete(uint8_t instance, uint8_t const *report, uint16_t len);
void usb_report_diag_snapshot(usb_report_diag_t *out);
#endif
#endif

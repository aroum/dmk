#ifndef HHKB_PFU_CHANNEL_H
#define HHKB_PFU_CHANNEL_H

#include <stdint.h>

/*
 * The stock HHKB Hybrid's vendor HID channel (usage page 0xFF00, 64-byte reports), as far as a firmware
 * update needs it: a flasher or the official tool finds the keyboard, reads its identity and asks it to
 * hand over to PFU's Bank 1 updater (command 0xE0), so SW6 is not the only way back.
 *
 *   request  AA AA <cmd> <chunk> <len> <payload...>
 *   reply    55 55 <cmd> <status> <chunk> <len> <payload...>    status 0 = ok, 1 = refused
 */

/* The vendor interface's HID report descriptor (usb_descriptors.c). */
extern const uint8_t pfu_channel_report_desc[34];

/* One output report from the host on interface `itf` (USB task). */
void pfu_channel_request(uint8_t itf, const uint8_t *buf, uint16_t len);
/* Sends what is waiting and runs a pending handoff (USB task, often). */
void pfu_channel_task(void);

#endif

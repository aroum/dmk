/*
 * The stock HHKB Hybrid's vendor channel, for firmware updates (pfu_channel.h).
 *
 * Commands:
 *   0x01 NotifyApplicationState    acknowledged
 *   0x02 GetKeyboardInformation    type, revision and serial from this keyboard's EEPROM (where the
 *                                  factory wrote them), DMK's own version, boot version, running state 0
 *   0xE0 FirmupModeChange          acknowledged, then the stock handoff: Bank 1's updater starts and
 *                                  takes the update (E1-E3) itself
 * Every other command is refused (status 1); an unknown one is answered 55 55 99 01, as stock does.
 * Nothing here writes the keymap or settings; the handoff writes the three EEPROM words stock writes.
 */
#include "config.h"

#if defined(HHKB_HYBRID) && HHKB_PFU_CHANNEL

#include <string.h>

#include "FreeRTOS.h"
#include "pfu_channel.h"
#include "stm32l0xx.h"
#include "task.h"
#include "tusb.h"

const uint8_t pfu_channel_report_desc[34] = {
    0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01,                                     /* vendor page 0xFF00, usage 1 */
    0x09, 0x02, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x40, 0x81, 0x02, /* 64-byte input */
    0x09, 0x03, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x40, 0x91, 0x02, /* 64-byte output */
    0xC0};

/* Identity written by the factory (HHKB Hybrid EEPROM). */
#define EE_TYPE 0x08080310UL     /* 20 bytes, "PD-KB820WS" */
#define EE_REVISION 0x08080324UL /* 4 bytes, "A8" */
#define EE_SERIAL 0x08080328UL   /* 16 bytes */
#define EE_BOOT 0x08080340UL     /* 4 bytes, the updater's version */

/* The version DMK reports: below every stock release, so the official tool offers stock as an update. */
static const uint8_t DMK_APP_VERSION[4] = {0x0A, 0x00, 0x00, 0x00};

static uint8_t s_tx[64];
static volatile uint8_t s_tx_pending;
static uint8_t s_itf; /* the vendor interface's number (usb_descriptors.c) */
static volatile uint8_t s_handoff;
static TickType_t s_handoff_at;

static void eeprom_write_word(uint32_t addr, uint32_t value) {
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEFU;
        FLASH->PEKEYR = 0x02030405U;
    }
    while (FLASH->SR & FLASH_SR_BSY) {}
    *(volatile uint32_t *)addr = value;
    while (FLASH->SR & FLASH_SR_BSY) {}
    FLASH->PECR |= FLASH_PECR_PELOCK;
}

/* The stock A2.49 handoff: clear the Bank 2 boot flag and its companion word, set the update request,
 * reset. Bank 1 starts in updater mode. Bank 2 is not erased: if Bank 1 ignored the flags it would
 * boot this image again. */
static void enter_stock_updater(void) {
    taskDISABLE_INTERRUPTS();
    eeprom_write_word(0x08080360U, 0x00000000U);
    eeprom_write_word(0x08080370U, 0x00000000U);
    eeprom_write_word(0x08081350U, 0x00000001U);
    NVIC_SystemReset();
}

static uint8_t *reply(uint8_t cmd, uint8_t status, uint8_t len) {
    memset(s_tx, 0, sizeof s_tx);
    s_tx[0] = 0x55;
    s_tx[1] = 0x55;
    s_tx[2] = cmd;
    s_tx[3] = status;
    s_tx[5] = len;
    s_tx_pending = 1;
    return s_tx + 6;
}

static void copy_field(uint8_t *dst, uint32_t addr, uint8_t n) {
    const volatile uint8_t *src = (const volatile uint8_t *)addr;
    for (uint8_t i = 0; i < n && src[i] >= 0x20 && src[i] < 0x7F; i++) dst[i] = src[i];
}

void pfu_channel_request(uint8_t itf, const uint8_t *buf, uint16_t len) {
    s_itf = itf;
    if (s_tx_pending || s_handoff || len < 5 || buf[0] != 0xAA || buf[1] != 0xAA) return;
    const uint8_t cmd = buf[2];
    switch (cmd) {
    case 0x01:
        reply(cmd, 0, 0);
        break;
    case 0x02: {
        uint8_t *r = reply(cmd, 0, 0x39) - 6;
        copy_field(r + 6, EE_TYPE, 20);
        copy_field(r + 26, EE_REVISION, 4);
        copy_field(r + 30, EE_SERIAL, 16);
        memcpy(r + 46, DMK_APP_VERSION, 4);
        memcpy(r + 54, (const void *)EE_BOOT, 4);
        r[62] = 0x00; /* running: the application */
        break;
    }
    case 0xE0:
        if (buf[3] == 0 && buf[4] == 0) {
            reply(cmd, 0, 0);
            s_handoff = 1;
            s_handoff_at = xTaskGetTickCount() + pdMS_TO_TICKS(50); /* let the ack reach the host */
        } else {
            reply(cmd, 1, 0);
        }
        break;
    case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x86: case 0x87:
    case 0xD0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
        reply(cmd, 1, 0); /* the keymap and settings are DMK's (Vial); update data goes to Bank 1 */
        break;
    default:
        reply(0x99, 1, 0);
        break;
    }
    pfu_channel_task();
}

void pfu_channel_task(void) {
    if (s_tx_pending && tud_mounted() && tud_hid_n_ready(s_itf) && tud_hid_n_report(s_itf, 0, s_tx, 64))
        s_tx_pending = 0;
    if (s_handoff && !s_tx_pending && (int32_t)(xTaskGetTickCount() - s_handoff_at) >= 0) enter_stock_updater();
}

#endif

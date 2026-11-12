// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdbool.h>

// HAL includes
#include "app_usb_hid.h"

// General C includes
#include "stdio.h"

// Project includes
#include "config.h"
#include "hooks.h"
#include "keys.h"
#include "task_internal.h"
#include "usb.h"

// Standard 6KRO USB HID keyboard report size
#define KEY_REPORT_SIZE 6

// Active keyboard report cache sent to host on change
static USB_HID_KeyboardReport_TypeDef current_report;
static uint8_t s_real_mods = 0;
static uint8_t s_weak_mods = 0;

#ifdef MATRIX_REPORT_BATCH
static bool add_key_to_report(uint8_t keycode);
static bool del_key_from_report(uint8_t keycode);
static uint32_t s_batched_usages[8];
static bool s_batch_pending;

void usb_flush_batched_keys(void) {
    if (!s_batch_pending) return;
    s_batch_pending = false;
    for (unsigned i = 0; i < 8; ++i) s_batched_usages[i] = 0;
    USB_HID_SendReport(&current_report);
}

void usb_process_batched_key(uint16_t keycode, bool pressed) {
    // Keep modifier ordering and non-keyboard reports on the immediate path.
    if (keycode == 0 || keycode >= 0xe0) {
        usb_process_key(keycode, pressed);
        return;
    }
    uint32_t *seen = &s_batched_usages[keycode / 32];
    uint32_t bit = UINT32_C(1) << (keycode % 32);
    if (*seen & bit) usb_flush_batched_keys();
    bool changed = pressed ? add_key_to_report((uint8_t)keycode) : del_key_from_report((uint8_t)keycode);
    if (changed) {
        *seen |= bit;
        s_batch_pending = true;
    }
}
#endif

static bool sync_report_modifiers(void) {
    uint8_t combined = s_real_mods | s_weak_mods;
    if (current_report.Modifier != combined) {
        current_report.Modifier = combined;
        return true;
    }
    return false;
}

static bool add_key_to_report(uint8_t keycode) {
    int empty_idx = -1;
    for (int i = 0; i < KEY_REPORT_SIZE; i++) {
        if (current_report.Keycodes[i] == keycode) {
            return false;
        }
        if (empty_idx < 0 && current_report.Keycodes[i] == 0) {
            empty_idx = i;
        }
    }
    if (empty_idx >= 0) {
        current_report.Keycodes[empty_idx] = keycode;
        return true;
    }
    return false;
}

static bool del_key_from_report(uint8_t keycode) {
    for (int i = 0; i < KEY_REPORT_SIZE; i++) {
        if (current_report.Keycodes[i] == keycode) {
            current_report.Keycodes[i] = 0;
            return true;
        }
    }
    return false;
}

/**
 * @brief Initialize USB HID stack and clear initial keyboard report buffers.
 */
void usb_init(void) {
    USB_HID_Init();

    s_real_mods = 0;
    s_weak_mods = 0;
    current_report.ReportID = 1;
    current_report.Modifier = 0;
    current_report.Reserved = 0;
    for (int i = 0; i < KEY_REPORT_SIZE; i++) {
        current_report.Keycodes[i] = 0;
    }
}

/**
 * @brief Zero-Queue Fast Path: processes a key event, updates the active HID report, and transmits to host.
 * @param keycode 16-bit USB HID / Consumer keycode.
 * @param pressed True for press, false for release.
 */
void usb_process_key(uint16_t keycode, bool pressed) {
#ifdef MATRIX_REPORT_BATCH
    usb_flush_batched_keys();
#endif
    // Handle Consumer / Media keys (Volume, Play/Pause, Brightness, etc.)
    if (keycode & KEY_CONSUMER_FLAG) {
        uint16_t usage = keycode & ~KEY_CONSUMER_FLAG;
        USB_HID_SendConsumerReport(pressed ? usage : 0);
        return;
    }

    bool report_changed = false;

    if (keycode >= 0xE0 && keycode <= 0xE7) {
        uint8_t mod_bit = (uint8_t)(1U << (keycode - 0xE0));
        if (pressed) {
            s_real_mods |= mod_bit;
        } else {
            s_real_mods &= ~mod_bit;
        }
        report_changed = sync_report_modifiers();
    } else if (keycode != 0) {
        report_changed = pressed ? add_key_to_report((uint8_t)keycode) : del_key_from_report((uint8_t)keycode);
    }

    // Only send over USB if report payload changed (saves bus bandwidth)
    if (report_changed) {
        USB_HID_SendReport(&current_report);
    }
}

/**
 * @brief Batch process modifier keys: atomically update modifier bitmask and transmit to host.
 */
void usb_process_modifiers(uint8_t mod_mask, bool pressed) {
#ifdef MATRIX_REPORT_BATCH
    usb_flush_batched_keys();
#endif
    if (mod_mask == 0) {
        return;
    }
    if (pressed) {
        s_real_mods |= mod_mask;
    } else {
        s_real_mods &= ~mod_mask;
    }
    if (sync_report_modifiers()) {
        USB_HID_SendReport(&current_report);
    }
}

/**
 * @brief Batch process modified key: atomically update modifiers and keycode in a single HID report.
 */
void usb_process_key_with_modifiers(uint8_t mod_mask, uint8_t keycode, bool pressed) {
#ifdef MATRIX_REPORT_BATCH
    usb_flush_batched_keys();
#endif
    if (pressed) {
        s_weak_mods |= mod_mask;
    } else {
        s_weak_mods &= ~mod_mask;
    }
    bool mod_changed = sync_report_modifiers();
    bool key_changed = false;
    if (keycode != 0) {
        key_changed = pressed ? add_key_to_report(keycode) : del_key_from_report(keycode);
    }

    if (mod_changed || key_changed) {
        USB_HID_SendReport(&current_report);
    }
}

#ifdef MIDI_USB
#include "tusb.h"

static inline void midi_write(const uint8_t *pkt) {
    tud_midi_packet_write(pkt);
}

void tud_midi_rx_cb(uint8_t itf) {
    (void)itf;
    uint8_t packet[4];
    while (tud_midi_available()) {
        if (tud_midi_packet_read(packet)) {
            hooks_dispatch_midi_receive(packet);
        }
    }
}

void usb_send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x09, 0x90 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    midi_write(packet);
}
void usb_send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x08, 0x80 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    midi_write(packet);
}
void usb_send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val) {
    uint8_t packet[4] = {0x0B, 0xB0 | (chan & 0x0F), cc & 0x7F, val & 0x7F};
    midi_write(packet);
}
void usb_send_midi_pitchbend(uint8_t chan, int16_t val) {
    uint16_t pb = (uint16_t)(val + 8192);
    uint8_t packet[4] = {0x0E, 0xE0 | (chan & 0x0F), pb & 0x7F, (pb >> 7) & 0x7F};
    midi_write(packet);
}
#endif

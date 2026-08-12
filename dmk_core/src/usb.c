// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdbool.h>

// HAL includes
#include "app_usb_hid.h"
#if defined(MCU_milandr)
#include "board_usb.h"
#include "clk.h"
#endif

// General C includes
#include "stdio.h"

// Project includes
#include "keys.h"
#include "task_internal.h"
#include "usb.h"

// Standard 6KRO USB HID keyboard report size
#define KEY_REPORT_SIZE 6

// Active keyboard report cache sent to host on change
static USB_HID_KeyboardReport_TypeDef current_report;

/**
 * @brief Initialize USB HID stack and clear initial keyboard report buffers.
 */
void usb_init(void) {
    USB_HID_Init();
#if defined(MCU_milandr)
    Board_USB_Init(true);
#endif

#ifdef EXTRAKEY_ENABLE
    current_report.ReportID = 1;
#endif
    current_report.Modifier = 0;
    current_report.Reserved = 0;
    for (int i = 0; i < 6; i++) {
        current_report.Keycodes[i] = 0;
    }
}

/**
 * @brief FreeRTOS task handling the USB event queue, updating HID reports, and transmitting to host.
 */
void usb_task(void *pvParameters) {
    (void)pvParameters;
    usb_init();

    while (1) {
        key_event_t event;
        if (pdTRUE == xQueueReceive(usb_queue, &event, portMAX_DELAY)) {
            // Handle Consumer / Media keys (Volume, Play/Pause, Brightness, etc.)
            if (event.keycode & KEY_CONSUMER_FLAG) {
                uint16_t usage = event.keycode & ~KEY_CONSUMER_FLAG;
                extern USB_Result USB_HID_SendConsumerReport(uint16_t usage);
                USB_HID_SendConsumerReport(event.pressed ? usage : 0);
                continue;
            }

            bool report_changed = false;

            if (event.pressed) {
                // Handle 8 standard HID modifier keys (0xE0..0xE7)
                if (event.keycode >= 0xE0 && event.keycode <= 0xE7) {
                    uint8_t mod_bit = 1 << (event.keycode - 0xE0);
                    if (!(current_report.Modifier & mod_bit)) {
                        current_report.Modifier |= mod_bit;
                        report_changed = true;
                    }
                }
                // Handle standard 6KRO keycodes
                else if (event.keycode != 0) {
                    bool found = false;
                    for (int i = 0; i < 6; i++) {
                        if (current_report.Keycodes[i] == event.keycode) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        for (int i = 0; i < 6; i++) {
                            if (current_report.Keycodes[i] == 0) {
                                current_report.Keycodes[i] = event.keycode;
                                report_changed = true;
                                break;
                            }
                        }
                    }
                }
            } else {
                // Handle modifier key release
                if (event.keycode >= 0xE0 && event.keycode <= 0xE7) {
                    uint8_t mod_bit = 1 << (event.keycode - 0xE0);
                    if (current_report.Modifier & mod_bit) {
                        current_report.Modifier &= ~mod_bit;
                        report_changed = true;
                    }
                }
                // Handle standard keycode release
                else if (event.keycode != 0) {
                    for (int i = 0; i < 6; i++) {
                        if (current_report.Keycodes[i] == event.keycode) {
                            current_report.Keycodes[i] = 0;
                            report_changed = true;
                            break;
                        }
                    }
                }
            }

            // Only send over USB if report payload actually changed (saves bus bandwidth)
            if (report_changed) {
                USB_HID_SendReport(&current_report);
            }
        }
    }
}

#ifdef MIDI_USB
#if defined(MCU_milandr)
extern USB_Result USB_MIDI_SendPacket(const uint8_t *packet);
void usb_send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x09, 0x90 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    USB_MIDI_SendPacket(packet);
}
void usb_send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x08, 0x80 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    USB_MIDI_SendPacket(packet);
}
void usb_send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val) {
    uint8_t packet[4] = {0x0B, 0xB0 | (chan & 0x0F), cc & 0x7F, val & 0x7F};
    USB_MIDI_SendPacket(packet);
}
void usb_send_midi_pitchbend(uint8_t chan, int16_t val) {
    uint16_t pb = (uint16_t)(val + 8192);
    uint8_t packet[4] = {0x0E, 0xE0 | (chan & 0x0F), pb & 0x7F, (pb >> 7) & 0x7F};
    USB_MIDI_SendPacket(packet);
}
#else
#undef KEYBOARD_MODIFIER_LEFTCTRL
#undef KEYBOARD_MODIFIER_LEFTSHIFT
#undef KEYBOARD_MODIFIER_LEFTALT
#undef KEYBOARD_MODIFIER_LEFTGUI
#undef KEYBOARD_MODIFIER_RIGHTCTRL
#undef KEYBOARD_MODIFIER_RIGHTSHIFT
#undef KEYBOARD_MODIFIER_RIGHTALT
#undef KEYBOARD_MODIFIER_RIGHTGUI
#undef HID_USAGE_CONSUMER_PLAY_PAUSE
#undef HID_USAGE_CONSUMER_SCAN_NEXT
#undef HID_USAGE_CONSUMER_SCAN_PREVIOUS
#undef HID_USAGE_CONSUMER_STOP
#undef HID_USAGE_CONSUMER_MUTE
#undef HID_USAGE_CONSUMER_VOLUME_UP
#undef HID_USAGE_CONSUMER_VOLUME_DOWN
#undef HID_USAGE_CONSUMER_FAST_FORWARD
#undef HID_USAGE_CONSUMER_REWIND
#undef HID_USAGE_CONSUMER_EJECT
#undef HID_USAGE_CONSUMER_AL_CC_CONFIG
#undef HID_USAGE_CONSUMER_AL_EMAIL
#undef HID_USAGE_CONSUMER_AL_CALCULATOR
#undef HID_USAGE_CONSUMER_AL_LOCAL_BROWSER
#undef HID_USAGE_CONSUMER_AC_SEARCH
#undef HID_USAGE_CONSUMER_AC_HOME
#undef HID_USAGE_CONSUMER_AC_BACK
#undef HID_USAGE_CONSUMER_AC_FORWARD
#undef HID_USAGE_CONSUMER_AC_STOP
#undef HID_USAGE_CONSUMER_AC_REFRESH
#undef HID_USAGE_CONSUMER_AC_BOOKMARKS
#include "tusb.h"
void usb_send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x09, 0x90 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    tud_midi_packet_write(packet);
}
void usb_send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel) {
    uint8_t packet[4] = {0x08, 0x80 | (chan & 0x0F), note & 0x7F, vel & 0x7F};
    tud_midi_packet_write(packet);
}
void usb_send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val) {
    uint8_t packet[4] = {0x0B, 0xB0 | (chan & 0x0F), cc & 0x7F, val & 0x7F};
    tud_midi_packet_write(packet);
}
void usb_send_midi_pitchbend(uint8_t chan, int16_t val) {
    uint16_t pb = (uint16_t)(val + 8192);
    uint8_t packet[4] = {0x0E, 0xE0 | (chan & 0x0F), pb & 0x7F, (pb >> 7) & 0x7F};
    tud_midi_packet_write(packet);
}
#endif
#endif

// SPDX-License-Identifier: MIT
// Protocol constants and SPI transaction shape adapted from l3tnun's
// hhkb_hybrid nrf_spi.c and power.c. See LICENSE.protocol for the upstream notice.
#include "hybrid.h"
#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "ec_settings.h"
#include "app_usb_hid.h"
#include "stm32l072xx.h"
#include "task.h"
#include <string.h>

static uint8_t report[8] = {1};
static uint8_t keyboard_queue[8][7];
static uint8_t queue_read;
static uint8_t queue_write;
static volatile uint8_t queue_count;
static volatile bool hello_seen;
static volatile uint8_t control_command;
static volatile bool control_polled;
static uint8_t control_slot;
static uint8_t reply[9];
static volatile uint8_t reply_length;
static uint8_t reply_position;
static bool notification_value;
static uint8_t notification_type;
static volatile uint8_t active_slot = 0xff;
static volatile bool pairing;
static volatile bool pairing_active;
static volatile uint8_t battery_pending;
static volatile bool battery_polled;
static volatile uint8_t battery_reported = 0xff;
static volatile uint32_t battery_display_until;
static volatile uint32_t profile_display_until;
static volatile uint8_t display_slot;
static volatile uint8_t attention_retries;
static volatile bool powered = true;
static volatile bool poweroff_pending;
static volatile bool matrix_sleeping;
static bool button_sleep_wake;

void hhkb_hybrid_set_sleeping(bool sleeping) { matrix_sleeping = sleeping; button_sleep_wake = false; }
static bool off_from_battery;
static bool off_is_deep;
static volatile bool manual_bt_output;
static volatile bool sleep_requested;
static bool auto_select_needed = true;
static bool previous_battery;
static bool source_seen;
static volatile uint32_t power_off_flash_until;
static bool button_was_down;
static bool button_handled;
static uint32_t button_down_since;
static volatile uint8_t *const spi_data = (volatile uint8_t *)&SPI1->DR;

static void start_reply(const uint8_t *payload, uint8_t count) {
    memcpy(reply, payload, count);
    reply_length = count;
    reply_position = 0;
    *spi_data = 0xaa;
    SPI1->CR2 |= SPI_CR2_TXEIE;
}

void SPI1_IRQHandler(void) {
    uint32_t status = SPI1->SR;
    if (status & SPI_SR_OVR) {
        (void)*spi_data;
        (void)SPI1->SR;
    } else if (status & SPI_SR_RXNE) {
        uint8_t command = *spi_data;
        attention_retries = 0; // A responsive radio gets fast requests.
        uint8_t payload[8] = {0};
        if (notification_value) {
            notification_value = false;
            if (notification_type == 0x83 && command == 0x02) {
                pairing = true;
                pairing_active = true;
                active_slot = 0xff;
            } else if (notification_type == 0x83 && command == 0x01) {
                pairing = false;
                pairing_active = false;
                active_slot = 0xff;
            } else if (notification_type == 0x83 && (command & 0x0f) == 0x03 && (command >> 4) < 4) {
                pairing = false;
                pairing_active = false;
                active_slot = command >> 4;
                display_slot = active_slot;
                profile_display_until = xTaskGetTickCountFromISR() + pdMS_TO_TICKS(3000);
                if (battery_reported <= 100) battery_pending = battery_reported + 1;
            }
        } else if (reply_length == 0 && command != 0) {
            switch (command) {
            case 0x83: case 0x85: case 0x61: case 0x93:
                notification_value = true;
                notification_type = command;
                break;
            case 0x01:
                hello_seen = true;
                payload[0] = 0xe5;
                start_reply(payload, 1);
                break;
            case 0x02:
                if (control_command) {
                    payload[0] = (control_command == 0x03 || control_command == 0x04) ? 0x40 :
                                 control_command == 0x06 ? 0x10 : 0x01;
                    control_polled = true;
                } else if (battery_pending) {
                    payload[0] = 0x80;
                    battery_polled = true;
                    battery_pending = 0;
                } else if (queue_count) {
                    memcpy(report + 1, keyboard_queue[queue_read], 7);
                    queue_read = (queue_read + 1) % 8;
                    --queue_count;
                    memcpy(payload, report, sizeof(report));
                } else {
                    memcpy(payload, report, sizeof(report));
                }
                start_reply(payload, sizeof(payload));
                break;
            case 0x05:
                payload[0] = battery_reported;
                battery_polled = false;
                start_reply(payload, 1);
                break;
            case 0x03:
                if (!control_polled || (control_command != 0x03 && control_command != 0x04)) break;
                control_polled = false;
                if (control_command == command) {
                    payload[0] = 1; // Start pairing on the selected slot.
                    payload[1] = control_slot;
                    control_command = 0;
                } else if (control_command == 0x04) {
                    payload[0] = 0x10; // Follow-up idle reply: 00 AA 10 00 on MISO.
                    control_command = 0;
                }
                start_reply(payload, 2);
                break;
            case 0x06:
                if (!control_polled || control_command != 0x06) break;
                control_polled = false;
                if (control_command == command) {
                    payload[0] = control_slot;
                    control_command = 0;
                }
                start_reply(payload, 1);
                break;
            default:
                break;
            }
        }
    }
    if ((SPI1->CR2 & SPI_CR2_TXEIE) && (SPI1->SR & SPI_SR_TXE)) {
        if (reply_position < reply_length) {
            *spi_data = reply[reply_position++];
        } else {
            *spi_data = 0;
            reply_length = 0;
            SPI1->CR2 &= ~SPI_CR2_TXEIE;
        }
    }
}

void hhkb_hybrid_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN |
                   RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIODEN;
    GPIOA->AFR[1] &= ~(15UL << 28);
    GPIOA->MODER = (GPIOA->MODER & ~(3UL << 30)) | (2UL << 30);
    GPIOB->AFR[0] &= ~(0xfffUL << 12);
    GPIOB->MODER = (GPIOB->MODER & ~(0x3fUL << 6)) | (0x2aUL << 6);
    GPIOB->OSPEEDR |= 3UL << 8;
    GPIOB->MODER &= ~((3UL << 14) | (3UL << 16) | (3UL << 18));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~((3UL << 14) | (3UL << 16) | (3UL << 18))) |
                   (1UL << 14) | (1UL << 16) | (1UL << 18);

    GPIOB->BSRR = 1U << 6;
    GPIOB->MODER = (GPIOB->MODER & ~(3UL << 12)) | (1UL << 12);
    GPIOC->MODER &= ~((3UL << 10) | (3UL << 26));
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(3UL << 10)) | (1UL << 10);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 = SPI_CR1_CPOL | SPI_CR1_CPHA;
    SPI1->CR2 = SPI_CR2_RXNEIE;
    SPI1->CR1 |= SPI_CR1_SPE;
    *spi_data = 0;
    NVIC_EnableIRQ(SPI1_IRQn);

    GPIOD->BRR = 1U << 2;
    GPIOD->MODER = (GPIOD->MODER & ~(3UL << 4)) | (1UL << 4);
}

bool hhkb_hybrid_on_battery(void) { return (GPIOC->IDR & (1U << 13)) != 0; }
bool hhkb_hybrid_button(void) { return (GPIOC->IDR & (1U << 5)) == 0; }
bool hhkb_hybrid_powered(void) { return powered; }
bool hhkb_hybrid_output_bluetooth(void) {
    return hhkb_hybrid_on_battery() || manual_bt_output;
}
bool hhkb_hybrid_sleep_requested(void) { return sleep_requested; }
void hhkb_hybrid_clear_sleep_request(void) { sleep_requested = false; }
void hhkb_hybrid_request_sleep(void) { if (hhkb_hybrid_on_battery()) sleep_requested = true; }

static void release_current_output(void) {
    const USB_HID_KeyboardReport_TypeDef released = {.ReportID = 1};
    USB_HID_SendReport(&released);
}

void hhkb_hybrid_show_profile(void) {
    display_slot = active_slot < 4 ? active_slot : hhkb_bt_last_slot();
    profile_display_until = xTaskGetTickCount() + pdMS_TO_TICKS(3000);
}

void hhkb_hybrid_toggle_output(void) {
    release_current_output();
    if (hhkb_hybrid_output_bluetooth() && !hhkb_hybrid_on_battery()) {
        // Give the nRF time to fetch the all-released report before USB takes over.
        for (uint8_t i = 0; i < 40 && queue_count; ++i) {
            hhkb_hybrid_service();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        if (!control_command) { control_polled = false; control_command = 0x04; }
        manual_bt_output = false;
    } else if (!hhkb_hybrid_on_battery()) {
        if (!hello_seen) return;
        manual_bt_output = true;
        hhkb_hybrid_request_slot(hhkb_bt_last_slot(), false);
    } else {
        // On battery USB is unavailable; select USB for the next USB connection.
        manual_bt_output = false;
    }
    release_current_output();
}

void hhkb_hybrid_cycle_profile(void) {
    uint8_t slot = active_slot < 4 ? active_slot : hhkb_bt_last_slot();
    hhkb_hybrid_request_slot((slot + 1) & 3, false);
}

void hhkb_hybrid_pair_last_profile(void) {
    hhkb_hybrid_request_slot(active_slot < 4 ? active_slot : hhkb_bt_last_slot(), true);
}

static void request_power_off(bool deep) {
    if (!powered || poweroff_pending) return; // OFF is woken only by PC5 or connecting USB.
    if (!hhkb_hybrid_on_battery()) return; // Stock button does not power off on USB.
    poweroff_pending = true;
    hhkb_hybrid_refresh_indicators(); // LED owner displays orange now.
    release_current_output();
    if (hhkb_hybrid_output_bluetooth()) {
        for (uint8_t i = 0; i < 40 && queue_count; ++i) {
            hhkb_hybrid_service();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    // Ask Nordic to disconnect through the already implemented idle handshake.
    // Keep SPI alive for its follow-up request/confirmation, bounded to 3 s.
    // Do not replace a control request whose follow-up is already in flight.
    for (uint8_t i = 0; i < 100 && control_polled; ++i) {
        hhkb_hybrid_service();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    NVIC_DisableIRQ(SPI1_IRQn);
    control_polled = false;
    control_command = 0x04;
    attention_retries = 0;
    NVIC_EnableIRQ(SPI1_IRQn);
    for (uint16_t i = 0; i < 3000; ++i) {
        if (!hhkb_hybrid_on_battery()) { poweroff_pending = false; return; }
        hhkb_hybrid_service();
        if ((i & 0x1ffU) == 0) IWDG->KR = 0xAAAA;
        if (i >= 1500 && !control_command && !control_polled && active_slot == 0xff && !reply_length &&
            !(SPI1->SR & SPI_SR_BSY)) break;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    off_from_battery = hhkb_hybrid_on_battery();
    off_is_deep = deep;
    powered = false;
    poweroff_pending = false;
    power_off_flash_until = xTaskGetTickCount();
#ifndef HHKB_STOCK_POWER_OFF
    GPIOD->BSRR = 1U << 2;
#endif
}
void hhkb_hybrid_toggle_power(void) { request_power_off(false); }
void hhkb_hybrid_enter_deep_sleep(void) { request_power_off(true); }
bool hhkb_hybrid_off_is_deep(void) { return off_is_deep; }
#ifdef HHKB_STOCK_POWER_OFF
void hhkb_hybrid_resume_after_stop(void) {
    powered = true;
    poweroff_pending = false;
    matrix_sleeping = false;
    sleep_requested = false;
    button_sleep_wake = false;
    button_was_down = false;
    button_handled = false;
    control_command = 0;
    control_polled = false;
    queue_count = queue_read = queue_write = 0;
    reply_length = reply_position = 0;
    notification_value = false;
    auto_select_needed = true;
    attention_retries = 0;
    active_slot = 0xff;
    pairing = pairing_active = false;
    memset(report, 0, sizeof(report));
    SPI1->CR2 &= ~SPI_CR2_TXEIE;
    *spi_data = 0;
}
#endif

bool hhkb_hybrid_take_button_wake(void) {
    bool wake = button_sleep_wake;
    button_sleep_wake = false;
    return wake;
}
uint32_t hhkb_hybrid_button_wait_ms(uint32_t now_ms) {
    if (!button_was_down || button_handled) return UINT32_MAX;
    uint32_t held = now_ms - button_down_since;
    return held >= 2000 ? 1 : 2000 - held;
}

static void button_short_action(void) {
    switch (hhkb_bt_button_action(false)) {
    case 1: hhkb_hybrid_show_profile(); break;
    case 2: hhkb_hybrid_show_battery(); break;
    case 3: hhkb_hybrid_request_sleep(); break;
    case 4: hhkb_hybrid_cycle_profile(); break;
    case 5: hhkb_hybrid_toggle_output(); break;
    default: break;
    }
}

static void button_long_action(void) {
    switch (hhkb_bt_button_action(true)) {
    case 0: hhkb_hybrid_toggle_power(); break;
    case 1: hhkb_hybrid_request_sleep(); break;
    case 2: hhkb_hybrid_pair_last_profile(); break;
    }
}

void hhkb_hybrid_button_poll(uint32_t now_ms) {
    if (!powered && off_from_battery && !hhkb_hybrid_on_battery()) {
        NVIC_SystemReset(); // USB insertion wakes OFF, as in the reference.
    }
    bool down = hhkb_hybrid_button();
    if (down && !button_was_down) {
        button_down_since = now_ms;
        button_handled = false;
    }
    if (down && !button_handled) {
        uint32_t held = now_ms - button_down_since;
        if (!powered && held >= 1000) {
            NVIC_SystemReset(); // Bootloader restarts the nRF and DMK cleanly.
        } else if (powered && held >= 2000) {
            button_long_action();
            button_handled = true;
        }
    }
    if (!down && button_was_down && !button_handled && powered) {
        uint32_t held = now_ms - button_down_since;
        if (held >= 2000) button_long_action();
        else if (matrix_sleeping) {
            if (held >= 100) button_sleep_wake = true;
        } else if (held <= 500) button_short_action();
    }
    if (!down) button_handled = false;
    button_was_down = down;
}
uint8_t hhkb_hybrid_battery_level(void) {
    if (!hhkb_hybrid_on_battery()) return 100;
    uint32_t pins = GPIOB->IDR;
    if (!(pins & (1U << 9))) return 0;
    if (!(pins & (1U << 8))) return 5;
    if (!(pins & (1U << 7))) return 15;
    return 100;
}
uint8_t hhkb_hybrid_active_slot(void) { return active_slot; }
bool hhkb_hybrid_pairing(void) { return pairing; }
void hhkb_hybrid_show_battery(void) {
    if (hhkb_bt_indicator_flags() & 1)
        battery_display_until = xTaskGetTickCount() + pdMS_TO_TICKS(4000);
}
uint32_t hhkb_hybrid_indicator_wait_ms(uint32_t now_ms) {
    if (matrix_sleeping) return 1000;
    if (!powered) return 50;
    uint8_t flags = hhkb_bt_indicator_flags();
    if (((flags & 1) && (int32_t)(battery_display_until - now_ms) > 0) ||
        ((flags & 8) && (int32_t)(profile_display_until - now_ms) > 0) ||
        ((flags & 4) && pairing) ||
        ((flags & 2) && hhkb_hybrid_battery_level() <= 15)) return 50;
    // Poll unsolicited radio/host status at 4 Hz; key events wake the core sooner.
    return 250;
}

bool hhkb_hybrid_led_override(uint8_t index, uint32_t now_ms, bool *on) {
    if (!powered) {
        *on = index == 1 && (int32_t)(power_off_flash_until - now_ms) > 0;
        return true;
    }
    if (poweroff_pending) {
        *on = index == 1;
        return true;
    }
    if (matrix_sleeping) {
        *on = false;
        return true;
    }
    uint8_t flags = hhkb_bt_indicator_flags();
    if (index == 1) {
        if ((flags & 1) && (int32_t)(battery_display_until - now_ms) > 0) {
            uint8_t level = hhkb_hybrid_battery_level();
            uint8_t flashes = level == 0 ? 1 : level == 5 ? 2 : level == 15 ? 3 : 4;
            uint32_t phase = (4000 - (battery_display_until - now_ms)) % 4000;
            *on = phase < flashes * 400 && phase % 400 < 200;
            return true;
        }
        if ((flags & 2) && hhkb_hybrid_on_battery() && hhkb_hybrid_battery_level() <= 15) {
            uint8_t level = hhkb_hybrid_battery_level();
            uint32_t period = level == 15 ? 30000 : 15000;
            uint32_t phase = now_ms % period;
            *on = phase < 150 || (level < 15 && phase >= 400 && phase < 550);
            return true;
        }
    } else if (index == 0) {
        if ((flags & 4) && pairing) {
            uint32_t period = pairing_active ? 250 : 500;
            *on = (now_ms % period) < period / 2;
            return true;
        }
        if ((flags & 8) && (int32_t)(profile_display_until - now_ms) > 0) {
            uint32_t phase = (3000 - (profile_display_until - now_ms)) % 3000;
            *on = phase < (display_slot + 1) * 400 && phase % 400 < 200;
            return true;
        }
    }
    return false;
}

void hhkb_hybrid_report(uint8_t modifiers, const uint8_t keys[6]) {
    NVIC_DisableIRQ(SPI1_IRQn);
    if (!control_command) {
        if (queue_count == 8) {
            // Keep the latest state, including releases, when the radio stalls.
            queue_read = (queue_read + 1) % 8;
            --queue_count;
        }
        keyboard_queue[queue_write][0] = modifiers;
        memcpy(keyboard_queue[queue_write] + 1, keys, 6);
        queue_write = (queue_write + 1) % 8;
        ++queue_count;
        attention_retries = 0;
    }
    NVIC_EnableIRQ(SPI1_IRQn);
}

bool hhkb_hybrid_request_slot(uint8_t slot, bool pair) {
    if (slot > 3 || !hello_seen || control_command) return false;
    release_current_output();
    if (!hhkb_hybrid_on_battery()) manual_bt_output = true;
    NVIC_DisableIRQ(SPI1_IRQn);
    bool accepted = control_command == 0;
    if (accepted) {
        memset(keyboard_queue[0], 0, sizeof(keyboard_queue[0]));
        queue_read = 0;
        queue_write = 1;
        queue_count = 1;
        attention_retries = 0;
        control_slot = slot;
        control_polled = false;
        control_command = pair ? 0x03 : 0x06;
        pairing = pair;
        pairing_active = false;
        display_slot = slot;
        profile_display_until = xTaskGetTickCount() + pdMS_TO_TICKS(3000);
    }
    NVIC_EnableIRQ(SPI1_IRQn);
    if (accepted) {
        auto_select_needed = false;
        hhkb_bt_set_last_slot(slot);
    }
    return accepted;
}

void hhkb_hybrid_service(void) {
    if (!powered) return;
    bool battery = hhkb_hybrid_on_battery();
    if (!source_seen) {
        source_seen = true;
        previous_battery = battery;
    } else if (battery != previous_battery) {
        previous_battery = battery;
        if (battery) {
            auto_select_needed = true;
        } else if (!manual_bt_output) {
            static const uint8_t released[6] = {0};
            hhkb_hybrid_report(0, released);
            if (!control_command) { control_polled = false; control_command = 0x04; }
        }
    }
    if (battery && auto_select_needed && hello_seen && !control_command)
        hhkb_hybrid_request_slot(hhkb_bt_last_slot(), false);
    if (active_slot < 4 && active_slot != hhkb_bt_last_slot())
        hhkb_bt_set_last_slot(active_slot);
    static TickType_t last_request;
    static TickType_t last_battery_sample;
    static uint8_t battery_candidate = 0xff, stable_samples;
    TickType_t now = xTaskGetTickCount();
    if (now - last_battery_sample >= pdMS_TO_TICKS(1000)) {
        last_battery_sample = now;
        uint8_t level = hhkb_hybrid_battery_level();
        stable_samples = level == battery_candidate ? (stable_samples < 3 ? stable_samples + 1 : 3) : 1;
        battery_candidate = level;
        if (stable_samples == 3 && level != battery_reported) {
            battery_reported = level;
            battery_pending = level + 1;
        }
    }
    if (!hello_seen || (!queue_count && !control_command && !battery_pending) ||
        now - last_request < pdMS_TO_TICKS(attention_retries < 5 ? 5 : 100)) return;
    last_request = now;
    if (attention_retries < 5) ++attention_retries;
    GPIOB->BRR = 1U << 6;
    hal_sleep_us(500);
    GPIOB->BSRR = 1U << 6;
}

static bool process_profile_key(uint32_t key, bool pressed) {
    enum {
        SELECT_FIRST = 0x7e02, PAIR_FIRST = 0x7e06, SHOW_BATTERY = 0x7e0a,
        POWER = 0x7e0b, SLEEP = 0x7e0c, PAIR_LAST = 0x7e0d,
        SHOW_PROFILE = 0x7e0e, CYCLE = 0x7e0f, TOGGLE_OUTPUT = 0x7e10,
        END = 0x7e11
    };
    if (key < SELECT_FIRST || key >= END) return false;
    if (pressed) {
        if (key < SHOW_BATTERY) {
            bool pair = key >= PAIR_FIRST;
            hhkb_hybrid_request_slot(key - (pair ? PAIR_FIRST : SELECT_FIRST), pair);
        } else {
            switch (key) {
            case SHOW_BATTERY: hhkb_hybrid_show_battery(); break;
            case POWER: hhkb_hybrid_toggle_power(); break;
            case SLEEP: hhkb_hybrid_request_sleep(); break;
            case PAIR_LAST: hhkb_hybrid_pair_last_profile(); break;
            case SHOW_PROFILE: hhkb_hybrid_show_profile(); break;
            case CYCLE: hhkb_hybrid_cycle_profile(); break;
            case TOGGLE_OUTPUT: hhkb_hybrid_toggle_output(); break;
            }
        }
    }
    return true;
}

DMK_HOOK_SUBSCRIBER(hhkb_hybrid_hooks, .process_key = process_profile_key);

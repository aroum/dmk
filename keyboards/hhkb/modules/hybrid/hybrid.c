// SPDX-License-Identifier: MIT
// Protocol constants and SPI transaction shape adapted from l3tnun's
// hhkb_hybrid nrf_spi.c. See LICENSE.protocol for the upstream notice.
#include "hybrid.h"
#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
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
static uint8_t control_slot;
static uint8_t reply[9];
static uint8_t reply_length;
static uint8_t reply_position;
static bool notification_value;
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
        uint8_t payload[8] = {0};
        if (notification_value) {
            notification_value = false;
        } else if (reply_length == 0 && command != 0) {
            switch (command) {
            case 0x83: case 0x85: case 0x61: case 0x93:
                notification_value = true;
                break;
            case 0x01:
                hello_seen = true;
                payload[0] = 0xe5;
                start_reply(payload, 1);
                break;
            case 0x02:
                if (queue_count) {
                    memcpy(report + 1, keyboard_queue[queue_read], 7);
                    queue_read = (queue_read + 1) % 8;
                    --queue_count;
                    memcpy(payload, report, sizeof(report));
                } else if (!control_command) {
                    memcpy(payload, report, sizeof(report));
                } else {
                    payload[0] = control_command == 0x03 ? 0x40 :
                                 control_command == 0x06 ? 0x10 : 0x01;
                }
                start_reply(payload, sizeof(payload));
                break;
            case 0x03:
                if (control_command == command) {
                    payload[0] = 1; // Start pairing on the selected slot.
                    payload[1] = control_slot;
                    control_command = 0;
                }
                start_reply(payload, 2);
                break;
            case 0x06:
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
    }
    NVIC_EnableIRQ(SPI1_IRQn);
}

bool hhkb_hybrid_request_slot(uint8_t slot, bool pair) {
    if (slot > 3 || !hello_seen) return false;
    NVIC_DisableIRQ(SPI1_IRQn);
    bool accepted = control_command == 0;
    if (accepted) {
        memset(keyboard_queue[0], 0, sizeof(keyboard_queue[0]));
        queue_read = 0;
        queue_write = 1;
        queue_count = 1;
        control_slot = slot;
        control_command = pair ? 0x03 : 0x06;
    }
    NVIC_EnableIRQ(SPI1_IRQn);
    return accepted;
}

void hhkb_hybrid_service(void) {
    static TickType_t last_request;
    TickType_t now = xTaskGetTickCount();
    if (!hello_seen || (!queue_count && !control_command) ||
        now - last_request < pdMS_TO_TICKS(5)) return;
    last_request = now;
    GPIOB->BRR = 1U << 6;
    hal_sleep_us(500);
    GPIOB->BSRR = 1U << 6;
}

static bool process_profile_key(uint32_t key, bool pressed) {
    enum { SELECT_FIRST = 0x7e02, PAIR_FIRST = 0x7e06, END = 0x7e0a };
    if (key < SELECT_FIRST || key >= END) return false;
    if (pressed) {
        bool pair = key >= PAIR_FIRST;
        hhkb_hybrid_request_slot(key - (pair ? PAIR_FIRST : SELECT_FIRST), pair);
    }
    return true;
}

DMK_HOOK_SUBSCRIBER(hhkb_hybrid_hooks, .process_key = process_profile_key);

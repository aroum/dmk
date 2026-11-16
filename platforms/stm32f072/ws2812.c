#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include "stm32f0xx.h"
#include "config.h"
#include <stdbool.h>
#include <string.h>

#ifndef WS2812_MAX_LEDS
#if defined(RGB_NUM)
#define WS2812_MAX_LEDS RGB_NUM
#else
#define WS2812_MAX_LEDS 16
#endif
#endif

// Two trailing zero samples let the CCR preload latch LOW before DMA completes.
#define WS2812_DMA_BUF_LEN (WS2812_MAX_LEDS * 24 + 2)

static uint16_t ws2812_dma_buf[WS2812_DMA_BUF_LEN];
static uint32_t ws2812_colors[WS2812_MAX_LEDS];
static uint32_t ws2812_len = 0;
static uint32_t ws2812_pin = 0;
static volatile bool ws2812_dma_busy = false;

// -----------------------------------------------------------------------------
// PA15 -> TIM2_CH1 (AF2), with TIM2_UP requests on DMA1 Channel 2.
// -----------------------------------------------------------------------------

void DMA1_Channel2_3_IRQHandler(void) {
    if (DMA1->ISR & (DMA_ISR_TCIF2 | DMA_ISR_TEIF2)) {
        DMA1->IFCR = DMA_IFCR_CGIF2;
        DMA1_Channel2->CCR &= ~DMA_CCR_EN;

        // Stop timer counter and keep output LOW
        TIM2->CR1 &= ~TIM_CR1_CEN;
        TIM2->DIER = 0;
        TIM2->CCR1 = 0;
        TIM2->EGR = TIM_EGR_UG;

        ws2812_dma_busy = false;
    }
}

static void ws2812_init_tim2_dma(void) {
    // Enable peripheral clocks: GPIOA, DMA1, TIM2
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configure PA15 as Alternate Function 2 (TIM2_CH1), Push-Pull, High-Speed
    GPIOA->MODER &= ~(3U << (15 * 2));
    GPIOA->MODER |= (2U << (15 * 2)); // AF mode
    GPIOA->AFR[1] &= ~(0xFU << ((15 - 8) * 4));
    GPIOA->AFR[1] |= (2U << ((15 - 8) * 4)); // AF2: TIM2_CH1
    GPIOA->OSPEEDR |= (3U << (15 * 2));      // High speed
    GPIOA->OTYPER &= ~(1U << 15);            // Push-pull
    GPIOA->PUPDR &= ~(3U << (15 * 2));       // No pull

    // Configure TIM2: 48 MHz clock, 800 kHz PWM (1.25 us period)
    TIM2->CR1 = 0;
    TIM2->PSC = 0;  // 48 MHz tick
    TIM2->ARR = 59; // Period = (59 + 1) / 48 MHz = 1.25 us
    TIM2->CCR1 = 0; // Default duty 0 (LOW)

    // PWM Mode 1 on Channel 1, preload enabled
    TIM2->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM2->CCER = TIM_CCER_CC1E;  // Active high, enable output
    TIM2->DIER = 0;
    TIM2->CR1 |= TIM_CR1_ARPE;   // Preload ARR

    DMA1_Channel2->CCR = 0;
    DMA1_Channel2->CPAR = (uint32_t)&TIM2->CCR1;
    DMA1_Channel2->CMAR = (uint32_t)ws2812_dma_buf;
    DMA1_Channel2->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 |
                         DMA_CCR_PL_1 | DMA_CCR_TCIE | DMA_CCR_TEIE;

    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3);
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

static void ws2812_show_tim2_dma(void) {
    // Wait for any previous DMA transfer to complete (< 330 us)
    while (ws2812_dma_busy) {
        __NOP();
    }
    hal_sleep_us(350);

    uint32_t bit_idx = 0;
    for (uint32_t i = 0; i < ws2812_len; i++) {
        uint32_t color = ws2812_colors[i];
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t b = color & 0xFF;
        uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

        // WS2812 protocol: MSB first
        // Bit 1: CCR1 = 36 (750 ns HIGH, 500 ns LOW)
        // Bit 0: CCR1 = 18 (375 ns HIGH, 875 ns LOW)
        for (int bit = 23; bit >= 0; bit--) {
            ws2812_dma_buf[bit_idx++] = (grb & (1U << bit)) ? 36 : 18;
        }
    }

    // The first zero becomes active before the final DMA write raises TC.
    ws2812_dma_buf[bit_idx++] = 0;
    ws2812_dma_buf[bit_idx++] = 0;

    ws2812_dma_busy = true;

    // Set transfer length and trigger DMA
    DMA1_Channel2->CNDTR = bit_idx;
    DMA1_Channel2->CCR |= DMA_CCR_EN;

    TIM2->DIER = TIM_DIER_UDE;
    TIM2->CR1 |= TIM_CR1_CEN;
}

// -----------------------------------------------------------------------------
// Fallback GPIO Bit-Bang Implementation (for non-PA15 pins on STM32F072)
// -----------------------------------------------------------------------------

#define NOP1   __asm__ volatile("nop")
#define NOP2   NOP1; NOP1
#define NOP4   NOP2; NOP2
#define NOP8   NOP4; NOP4
#define NOP11  NOP8; NOP2; NOP1
#define NOP22  NOP8; NOP8; NOP4; NOP2
#define NOP26  NOP8; NOP8; NOP8; NOP2
#define NOP39  NOP26; NOP11; NOP2

static uint8_t ws2812_fallback_stream[WS2812_MAX_LEDS * 3];

__attribute__((section(".RamFunc"), noinline))
static void ws2812_send_stream(volatile uint32_t *bsrr, uint32_t set_mask, uint32_t reset_mask,
                               const uint8_t *data, uint32_t len) {
    while (len--) {
        uint8_t byte = *data++;
        for (int i = 0; i < 8; i++) {
            if (byte & 0x80) {
                *bsrr = set_mask;
                NOP26;
                *bsrr = reset_mask;
                NOP22;
            } else {
                *bsrr = set_mask;
                NOP11;
                *bsrr = reset_mask;
                NOP39;
            }
            byte <<= 1;
        }
    }
}

// -----------------------------------------------------------------------------
// Public Platform WS2812 Driver Interface
// -----------------------------------------------------------------------------

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_pin = pin;
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    memset(ws2812_colors, 0, sizeof(ws2812_colors));

    if (pin == PA15) {
        ws2812_init_tim2_dma();
    } else {
        hal_gpio_init((pin_t)pin);
        hal_gpio_set_dir((pin_t)pin, true);
        hal_gpio_put((pin_t)pin, false);
    }
    hal_sleep_us(400); // WS2812 reset latch
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;
    ws2812_colors[index] = color;
}

void ws2812_show(void) {
    if (ws2812_len == 0)
        return;

    if (ws2812_pin == PA15) {
        // Hardware TIM2 + DMA: 0 CPU overhead, 0 interrupt lock time
        ws2812_show_tim2_dma();
        return;
    }

    // Fallback: cycle-accurate GPIO bitbang
    for (uint32_t i = 0; i < ws2812_len; i++) {
        uint32_t color = ws2812_colors[i];
        ws2812_fallback_stream[i * 3 + 0] = (color >> 8) & 0xFF;  // Green
        ws2812_fallback_stream[i * 3 + 1] = (color >> 16) & 0xFF; // Red
        ws2812_fallback_stream[i * 3 + 2] = color & 0xFF;         // Blue
    }

    uint8_t port_idx = (uint8_t)(ws2812_pin >> 4);
    GPIO_TypeDef *port = (port_idx == 0) ? GPIOA :
                         (port_idx == 1) ? GPIOB :
                         (port_idx == 2) ? GPIOC :
                         (port_idx == 3) ? GPIOD : GPIOF;
    uint32_t pin_mask = 1U << (ws2812_pin & 0x0F);
    uint32_t set_mask = pin_mask;
    uint32_t reset_mask = pin_mask << 16;

    // Note: Reset pause is performed BEFORE disabling interrupts
    port->BSRR = reset_mask;
    hal_sleep_us(350);

    taskENTER_CRITICAL();
    ws2812_send_stream(&port->BSRR, set_mask, reset_mask, ws2812_fallback_stream, ws2812_len * 3);
    port->BSRR = reset_mask;
    taskEXIT_CRITICAL();
}

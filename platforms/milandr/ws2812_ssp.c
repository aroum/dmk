#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_ssp.h"
#include "MDR32FxQI_dma.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pin_defs.h"
#include "hal_gpio.h"
#include <stdbool.h>
#include <string.h>

#define WS2812_MAX_LEDS 64
static uint16_t _rgb[WS2812_MAX_LEDS * 6];
static uint32_t ws2812_len = 0;
static MDR_SSP_TypeDef *active_ssp = MDR_SSP1;
static uint32_t active_ssp_idx = 1;
volatile uint32_t ws2812_dma_timeouts;

extern uint32_t SystemCoreClock;

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    memset(_rgb, 0, sizeof(_rgb));

    // Determine which SSP and pin configuration to use
    // PA15: SSP1 (Main function)
    // PF0: SSP1 (Alternative function)
    // PD3: SSP2 (Main function)
    // PD15: SSP2 (Alternative function)

    SSP_InitTypeDef ssp_init;
    ssp_init.SSP_CPSDVSR = (SystemCoreClock / 3000000) & 0xFE; // 3 MHz
    ssp_init.SSP_SCR = 0;
    ssp_init.SSP_Mode = SSP_ModeMaster;
    ssp_init.SSP_WordLength = SSP_WordLength16b;
    ssp_init.SSP_SPH = SSP_SPH_1Edge;
    ssp_init.SSP_SPO = SSP_SPO_Low;
    ssp_init.SSP_FRF = SSP_FRF_SPI_Motorola;
    ssp_init.SSP_HardwareFlowControl = SSP_HardwareFlowControl_SSE;

    PORT_InitTypeDef port_init;
    PORT_StructInit(&port_init);
    port_init.PORT_MODE = PORT_MODE_DIGITAL;
    port_init.PORT_OE = PORT_OE_OUT;
    port_init.PORT_SPEED = PORT_SPEED_MAXFAST;
    port_init.PORT_PULL_UP = PORT_PULL_UP_OFF;
    port_init.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;

    if (pin == PA15 || pin == PF0) {
        active_ssp = MDR_SSP1;
        active_ssp_idx = 1;

        RST_CLK_PCLKcmd(RST_CLK_PCLK_SSP1, ENABLE);
        SSP_BRGInit(MDR_SSP1, SSP_HCLKdiv1);
        SSP_DeInit(MDR_SSP1);
        SSP_Init(MDR_SSP1, &ssp_init);

        if (pin == PA15) {
            RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA, ENABLE);
            // Configure PA13 (CLK), PA14 (FSS), PA15 (TXD)
            port_init.PORT_Pin = PORT_Pin_13 | PORT_Pin_14 | PORT_Pin_15;
            port_init.PORT_FUNC = PORT_FUNC_MAIN;
            PORT_Init(MDR_PORTA, &port_init);
        } else {
            RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);
            // Configure PF0 (TXD), PF1 (CLK), PF2 (FSS)
            port_init.PORT_Pin = PORT_Pin_0 | PORT_Pin_1 | PORT_Pin_2;
            port_init.PORT_FUNC = PORT_FUNC_ALTER;
            PORT_Init(MDR_PORTF, &port_init);
        }
    } else {
        // Default to SSP2 for PD3, PD15, and any other pin
        active_ssp = MDR_SSP2;
        active_ssp_idx = 2;

        RST_CLK_PCLKcmd(RST_CLK_PCLK_SSP2, ENABLE);
        SSP_BRGInit(MDR_SSP2, SSP_HCLKdiv1);
        SSP_DeInit(MDR_SSP2);
        SSP_Init(MDR_SSP2, &ssp_init);

        RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);
        if (pin == PD3) {
            // Configure PD3 (TXD), PD5 (CLK), PD6 (FSS)
            port_init.PORT_Pin = PORT_Pin_3 | PORT_Pin_5 | PORT_Pin_6;
            port_init.PORT_FUNC = PORT_FUNC_MAIN;
        } else {
            // Configure PD15 (TXD), PD14 (CLK), PD12 (FSS)
            port_init.PORT_Pin = PORT_Pin_12 | PORT_Pin_14 | PORT_Pin_15;
            port_init.PORT_FUNC = PORT_FUNC_ALTER;
        }
        PORT_Init(MDR_PORTD, &port_init);
    }

    SSP_Cmd(active_ssp, ENABLE);
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;

    // Convert 0xRRGGBB color to GRB 4-bit SSP patterns
    // 0x08 is ZERO (0b1000)
    // 0x0C is ONE (0b1100)
    const uint8_t ZERO = 0x08;
    const uint8_t ONE = 0x0C;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
    uint32_t word_idx = index * 6;

    for (int w = 0; w < 6; w++) {
        uint16_t word_val = 0;
        for (int bit = 0; bit < 4; bit++) {
            bool is_one = (grb & 0x800000) != 0;
            grb <<= 1;

            uint16_t pattern = is_one ? ONE : ZERO;
            word_val = (word_val << 4) | pattern;
        }
        _rgb[word_idx + w] = word_val;
    }
}

void ws2812_show(void) {
    if (!ws2812_len) return;
    // rgb.c serializes setters and show(), so DMA owns an immutable frame.
    const uint8_t channel = active_ssp_idx == 1 ? DMA_Channel_SSP1_TX : DMA_Channel_SSP2_TX;
    RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE);
    SSP_DMACmd(active_ssp, SSP_DMA_TXE, DISABLE);
    DMA_Cmd(channel, DISABLE);
    DMA_CtrlDataInitTypeDef data = {
        .DMA_SourceBaseAddr = (uint32_t)_rgb,
        .DMA_DestBaseAddr = (uint32_t)&active_ssp->DR,
        .DMA_SourceIncSize = DMA_SourceIncHalfword,
        .DMA_DestIncSize = DMA_DestIncNo,
        .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
        .DMA_Mode = DMA_Mode_Basic,
        .DMA_CycleSize = ws2812_len * 6,
        .DMA_NumContinuous = DMA_Transfers_1,
    };
    DMA_ChannelInitTypeDef init;
    DMA_StructInit(&init);
    init.DMA_PriCtrlData = &data;
    DMA_Init(channel, &init);
    __DMB();
    SSP_DMACmd(active_ssp, SSP_DMA_TXE, ENABLE);

    // SSP DMA requests keep FIFO full independently of USB/scan interrupts.
    // Poll once per RTOS tick instead of occupying the CPU for the whole frame.
    uint32_t waited_us = 0;
    bool scheduled = xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
    while (MDR_DMA->CHNL_ENABLE_SET & (1u << channel)) {
        if (waited_us >= 20000) {
            SSP_DMACmd(active_ssp, SSP_DMA_TXE, DISABLE);
            DMA_Cmd(channel, DISABLE);
            // Bound an unavailable DMA peripheral; preserve a diagnostic for SWD.
            ++ws2812_dma_timeouts;
            break;
        }
        if (scheduled) {
            vTaskDelay(1);
            waited_us += portTICK_PERIOD_MS * 1000U;
        } else {
            // Also support an initial frame before the scheduler starts.
            hal_sleep_us(10);
            waited_us += 10;
        }
    }
    SSP_DMACmd(active_ssp, SSP_DMA_TXE, DISABLE);
    // DMA completion is not completion of the final bit in the SSP shifter.
    while (SSP_GetFlagStatus(active_ssp, SSP_FLAG_BSY) == SET) {}
    hal_sleep_us(350);
}

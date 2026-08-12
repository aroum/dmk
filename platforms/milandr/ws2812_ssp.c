#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_ssp.h"
#include "pin_defs.h"
#include <stdbool.h>
#include <string.h>

#define WS2812_MAX_LEDS 64
#define RGB_RESET_WORDS 20

static uint16_t _rgb[RGB_RESET_WORDS + WS2812_MAX_LEDS * 6];
static uint32_t ws2812_len = 0;
static MDR_SSP_TypeDef *active_ssp = MDR_SSP1;
static uint32_t active_ssp_idx = 1;

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
    uint32_t word_idx = RGB_RESET_WORDS + index * 6;

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
    __disable_irq();

    for (uint32_t i = 0; i < RGB_RESET_WORDS + ws2812_len * 6; ++i) {
        while (SSP_GetFlagStatus(active_ssp, SSP_FLAG_TNF) == RESET) {
        }
        SSP_SendData(active_ssp, _rgb[i]);
    }

    // Wait for the transmission to finish completely
    while (SSP_GetFlagStatus(active_ssp, SSP_FLAG_TFE) == RESET) {
    }
    while (SSP_GetFlagStatus(active_ssp, SSP_FLAG_BSY) == SET) {
    }

    __enable_irq();
}

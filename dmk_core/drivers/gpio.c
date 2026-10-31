#include "hal_gpio.h"

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

void hal_gpio_init(pin_t gpio) {
    gpio_init(gpio);
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    gpio_set_dir(gpio, is_output);
}



void hal_gpio_pull_down(pin_t gpio) {
    gpio_pull_down(gpio);
}

void hal_gpio_pull_up(pin_t gpio) {
    gpio_pull_up(gpio);
}

void hal_sleep_us(uint32_t us) {
    sleep_us(us);
}

void platform_init(void) {}

void platform_bootloader_jump(void) {
    reset_usb_boot(0, 0);
}

#elif defined(MCU_nrf52840)
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"

void hal_gpio_init(pin_t gpio) {
    (void)gpio;
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    if (is_output) {
        nrf_gpio_cfg_output(gpio);
    } else {
        nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_NOPULL);
    }
}



void hal_gpio_pull_down(pin_t gpio) {
    nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_PULLDOWN);
}

void hal_gpio_pull_up(pin_t gpio) {
    nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_PULLUP);
}

void hal_sleep_us(uint32_t us) {
    nrf_delay_us(us);
}

void platform_init(void) {}

void platform_bootloader_jump(void) {
    NRF_POWER->GPREGRET = 0x57;
    NVIC_SystemReset();
}

#elif defined(MCU_baikal)
#include "bmcu_cru.h"
#include "bmcu_eflash.h"
#include "bmcu_gpio.h"
#include "bmcu_wdt.h"
#include "bsp/board_api.h"

static inline GPIO_TypeDef *get_gpio_port(pin_t pin) {
    if (pin < 16)
        return GPIO0;
    if (pin < 32)
        return GPIO1;
    return GPIO2;
}

static inline uint16_t get_gpio_pin_mask(pin_t pin) {
    return (uint16_t)(1U << (pin % 16));
}

void hal_gpio_init(pin_t gpio) {
    uint8_t port_idx = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    GPIO_TypeDef *port = get_gpio_port(gpio);

    CRU_SetPinAF(port_idx, mask, CRU_PIN_AF_0);
    CRU_EnablePinInput(port_idx, mask);

    GPIO_InitStruct_TypeDef init = {0};
    GPIO_StructInit(&init);
    init.PinMask = mask;
    init.Mode = GPIO_MODE_INPUT;
    GPIO_Init(port, &init);
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    uint8_t port_idx = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    GPIO_TypeDef *port = get_gpio_port(gpio);

    CRU_SetPinAF(port_idx, mask, CRU_PIN_AF_0);
    if (is_output) {
        CRU_DisablePinInput(port_idx, mask);
    } else {
        CRU_EnablePinInput(port_idx, mask);
    }

    GPIO_InitStruct_TypeDef init = {0};
    GPIO_StructInit(&init);
    init.PinMask = mask;
    init.Mode = is_output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    GPIO_Init(port, &init);
}



void hal_gpio_pull_down(pin_t gpio) {
    uint8_t port = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    CRU_SetPinPull(port, mask, CRU_PIN_PULL_DOWN);
}

void hal_gpio_pull_up(pin_t gpio) {
    uint8_t port = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    CRU_SetPinPull(port, mask, CRU_PIN_PULL_UP);
}

void hal_sleep_us(uint32_t us) {
    __delay_us(us);
}

void platform_init(void) {
    /* WDT reset does not disable it automatically. Ensure WDT0 is deinitialized */
    CRU_APB0_EnableClock(CRU_APB0_PERIPH_WDT0);
    WDT_DeInit(WDT0);

    /* Enable GPIO peripheral clocks in CRU */
    CRU_APB0_EnableClock(CRU_APB0_PERIPH_GPIO0);
    CRU_APB1_EnableClock(CRU_APB1_PERIPH_GPIO1);
    CRU_APB2_EnableClock(CRU_APB2_PERIPH_GPIO2);

    /* Prepare eFlash to 200 MHz CCLK. See PLL configuration */
    EFLASH_Init(200000000UL);

    /* Configure System Clocks: PLL = 200MHz, CCLK = 200MHz, PCLK0/1/2 = 100MHz, HCLK = 50MHz */
    CRU_PLL_BypassMode_Disable();
    CRU_PLL_SetSource(CRU_PLL_SRC_C0);
    CRU_PLL_ForceReset();
    CRU_PLL_Config(1UL, 48UL, 6UL, 48UL);
    CRU_PLL_ReleaseReset();
    __delay_cycles(500UL);

    /** Configure CCLK = 200MHz */
    CRU_SetCCLKPrescaler(CRU_CLK_DIV_1);
    CRU_CCLKPrescaler_Enable();
    CRU_SetCCLKSource(CRU_CLK_SRC_PLL);

    /** Configure PCLK0 = 100MHz */
    CRU_SetPCLK0Prescaler(CRU_CLK_DIV_2);
    CRU_PCLK0Prescaler_Enable();
    CRU_SetPCLK0Source(CRU_CLK_SRC_PLL);

    /** Configure PCLK1 = 100MHz */
    CRU_SetPCLK1Prescaler(CRU_CLK_DIV_2);
    CRU_PCLK1Prescaler_Enable();
    CRU_SetPCLK1Source(CRU_CLK_SRC_PLL);

    /** Configure PCLK2 = 100MHz */
    CRU_SetPCLK2Prescaler(CRU_CLK_DIV_2);
    CRU_PCLK2Prescaler_Enable();
    CRU_SetPCLK2Source(CRU_CLK_SRC_PLL);

    /** Configure HCLK = 50MHz */
    CRU_SetHCLKPrescaler(CRU_CLK_DIV_4);
    CRU_HCLKPrescaler_Enable();
    CRU_SetHCLKSource(CRU_CLK_SRC_PLL);

    /** Configure 1MHz clocks for FreeRTOS mtime tick counter */
    CRU_SetCCLK1MHzPrescaler(199UL);
    CRU_SetHCLK1MHzPrescaler(49UL);

    /* Initialize and release reset on USB PHY */
    CRU_USB_PHY_PLL_Enable();
    __delay_cycles(100UL);
    CRU_USB_PHY_ReleaseReset();
    __delay_us(500UL);

    /* Configure CLIC */
    CLIC_Config(1U, 3U);
    CLIC_SetLevelThreshold(0U);

    /* Configure timer interrupt for FreeRTOS */
    CLIC_ConfigIRQ(CLIC_MT_IRQn,
                   CLIC_INTATTR_MODE_MACHINE,
                   1U,
                   1U,
                   CLIC_INTATTR_SHV_VECTORED,
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,
                   CLIC_INTATTR_TRIG_POL_P);

    __enable_irq();
}

void platform_bootloader_jump(void) {
    /* Set TinyUF2 double-tap signature in TCMB */
    *(__IO uint32_t*)TCMB_BASE = 0xf01669efUL;

    /* Reset MCU via WDT0 */
    CRU_APB0_EnableClock(CRU_APB0_PERIPH_WDT0);
    WDT_InitStruct_TypeDef WDT_InitStruct;
    WDT_StructInit(&WDT_InitStruct);
    WDT_InitStruct.Mode = WDT_MODE_RESET;
    WDT_InitStruct.Rstlen = WDT_RST_LEN_32;
    WDT_InitStruct.Timeout = WDT_TIME_PERIOD_10;
    WDT_Init(WDT0, &WDT_InitStruct);
    for (;;) {
        __asm volatile("nop");
    }
}

#elif defined(MCU_milandr)
#include "MDR32FxQI_bkp.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "board_pins.h"
#include "clk.h"
#include <stddef.h>

static MDR_PORT_TypeDef *const ports[] = {MDR_PORTA, MDR_PORTB, MDR_PORTC, MDR_PORTD, MDR_PORTE, MDR_PORTF};
static const uint32_t pclks[] = {RST_CLK_PCLK_PORTA, RST_CLK_PCLK_PORTB, RST_CLK_PCLK_PORTC,
                                 RST_CLK_PCLK_PORTD, RST_CLK_PCLK_PORTE, RST_CLK_PCLK_PORTF};

static inline bool get_gpio_port(uint8_t gpio, MDR_PORT_TypeDef **port, uint16_t *pin, uint32_t *pclk) {
    uint8_t idx = gpio / 16;
    if (idx >= sizeof(ports) / sizeof(ports[0]))
        return false;
    if (port)
        *port = ports[idx];
    if (pin)
        *pin = (1 << (gpio & 0x0F));
    if (pclk)
        *pclk = pclks[idx];
    return true;
}

void hal_gpio_init(uint8_t gpio) {
    uint32_t pclk;
    if (get_gpio_port(gpio, NULL, NULL, &pclk))
        RST_CLK_PCLKcmd(pclk, ENABLE);
}

static void configure_port(uint8_t gpio, uint8_t oe, uint8_t pull_up, uint8_t pull_down) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL))
        return;
    PORT_InitTypeDef init;
    PORT_StructInit(&init);
    init.PORT_Pin = pin;
    init.PORT_OE = oe;
    init.PORT_FUNC = PORT_FUNC_PORT;
    init.PORT_MODE = PORT_MODE_DIGITAL;
    init.PORT_SPEED = PORT_SPEED_SLOW;
    init.PORT_PULL_UP = pull_up;
    init.PORT_PULL_DOWN = pull_down;
    PORT_Init(port, &init);
}

void hal_gpio_set_dir(uint8_t gpio, bool is_output) {
    configure_port(gpio, is_output ? PORT_OE_OUT : PORT_OE_IN, PORT_PULL_UP_OFF, PORT_PULL_DOWN_OFF);
}



void hal_gpio_pull_down(uint8_t gpio) {
    configure_port(gpio, PORT_OE_IN, PORT_PULL_UP_OFF, PORT_PULL_DOWN_ON);
}

void hal_gpio_pull_up(uint8_t gpio) {
    configure_port(gpio, PORT_OE_IN, PORT_PULL_UP_ON, PORT_PULL_DOWN_OFF);
}

void hal_sleep_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 8; i++) {
        __NOP();
    }
}

void platform_init(void) {
    CLK_Init_80_mhz();
}

void platform_bootloader_jump(void) {
    RST_CLK_PCLKcmd(RST_CLK_PCLK_BKP, ENABLE);
    MDR_BKP->REG_00 = 0xDFDB007; // DFU bootloader request signature
    NVIC_SystemReset();
}

#endif

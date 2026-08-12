/**
 * *****************************************************************************
 *  @file       bmcu_cru.h
 *  @author     Baikal electronics SDK team
 *  @brief      Control Registers Unit (CRU) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_CRU_H
#define __BMCU_CRU_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CRU clocks frequency structure */
typedef struct
{
    uint32_t CCLK_Frequency;        /*!< CCLK clock frequency */
    uint32_t PCLK0_Frequency;       /*!< PCLK0 clock frequency */
    uint32_t PCLK1_Frequency;       /*!< PCLK2 clock frequency */
    uint32_t PCLK2_Frequency;       /*!< PCLK2 clock frequency */
    uint32_t HCLK_Frequency;        /*!< HCLK clock frequency */
    uint32_t TCLK_Frequency;        /*!< TCLK clock frequency */
    uint32_t CANx2CLK_Frequency;    /*!< CANx2 clock frequency */
} CRU_Clocks_TypeDef;

/** 
 * @brief Defines used to adapt values of different oscillators.
 * @note These values could be modified in the user environment according to hardware setup.
 */
#if !defined  (HSE_VALUE)
#define HSE_VALUE       25000000UL  /*!< Value of the CLKI oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
#define HSI_VALUE       20000000UL  /*!< Value of the RC oscillator in Hz */
#endif /* HSI_VALUE */

/** @brief GPIO ports */
#define CRU_PORT_A                      0U  /*!< Port A */
#define CRU_PORT_B                      1U  /*!< Port B */
#define CRU_PORT_C                      2U  /*!< Port C */

/** @brief GPIO pins */
#define CRU_PIN_0                       0x0001U     /*!< Pin 0  */
#define CRU_PIN_1                       0x0002U     /*!< Pin 1  */
#define CRU_PIN_2                       0x0004U     /*!< Pin 2  */
#define CRU_PIN_3                       0x0008U     /*!< Pin 3  */
#define CRU_PIN_4                       0x0010U     /*!< Pin 4  */
#define CRU_PIN_5                       0x0020U     /*!< Pin 5  */
#define CRU_PIN_6                       0x0040U     /*!< Pin 6  */
#define CRU_PIN_7                       0x0080U     /*!< Pin 7  */
#define CRU_PIN_8                       0x0100U     /*!< Pin 8  */
#define CRU_PIN_9                       0x0200U     /*!< Pin 9  */
#define CRU_PIN_10                      0x0400U     /*!< Pin 10 */
#define CRU_PIN_11                      0x0800U     /*!< Pin 11 */
#define CRU_PIN_12                      0x1000U     /*!< Pin 12 */
#define CRU_PIN_13                      0x2000U     /*!< Pin 13 */
#define CRU_PIN_14                      0x4000U     /*!< Pin 14 */
#define CRU_PIN_15                      0x8000U     /*!< Pin 15 */
#define CRU_PIN_ALL                     (CRU_PIN_0  | CRU_PIN_1  | CRU_PIN_2  | \
                                         CRU_PIN_3  | CRU_PIN_4  | CRU_PIN_5  | \
                                         CRU_PIN_6  | CRU_PIN_7  | CRU_PIN_8  | \
                                         CRU_PIN_9  | CRU_PIN_10 | CRU_PIN_11 | \
                                         CRU_PIN_12 | CRU_PIN_13 | CRU_PIN_14 | \
                                         CRU_PIN_15)   /*!< All pins */

/** @brief Pin pull state */
typedef enum {
    CRU_PIN_PULL_NO      = 0x0, /*!< No pull */
    CRU_PIN_PULL_UP      = 0x1, /*!< Pull up */
    CRU_PIN_PULL_DOWN    = 0x2, /*!< Pull down */
    CRU_PIN_PULL_UP_DOWN = 0x3  /*!< Both pull up and pull down */
} CRU_PinPull_TypeDef;

/** @brief Pin drive strength */
typedef enum {
    CRU_PIN_DRIVE_STRENGTH_0 = 0,   /*!< Pin drive strength 0 (minimal) */
    CRU_PIN_DRIVE_STRENGTH_1,       /*!< Pin drive strength 1 */
    CRU_PIN_DRIVE_STRENGTH_2,       /*!< Pin drive strength 2 */
    CRU_PIN_DRIVE_STRENGTH_3        /*!< Pin drive strength 3 (maximal) */
} CRU_PinDriveStrength_TypeDef;

/** @brief Pin alternate functions */
typedef enum {
    CRU_PIN_AF_0 = 0,   /*!< Alternate function 0 */
    CRU_PIN_AF_1,       /*!< Alternate function 1 */
    CRU_PIN_AF_2,       /*!< Alternate function 2 */
    CRU_PIN_AF_3,       /*!< Alternate function 3 */
    CRU_PIN_AF_4,       /*!< Alternate function 4 */
    CRU_PIN_AF_5        /*!< Alternate function 5 */
} CRU_PinAF_TypeDef;

/** @brief CRU Pin Init Structure definition */
typedef struct
{
    uint8_t Port;                                       /*!< CRU Port (CRU_PORT_x) */
    uint16_t Pin;                                       /*!< CRU Pin (CRU_PIN_x). Only one pin allowed. */
    CRU_PinPull_TypeDef Pull;                           /*!< Pin pull state */
    FunctionalState InputCtrl;                          /*!< Pin input control */
    CRU_PinDriveStrength_TypeDef DriveStrength;         /*!< Pin drive strength */
    CRU_PinAF_TypeDef Alternate;                        /*!< Pin alternate function */
} CRU_PIN_InitStruct_TypeDef;

/** @brief Boot clock source */
typedef enum {
    CRU_BOOT_CLK_STRAP = 0, /*!< Defined by STRAP inputs */
    CRU_BOOT_CLK_CLKI       /*!< CLKI */
} CRU_BootClkSrc_TypeDef;

/** @brief CLKI monitor threshold level */
typedef enum {
    CRU_CLKMON_ALRM_LVL_0  = CRU_SYSCR0_CLKI_ALARM_LVL_0,     /*!< CLKI monitor threshold level 0 */
    CRU_CLKMON_ALRM_LVL_1  = CRU_SYSCR0_CLKI_ALARM_LVL_1,     /*!< CLKI monitor threshold level 1 */
    CRU_CLKMON_ALRM_LVL_2  = CRU_SYSCR0_CLKI_ALARM_LVL_2,     /*!< CLKI monitor threshold level 2 */
    CRU_CLKMON_ALRM_LVL_3  = CRU_SYSCR0_CLKI_ALARM_LVL_3,     /*!< CLKI monitor threshold level 3 */
    CRU_CLKMON_ALRM_LVL_4  = CRU_SYSCR0_CLKI_ALARM_LVL_4,     /*!< CLKI monitor threshold level 4 */
    CRU_CLKMON_ALRM_LVL_5  = CRU_SYSCR0_CLKI_ALARM_LVL_5,     /*!< CLKI monitor threshold level 5 */
    CRU_CLKMON_ALRM_LVL_6  = CRU_SYSCR0_CLKI_ALARM_LVL_6,     /*!< CLKI monitor threshold level 6 */
    CRU_CLKMON_ALRM_LVL_7  = CRU_SYSCR0_CLKI_ALARM_LVL_7,     /*!< CLKI monitor threshold level 7 */
    CRU_CLKMON_ALRM_LVL_8  = CRU_SYSCR0_CLKI_ALARM_LVL_8,     /*!< CLKI monitor threshold level 8 */
    CRU_CLKMON_ALRM_LVL_9  = CRU_SYSCR0_CLKI_ALARM_LVL_9,     /*!< CLKI monitor threshold level 9 */
    CRU_CLKMON_ALRM_LVL_10 = CRU_SYSCR0_CLKI_ALARM_LVL_10,    /*!< CLKI monitor threshold level 10 */
    CRU_CLKMON_ALRM_LVL_11 = CRU_SYSCR0_CLKI_ALARM_LVL_11,    /*!< CLKI monitor threshold level 11 */
    CRU_CLKMON_ALRM_LVL_12 = CRU_SYSCR0_CLKI_ALARM_LVL_12,    /*!< CLKI monitor threshold level 12 */
    CRU_CLKMON_ALRM_LVL_13 = CRU_SYSCR0_CLKI_ALARM_LVL_13,    /*!< CLKI monitor threshold level 13 */
    CRU_CLKMON_ALRM_LVL_14 = CRU_SYSCR0_CLKI_ALARM_LVL_14,    /*!< CLKI monitor threshold level 14 */
    CRU_CLKMON_ALRM_LVL_15 = CRU_SYSCR0_CLKI_ALARM_LVL_15     /*!< CLKI monitor threshold level 15 */
} CRU_ClkMonAlarmLvl_TypeDef;

/** @brief MCU clock source */
typedef enum {
    CRU_MCU_CLK_CLKI = CRU_SYSSR0_STRAP_CLK_CLKI, /*!< CLKI */
    CRU_MCU_CLK_RC   = CRU_SYSSR0_STRAP_CLK_RC    /*!< RC generator */
} CRU_MCUClkSrc_TypeDef;

/** @brief PLL source */
typedef enum {
    CRU_PLL_SRC_C0 = CRU_CLKSEL_REFCLKSEL_C0,   /*!< C0 clock selected as PLL clock source */
    CRU_PLL_SRC_C1 = CRU_CLKSEL_REFCLKSEL_C1    /*!< C1 clock selected as PLL clock source */
} CRU_PLLSrc_TypeDef;

/** @brief Clock divider */
typedef enum {
    CRU_CLK_DIV_1 = 0,  /*!< Clock not divided */
    CRU_CLK_DIV_1_5,    /*!< Clock divided by 1.5 */
    CRU_CLK_DIV_2,      /*!< Clock divided by 2 */
    CRU_CLK_DIV_2_5,    /*!< Clock divided by 2.5 */
    CRU_CLK_DIV_3,      /*!< Clock divided by 3 */
    CRU_CLK_DIV_3_5,    /*!< Clock divided by 3.5 */
    CRU_CLK_DIV_4,      /*!< Clock divided by 4 */
    CRU_CLK_DIV_4_5,    /*!< Clock divided by 4.5 */
    CRU_CLK_DIV_5,      /*!< Clock divided by 5 */
    CRU_CLK_DIV_5_5,    /*!< Clock divided by 5.5 */
    CRU_CLK_DIV_6,      /*!< Clock divided by 6 */
    CRU_CLK_DIV_6_5,    /*!< Clock divided by 6.5 */
    CRU_CLK_DIV_7,      /*!< Clock divided by 7 */
    CRU_CLK_DIV_7_5,    /*!< Clock divided by 7.5 */
    CRU_CLK_DIV_8,      /*!< Clock divided by 8 */
    CRU_CLK_DIV_8_5,    /*!< Clock divided by 8.5 */
    CRU_CLK_DIV_9,      /*!< Clock divided by 9 */
    CRU_CLK_DIV_9_5,    /*!< Clock divided by 9.5 */
    CRU_CLK_DIV_10,     /*!< Clock divided by 10 */
    CRU_CLK_DIV_10_5,   /*!< Clock divided by 10.5 */
    CRU_CLK_DIV_11,     /*!< Clock divided by 11 */
    CRU_CLK_DIV_11_5,   /*!< Clock divided by 11.5 */
    CRU_CLK_DIV_12,     /*!< Clock divided by 12 */
    CRU_CLK_DIV_12_5,   /*!< Clock divided by 12.5 */
    CRU_CLK_DIV_13,     /*!< Clock divided by 13 */
    CRU_CLK_DIV_13_5,   /*!< Clock divided by 13.5 */
    CRU_CLK_DIV_14,     /*!< Clock divided by 14 */
    CRU_CLK_DIV_14_5,   /*!< Clock divided by 14.5 */
    CRU_CLK_DIV_15,     /*!< Clock divided by 15 */
    CRU_CLK_DIV_15_5,   /*!< Clock divided by 15.5 */
    CRU_CLK_DIV_16      /*!< Clock divided by 16 */
} CRU_ClkDiv_TypeDef;

/** @brief Clock source */
typedef enum {
    CRU_CLK_SRC_C0  = 0,    /*!< C0 clock selected as clock source */
    CRU_CLK_SRC_PLL,        /*!< PLL output selected as clock source */
    CRU_CLK_SRC_C1          /*!< C1 clock selected as clock source */
} CRU_ClkSrc_TypeDef;

/** @brief APB0 peripherals */
typedef enum {
    CRU_APB0_PERIPH_QSPI0 = 0,  /*!< QSPI0 */
    CRU_APB0_PERIPH_SPI0,       /*!< SPI0 */
    CRU_APB0_PERIPH_SPI1,       /*!< SPI1 */
    CRU_APB0_PERIPH_UART0,      /*!< UART0 */
    CRU_APB0_PERIPH_UART1,      /*!< UART1 */
    CRU_APB0_PERIPH_UART2,      /*!< UART2 */
    CRU_APB0_PERIPH_I2C0,       /*!< I2C0 */
    CRU_APB0_PERIPH_I2C1,       /*!< I2C1 */
    CRU_APB0_PERIPH_I2S0,       /*!< I2S0 */
    CRU_APB0_PERIPH_TIM0,       /*!< TIM0 */
    CRU_APB0_PERIPH_WDT0,       /*!< WDT0 */
    CRU_APB0_PERIPH_GPIO0,      /*!< GPIO0 */
    CRU_APB0_PERIPH_PWMG0,      /*!< PWMG0 */
    CRU_APB0_PERIPH_CANFD0      /*!< CANFD0 */
} CRU_APB0Periph_TypeDef;

/** @brief APB1 peripherals */
typedef enum {
    CRU_APB1_PERIPH_QSPI1 = 0,  /*!< QSPI1 */
    CRU_APB1_PERIPH_SPI2,       /*!< SPI2 */
    CRU_APB1_PERIPH_SPI3,       /*!< SPI3 */
    CRU_APB1_PERIPH_UART3,      /*!< UART3 */
    CRU_APB1_PERIPH_UART4,      /*!< UART4 */
    CRU_APB1_PERIPH_UART5,      /*!< UART5 */
    CRU_APB1_PERIPH_I2C2,       /*!< I2C2 */
    CRU_APB1_PERIPH_I2C3,       /*!< I2C3 */
    CRU_APB1_PERIPH_I2S1,       /*!< I2S1 */
    CRU_APB1_PERIPH_TIM1,       /*!< TIM1 */
    CRU_APB1_PERIPH_WDT1,       /*!< WDT1 */
    CRU_APB1_PERIPH_GPIO1,      /*!< GPIO1 */
    CRU_APB1_PERIPH_PWMG1,      /*!< PWMG1 */
    CRU_APB1_PERIPH_CANFD1      /*!< CANFD1 */
} CRU_APB1Periph_TypeDef;

/** @brief APB2 peripherals */
typedef enum {
    CRU_APB2_PERIPH_ADC0 = 0,   /*!< ADC0 */
    CRU_APB2_PERIPH_ADC1,       /*!< ADC1 */
    CRU_APB2_PERIPH_ADC2,       /*!< ADC2 */
    CRU_APB2_PERIPH_PWMA0,      /*!< PWMA0 */
    CRU_APB2_PERIPH_PWMA1,      /*!< PWMA1 */
    CRU_APB2_PERIPH_PWMA2,      /*!< PWMA2 */
    CRU_APB2_PERIPH_PWMA3,      /*!< PWMA3 */
    CRU_APB2_PERIPH_GPIO2,      /*!< GPIO2 */
    CRU_APB2_PERIPH_UART6,      /*!< UART6 */
    CRU_APB2_PERIPH_UART7       /*!< UART7 */
} CRU_APB2Periph_TypeDef;

/** @brief AXI masters */
typedef enum {
    CRU_AXI_MASTER_CORE0MP  = CRU_PRIOR0_CORE0MP_PL,    /*!< MP Core0 AXI Interconnect */
    CRU_AXI_MASTER_CORE0DIO = CRU_PRIOR0_CORE0DIO_PL,   /*!< DIO Core0 AXI Interconnect */
    CRU_AXI_MASTER_CORE1MP  = CRU_PRIOR0_CORE1MP_PL,    /*!< MP Core1 AXI Interconnect */
    CRU_AXI_MASTER_CORE1DIO = CRU_PRIOR0_CORE1DIO_PL,   /*!< DIO Core1 AXI Interconnect */
    CRU_AXI_MASTER_DMA0     = CRU_PRIOR0_DMA0_PL,       /*!< DMA0 AXI Interconnect */
    CRU_AXI_MASTER_DMA1     = CRU_PRIOR0_DMA1_PL,       /*!< DMA1 AXI Interconnect */
#if defined (BMCU_U)
    CRU_AXI_MASTER_DMA2     = CRU_PRIOR0_DMA2_PL,       /*!< DMA2 AXI Interconnect */
#endif
    CRU_AXI_MASTER_DMAUSB   = CRU_PRIOR0_DMAUSB_PL      /*!< DMA USB AXI Interconnect */
} CRU_AXIMaster_TypeDef;

/** @brief AXI slaves */
typedef enum {
    CRU_AXI_SLAVE_CORE0FP = CRU_PRIOR1_CORE0FP_PL,      /*!< FP Core0 AXI Interconnect */
#if defined (BMCU_U)
    CRU_AXI_SLAVE_CORE1FP = CRU_PRIOR1_CORE1FP_PL,      /*!< FP Core1 AXI Interconnect */
#endif
    CRU_AXI_SLAVE_SRAM    = CRU_PRIOR1_SRAM_PL,         /*!< SRAM AXI Interconnect */
    CRU_AXI_SLAVE_EFLASH  = CRU_PRIOR1_EFLASH_PL,       /*!< EFLASH AXI Interconnect */
    CRU_AXI_SLAVE_APB0    = CRU_PRIOR1_APB0_PL,         /*!< APB0 AXI Interconnect */
    CRU_AXI_SLAVE_APB1    = CRU_PRIOR1_APB1_PL,         /*!< APB1 AXI Interconnect */
    CRU_AXI_SLAVE_APB2    = CRU_PRIOR1_APB2_PL,         /*!< APB2 AXI Interconnect */
    CRU_AXI_SLAVE_AHB     = CRU_PRIOR1_AHB_PL           /*!< AHB AXI Interconnect */
} CRU_AXISlave_TypeDef;

/** @brief AXI priority */
typedef enum {
    CRU_AXI_PRIO_0 = 0, /*!< Priority 0 (Lowest) */
    CRU_AXI_PRIO_1,     /*!< Priority 1 */
    CRU_AXI_PRIO_2,     /*!< Priority 2 */
    CRU_AXI_PRIO_3,     /*!< Priority 3 */
    CRU_AXI_PRIO_4,     /*!< Priority 4 */
    CRU_AXI_PRIO_5,     /*!< Priority 5 */
    CRU_AXI_PRIO_6,     /*!< Priority 6 */
    CRU_AXI_PRIO_7,     /*!< Priority 7 */
#if defined (BMCU_U)
    CRU_AXI_PRIO_8,     /*!< Priority 8 */
    CRU_AXI_PRIO_9,     /*!< Priority 9 */
    CRU_AXI_PRIO_10,    /*!< Priority 10 */
    CRU_AXI_PRIO_11,    /*!< Priority 11 */
    CRU_AXI_PRIO_12,    /*!< Priority 12 */
    CRU_AXI_PRIO_13,    /*!< Priority 13 */
    CRU_AXI_PRIO_14,    /*!< Priority 14 */
    CRU_AXI_PRIO_15     /*!< Priority 15 (Highest) */
#endif
} CRU_AXIPriority_TypeDef;

/** @brief LDO CORE output voltage */
typedef enum {
    CRU_LDOC_ADJ_LEV0 = CRU_LDOCR_LDOC_ADJ_LEV0,  /*!< LDO CORE output voltage 1.10V */
    CRU_LDOC_ADJ_LEV1 = CRU_LDOCR_LDOC_ADJ_LEV1,  /*!< LDO CORE output voltage 1.15V */
    CRU_LDOC_ADJ_LEV2 = CRU_LDOCR_LDOC_ADJ_LEV2,  /*!< LDO CORE output voltage 1.00V */
    CRU_LDOC_ADJ_LEV3 = CRU_LDOCR_LDOC_ADJ_LEV3,  /*!< LDO CORE output voltage 1.05V */
    CRU_LDOC_ADJ_LEV4 = CRU_LDOCR_LDOC_ADJ_LEV4,  /*!< LDO CORE output voltage 1.30V */
    CRU_LDOC_ADJ_LEV5 = CRU_LDOCR_LDOC_ADJ_LEV5,  /*!< LDO CORE output voltage 1.35V */
    CRU_LDOC_ADJ_LEV6 = CRU_LDOCR_LDOC_ADJ_LEV6,  /*!< LDO CORE output voltage 1.20V */
    CRU_LDOC_ADJ_LEV7 = CRU_LDOCR_LDOC_ADJ_LEV7   /*!< LDO CORE output voltage 1.25V */
} CRU_LDOC_Adj_TypeDef;

/** @brief LDO CORE protection current */
typedef enum {
    CRU_LDOC_PI_LEV0 = CRU_LDOCR_LDOC_LDO_PI_LEV0,    /*!< LDO CORE protection current 500mA */
    CRU_LDOC_PI_LEV1 = CRU_LDOCR_LDOC_LDO_PI_LEV1,    /*!< LDO CORE protection current 460mA */
    CRU_LDOC_PI_LEV2 = CRU_LDOCR_LDOC_LDO_PI_LEV2,    /*!< LDO CORE protection current 420mA */
    CRU_LDOC_PI_LEV3 = CRU_LDOCR_LDOC_LDO_PI_LEV3,    /*!< LDO CORE protection current 380mA */
    CRU_LDOC_PI_LEV4 = CRU_LDOCR_LDOC_LDO_PI_LEV4,    /*!< LDO CORE protection current 660mA */
    CRU_LDOC_PI_LEV5 = CRU_LDOCR_LDOC_LDO_PI_LEV5,    /*!< LDO CORE protection current 620mA */
    CRU_LDOC_PI_LEV6 = CRU_LDOCR_LDOC_LDO_PI_LEV6     /*!< LDO CORE protection current 580mA */
} CRU_LDOC_PI_TypeDef;

/** @brief LDO EFLASH output voltage */
typedef enum {
    CRU_LDOF_ADJ_LEV0 = CRU_LDOCR_LDOF_ADJ_LEV0,  /*!< LDO EFLASH output voltage 1.70V */
    CRU_LDOF_ADJ_LEV1 = CRU_LDOCR_LDOF_ADJ_LEV1,  /*!< LDO EFLASH output voltage 1.75V */
    CRU_LDOF_ADJ_LEV2 = CRU_LDOCR_LDOF_ADJ_LEV2,  /*!< LDO EFLASH output voltage 1.60V */
    CRU_LDOF_ADJ_LEV3 = CRU_LDOCR_LDOF_ADJ_LEV3,  /*!< LDO EFLASH output voltage 1.65V */
    CRU_LDOF_ADJ_LEV4 = CRU_LDOCR_LDOF_ADJ_LEV4,  /*!< LDO EFLASH output voltage 1.90V */
    CRU_LDOF_ADJ_LEV5 = CRU_LDOCR_LDOF_ADJ_LEV5,  /*!< LDO EFLASH output voltage 1.95V */
    CRU_LDOF_ADJ_LEV6 = CRU_LDOCR_LDOF_ADJ_LEV6,  /*!< LDO EFLASH output voltage 0.80V */
    CRU_LDOF_ADJ_LEV7 = CRU_LDOCR_LDOF_ADJ_LEV7   /*!< LDO EFLASH output voltage 0.85V */
} CRU_LDOF_Adj_TypeDef;

/** @brief LDO EFLASH protection current */
typedef enum {
    CRU_LDOF_PI_LEV0 = CRU_LDOCR_LDOF_LDO_PI_LEV0,    /*!< LDO EFLASH protection current 290mA */
    CRU_LDOF_PI_LEV1 = CRU_LDOCR_LDOF_LDO_PI_LEV1,    /*!< LDO EFLASH protection current 260mA */
    CRU_LDOF_PI_LEV2 = CRU_LDOCR_LDOF_LDO_PI_LEV2,    /*!< LDO EFLASH protection current 225mA */
    CRU_LDOF_PI_LEV3 = CRU_LDOCR_LDOF_LDO_PI_LEV3,    /*!< LDO EFLASH protection current 195mA */
    CRU_LDOF_PI_LEV4 = CRU_LDOCR_LDOF_LDO_PI_LEV4,    /*!< LDO EFLASH protection current 410mA */
    CRU_LDOF_PI_LEV5 = CRU_LDOCR_LDOF_LDO_PI_LEV5,    /*!< LDO EFLASH protection current 380mA */
    CRU_LDOF_PI_LEV6 = CRU_LDOCR_LDOF_LDO_PI_LEV6,    /*!< LDO EFLASH protection current 350mA */
    CRU_LDOF_PI_LEV7 = CRU_LDOCR_LDOF_LDO_PI_LEV7     /*!< LDO EFLASH protection current 320mA */
} CRU_LDOF_PI_TypeDef;

/** @brief Flags */
typedef enum {

#if defined (BMCU_U)
    CRU_FLAG_SECUREDIS = 4,     /*!< Secure boot disable flag */
    CRU_FLAG_EFLASH_OK  = 5,    /*!< EFLASH good flag */
    CRU_FLAG_PACK  = 6,         /*!< MCU pin count flag */
    CRU_FLAG_WDTHIT = 7         /*!< Watchdog reset flag */
#endif

#if defined (BE_U1000)
    CRU_FLAG_NOSECURE = 5,      /*!< Secure boot disable flag */
    CRU_FLAG_EFLASH_OK = 6,     /*!< EFLASH good flag */
    CRU_FLAG_POR_OK = 7,        /*!< PoR circuit good flag */
    CRU_FLAG_WDTHIT = 8,        /*!< Watchdog reset flag */
    CRU_FLAG_PLL_RFSLIP = 9,    /*!< PLL RFSLIP output state */
    CRU_FLAG_PLL_FBSLIP = 10,   /*!< PLL FBSLIP output state */
    CRU_FLAG_PLL_NLOCK = 11,    /*!< PLL NLOCK flag */
    CRU_FLAG_NPOR_LDOF = 12     /*!< EFLASH LDO NPOR output state */
#endif

} CRU_Flag_TypeDef;

/**
 * @brief Writes a value in CRU register.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define CRU_WriteReg(REG, VALUE)        WRITE_REG(CRU->REG, (VALUE))

/**
 * @brief Reads a value in CRU register.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define CRU_ReadReg(REG)                READ_REG(CRU->REG)

/******************************************************************************/
/*                               Clock control                                */
/******************************************************************************/

/**
 * @brief Configures boot clock source.
 * @param Source The boot clock source. Can be one of CRU_BootClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetBootClkSource(CRU_BootClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_BOOTCLKSEL, ((uint32_t)Source) << CRU_CLKCR0_BOOTCLKSEL_Pos);
}

/**
 * @brief Returns boot clock source.
 * @returns The boot clock source. Can be one of CRU_BootClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_BootClkSrc_TypeDef CRU_GetBootClkSource(void)
{
    return (CRU_BootClkSrc_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_BOOTCLKSEL) >> CRU_CLKCR0_BOOTCLKSEL_Pos);
}

/**
 * @brief Enables RC generator.
 */
__STATIC_INLINE void CRU_RCGen_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_RCGENEN);
}

/**
 * @brief Disables RC generator.
 */
__STATIC_INLINE void CRU_RCGen_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_RCGENEN);
}

/**
 * @brief Checks if RC generator is enabled.
 * @returns 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_RCGen_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_RCGENEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables CLKI clock monitoring system.
 */
__STATIC_INLINE void CRU_ClkMon_Enable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CLKI_ALARM_EN);
}

/**
 * @brief Disables CLKI clock monitoring system.
 */
__STATIC_INLINE void CRU_ClkMon_Disable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CLKI_ALARM_EN);
}

/**
 * @brief Checks if CLKI clock monitoring system is enabled.
 * @returns 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_ClkMon_IsEnabled(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CLKI_ALARM_EN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets CLKI clock monitoring system alarm level.
 * @param Level The alarm level. Can be one of CRU_ClkMonAlarmLvl_TypeDef values.
 */
__STATIC_INLINE void CRU_ClkMon_SetAlarmLvl(CRU_ClkMonAlarmLvl_TypeDef Level)
{
    MODIFY_REG(CRU->SYSCR0, CRU_SYSCR0_CLKI_ALARM_LVL, (uint32_t)Level);
}

/**
 * @brief Returns CLKI clock monitoring system alarm level.
 * @returns The alarm level. Can be one of CRU_ClkMonAlarmLvl_TypeDef values.
 */
__STATIC_INLINE CRU_ClkMonAlarmLvl_TypeDef CRU_ClkMon_GetAlarmLvl(void)
{
    return (CRU_ClkMonAlarmLvl_TypeDef)READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CLKI_ALARM_LVL);
}

/**
 * @brief Returns CLKI clock monitoring system ALARM state.
 * @returns 1 if ALARM is set, otherwise 0.
 */
__STATIC_INLINE uint8_t CRU_ClkMon_GetAlarm(void)
{
#if (__RV_CLIC_EN != 0U)
    return CLIC_GetPendingIRQ(CLIC_CLKI_Mon_IRQn);
#else
    return 0U;
#endif
}

/**
 * @brief Returns C0 clock source.
 *        The clock source is selected by BOOTCLKSEL, CLK_STRAP and 
 *        NOSECURE inputs.
 * @returns The C0 clock source. Can be one of CRU_MCUClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_MCUClkSrc_TypeDef CRU_GetC0Source(void)
{
    CRU_MCUClkSrc_TypeDef src;

    if ((CRU_ClkMon_IsEnabled() != 0UL) && (CRU_ClkMon_GetAlarm() != 0U))
    {
        /* ALARM is set */
        src = CRU_MCU_CLK_RC;
    }
    else
    {
        /* No ALARM set */
        if (CRU_GetBootClkSource() == CRU_BOOT_CLK_CLKI)
        {
            /* MCU clock source is set to CLKI */
            src = CRU_MCU_CLK_CLKI;
        }
        else
        {
            /* MCU clock source is defined by CLK_STRAP and NOSECURE inputs */
#if defined (BMCU_U)
            if (READ_BIT(CRU->SYSSR0, CRU_SYSSR0_SECUREDIS) != 0UL)
#endif

#if defined (BE_U1000)
            if (READ_BIT(CRU->SYSSR0, CRU_SYSSR0_NOSECURE) != 0UL)
#endif
            {
                /* MCU clock source is selected by the CLK_STRAP input */
                src = (CRU_MCUClkSrc_TypeDef)READ_BIT(CRU->SYSSR0, CRU_SYSSR0_STRAP_CLK);
            }
            else
            {
                /* MCU clock source is forced to the internal RC generator */
                src = CRU_MCU_CLK_RC;
            }
        }
    }

    return src;
}

/**
 * @brief Returns C1 clock source.
 *        The clock source is selected by BOOTCLKSEL, CLK_STRAP and 
 *        NOSECURE inputs.
 * @note C1 clock source is the complement of C0 clock source.
 * @returns The C1 clock source. Can be one of CRU_MCUClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_MCUClkSrc_TypeDef CRU_GetC1Source(void)
{
    CRU_MCUClkSrc_TypeDef src;

    if (CRU_GetC0Source() == CRU_MCU_CLK_CLKI)
    {
        src = CRU_MCU_CLK_RC;
    }
    else
    {
        src = CRU_MCU_CLK_CLKI;
    }

    return src;
}

/**
 * @brief Configures PLL source oscillator.
 * @param PLLSource The PLL source oscillator. Can be one of CRU_PLLSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_PLL_SetSource(CRU_PLLSrc_TypeDef PLLSource)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_REFCLKSEL, (uint32_t)PLLSource);
}

/**
 * @brief Returns the oscillator used as PLL source.
 * @returns The PLL source oscillator. Can be one of CRU_PLLSrc_TypeDef values.
 */
__STATIC_INLINE CRU_PLLSrc_TypeDef CRU_PLL_GetSource(void)
{
    return (CRU_PLLSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_REFCLKSEL));
}

/**
 * @brief Enables PLL.
 * @note In power-down mode all analog circuitry in the PLL is turned off
 *       so as to only dissipate leakage current.
 */
__STATIC_INLINE void CRU_PLL_Enable(void)
{
    SET_BIT(CRU->PLLSET, CRU_PLLSET_PWRDN);
}

/**
 * @brief Disables PLL.
 * @note In power-down mode all analog circuitry in the PLL is turned off
 *       so as to only dissipate leakage current.
 */
__STATIC_INLINE void CRU_PLL_Disable(void)
{
    CLEAR_BIT(CRU->PLLSET, CRU_PLLSET_PWRDN);
}

/**
 * @brief Checks if PLL is enabled.
 * @note In power-down mode all analog circuitry in the PLL is turned off
 *       so as to only dissipate leakage current.
 * @returns 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_IsEnabled(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_PWRDN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces PLL reset.
 * @note In reset mode the PLL outputs a fixed free-running frequency
 *       in the range of 10MHz to 100MHz.
 */
__STATIC_INLINE void CRU_PLL_ForceReset(void)
{
    SET_BIT(CRU->PLLSET, CRU_PLLSET_RESET);
}

/**
 * @brief Releases PLL reset.
 * @note In reset mode the PLL outputs a fixed free-running frequency
 *       in the range of 10MHz to 100MHz.
 */
__STATIC_INLINE void CRU_PLL_ReleaseReset(void)
{
    CLEAR_BIT(CRU->PLLSET, CRU_PLLSET_RESET);
}

/**
 * @brief Checks if PLL is in reset state.
 * @note In reset mode the PLL outputs a fixed free-running frequency
 *       in the range of 10MHz to 100MHz.
 * @returns 1 if in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_IsReset(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_RESET) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables PLL bypass mode.
 * @note In bypass mode the reference input is bypassed directly to the outputs.
 */
__STATIC_INLINE void CRU_PLL_BypassMode_Enable(void)
{
    SET_BIT(CRU->PLLSET, CRU_PLLSET_BYPASS);
}

/**
 * @brief Disables PLL bypass mode.
 * @note In bypass mode the reference input is bypassed directly to the outputs.
 */
__STATIC_INLINE void CRU_PLL_BypassMode_Disable(void)
{
    CLEAR_BIT(CRU->PLLSET, CRU_PLLSET_BYPASS);
}

/**
 * @brief Checks if PLL bypass mode is enabled.
 * @note In bypass mode the reference input is bypassed directly to the outputs.
 * @returns 1 if bypass mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_BypassMode_IsEnabled(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_BYPASS) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures PLL.
 *        The configuration uses internal feedback. The output frequency
 *        is related to the reference frequency as Fout = Fref*NF/NR/OD.
 *        The recommended setting for NB is NF, which will yield the nominal
 *        bandwidth.
 * @param NR The PLL reference divider value. Must be in the range of 1 to 16.
 * @param NF The PLL multiplication factor value. Must be in the range of 1 to 64.
 * @param OD The PLL post VCO divider value. Must be in the range of 1 to 16.
 * @param NB The PLL bandwidth divider value. Must be in the range of 1 to 64.
 */
__STATIC_INLINE void CRU_PLL_Config(uint32_t NR, uint32_t NF, uint32_t OD, uint32_t NB)
{
    MODIFY_REG(CRU->PLLSET,
               (CRU_PLLSET_BWADJ | CRU_PLLSET_CLKOD | CRU_PLLSET_CLKF | CRU_PLLSET_CLKR),
               (CRU_PLLSET_INTFB | ((NB-1) << CRU_PLLSET_BWADJ_Pos) | 
               ((OD-1) << CRU_PLLSET_CLKOD_Pos) | ((NF-1) << CRU_PLLSET_CLKF_Pos) |
               ((NR-1) << CRU_PLLSET_CLKR_Pos)));
}

/**
 * @brief Returns the PLL reference divider value.
 * @returns The PLL reference divider value in the range of 1 to 16.
 */
__STATIC_INLINE uint32_t CRU_PLL_GetNR(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_CLKR_Msk) >> CRU_PLLSET_CLKR_Pos) + 1U);
}

/**
 * @brief Returns the PLL multiplication factor value.
 * @returns The PLL multiplication factor value in the range of 1 to 64.
 */
__STATIC_INLINE uint32_t CRU_PLL_GetNF(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_CLKF_Msk) >> CRU_PLLSET_CLKF_Pos) + 1U);
}

/**
 * @brief Returns the PLL post VCO divider value.
 * @returns The PLL post VCO divider value in the range of 1 to 16.
 */
__STATIC_INLINE uint32_t CRU_PLL_GetOD(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_CLKOD_Msk) >> CRU_PLLSET_CLKOD_Pos) + 1U);
}

/**
 * @brief Returns the PLL bandwidth divider value.
 * @returns The PLL bandwidth divider value in the range of 1 to 64.
 */
__STATIC_INLINE uint32_t CRU_PLL_GetNB(void)
{
    return ((READ_BIT(CRU->PLLSET, CRU_PLLSET_BWADJ_Msk) >> CRU_PLLSET_BWADJ_Pos) + 1U);
}

#if defined (BE_U1000)

/**
 * @brief Checks if PLL RFSLIP flag is in active state.
 * @note RFSLIP goes active for one or more divided feedback VCO cycles
 *       when the phase detector misses a divided reference cycle,
 *       i.e. when the VCO is running too fast.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_IsActiveFlag_RFSLIP(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_PLL_RFSLIP) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if PLL FBSLIP flag is in active state.
 * @note FBSLIP goes active for one or more divided reference cycles
 *       when the phase detector misses a divided feedback VCO cycle,
 *       i.e. when the VCO is running too slow.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_IsActiveFlag_FBSLIP(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_PLL_FBSLIP) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if PLL NLOCK flag is in active state.
 * @note NLOCK goes active when PLL out-of-lock condition have been detected.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PLL_IsActiveFlag_NLOCK(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_PLL_NLOCK) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Resets PLL NLOCK flag to inactive state.
 * @note NLOCK goes active when PLL out-of-lock condition have been detected.
 */
__STATIC_INLINE void CRU_PLL_ResetFlag_NLOCK(void)
{
    CLEAR_BIT(CRU->PLLSET, CRU_PLLSET_PLL_NLOCK_RSTN);
    __ASM volatile("nop");
    SET_BIT(CRU->PLLSET, CRU_PLLSET_PLL_NLOCK_RSTN);
}

#endif

/**
 * @brief Sets CCLK prescaler.
 * @param Prescaler The CCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetCCLKPrescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_CCLKDIV, ((uint32_t)Prescaler) << CRU_CLKCR0_CCLKDIV_Pos);
}

/**
 * @brief Returns CCLK prescaler.
 * @returns The CCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetCCLKPrescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_CCLKDIV) >> CRU_CLKCR0_CCLKDIV_Pos);
}

/**
 * @brief Enables CCLK prescaler.
 */
__STATIC_INLINE void CRU_CCLKPrescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_CCLKDIVEN);
}

/**
 * @brief Disables CCLK prescaler.
 */
__STATIC_INLINE void CRU_CCLKPrescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_CCLKDIVEN);
}

/**
 * @brief Checks if CCLK prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_CCLKPrescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_CCLKDIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures CCLK source.
 * @param Source The CCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetCCLKSource(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_CCLKSEL, ((uint32_t)Source) << CRU_CLKSEL_CCLKSEL_Pos);
}

/**
 * @brief Returns CCLK source.
 * @returns The CCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetCCLKSource(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_CCLKSEL) >> CRU_CLKSEL_CCLKSEL_Pos);
}

/**
 * @brief Sets PCLK0 prescaler.
 * @param Prescaler The PCLK0 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK0Prescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_PCLK0DIV, ((uint32_t)Prescaler) << CRU_CLKCR0_PCLK0DIV_Pos);
}

/**
 * @brief Returns PCLK0 prescaler.
 * @returns The PCLK0 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetPCLK0Prescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK0DIV) >> CRU_CLKCR0_PCLK0DIV_Pos);
}

/**
 * @brief Enables PCLK0 prescaler.
 */
__STATIC_INLINE void CRU_PCLK0Prescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK0DIVEN);
}

/**
 * @brief Disables PCLK0 prescaler.
 */
__STATIC_INLINE void CRU_PCLK0Prescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK0DIVEN);
}

/**
 * @brief Checks if PCLK0 prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PCLK0Prescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK0DIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures PCLK0 source.
 * @param Source The PCLK0 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK0Source(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_PCLK0SEL, ((uint32_t)Source) << CRU_CLKSEL_PCLK0SEL_Pos);
}

/**
 * @brief Returns PCLK0 source.
 * @returns The PCLK0 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetPCLK0Source(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_PCLK0SEL) >> CRU_CLKSEL_PCLK0SEL_Pos);
}

/**
 * @brief Sets PCLK1 prescaler.
 * @param Prescaler The PCLK1 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK1Prescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_PCLK1DIV, ((uint32_t)Prescaler) << CRU_CLKCR0_PCLK1DIV_Pos);
}

/**
 * @brief Returns PCLK1 prescaler.
 * @returns The PCLK1 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetPCLK1Prescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK1DIV) >> CRU_CLKCR0_PCLK1DIV_Pos);
}

/**
 * @brief Enables PCLK1 prescaler.
 */
__STATIC_INLINE void CRU_PCLK1Prescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK1DIVEN);
}

/**
 * @brief Disables PCLK1 prescaler.
 */
__STATIC_INLINE void CRU_PCLK1Prescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK1DIVEN);
}

/**
 * @brief Checks if PCLK1 prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PCLK1Prescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK1DIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures PCLK1 source.
 * @param Source The PCLK1 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK1Source(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_PCLK1SEL, ((uint32_t)Source) << CRU_CLKSEL_PCLK1SEL_Pos);
}

/**
 * @brief Returns PCLK1 source.
 * @returns The PCLK1 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetPCLK1Source(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_PCLK1SEL) >> CRU_CLKSEL_PCLK1SEL_Pos);
}

/**
 * @brief Sets PCLK2 prescaler.
 * @param Prescaler The PCLK2 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK2Prescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_PCLK2DIV, ((uint32_t)Prescaler) << CRU_CLKCR0_PCLK2DIV_Pos);
}

/**
 * @brief Returns PCLK2 prescaler.
 * @returns The PCLK2 prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetPCLK2Prescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK2DIV) >> CRU_CLKCR0_PCLK2DIV_Pos);
}

/**
 * @brief Enables PCLK2 prescaler.
 */
__STATIC_INLINE void CRU_PCLK2Prescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK2DIVEN);
}

/**
 * @brief Disables PCLK2 prescaler.
 */
__STATIC_INLINE void CRU_PCLK2Prescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK2DIVEN);
}

/**
 * @brief Checks if PCLK2 prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_PCLK2Prescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_PCLK2DIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures PCLK2 source.
 * @param Source The PCLK2 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPCLK2Source(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_PCLK2SEL, ((uint32_t)Source) << CRU_CLKSEL_PCLK2SEL_Pos);
}

/**
 * @brief Returns PCLK2 source.
 * @returns The PCLK2 source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetPCLK2Source(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_PCLK2SEL) >> CRU_CLKSEL_PCLK2SEL_Pos);
}

/**
 * @brief Sets HCLK prescaler.
 * @param Prescaler The HCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetHCLKPrescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR0, CRU_CLKCR0_HCLKDIV, ((uint32_t)Prescaler) << CRU_CLKCR0_HCLKDIV_Pos);
}

/**
 * @brief Returns HCLK prescaler.
 * @returns The HCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetHCLKPrescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR0, CRU_CLKCR0_HCLKDIV) >> CRU_CLKCR0_HCLKDIV_Pos);
}

/**
 * @brief Enables HCLK prescaler.
 */
__STATIC_INLINE void CRU_HCLKPrescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR0, CRU_CLKCR0_HCLKDIVEN);
}

/**
 * @brief Disables HCLK prescaler.
 */
__STATIC_INLINE void CRU_HCLKPrescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR0, CRU_CLKCR0_HCLKDIVEN);
}

/**
 * @brief Checks if HCLK prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_HCLKPrescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR0, CRU_CLKCR0_HCLKDIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures HCLK source.
 * @param Source The HCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetHCLKSource(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_HCLKSEL, ((uint32_t)Source) << CRU_CLKSEL_HCLKSEL_Pos);
}

/**
 * @brief Returns HCLK source.
 * @returns The HCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetHCLKSource(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_HCLKSEL) >> CRU_CLKSEL_HCLKSEL_Pos);
}

/**
 * @brief Sets TCLK prescaler.
 * @param Prescaler The TCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetTCLKPrescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR1, CRU_CLKCR1_TCLKDIV, ((uint32_t)Prescaler) << CRU_CLKCR1_TCLKDIV_Pos);
}

/**
 * @brief Returns TCLK prescaler.
 * @returns The TCLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetTCLKPrescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR1, CRU_CLKCR1_TCLKDIV) >> CRU_CLKCR1_TCLKDIV_Pos);
}

/**
 * @brief Enables TCLK prescaler.
 */
__STATIC_INLINE void CRU_TCLKPrescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR1, CRU_CLKCR1_TCLKDIVEN);
}

/**
 * @brief Disables TCLK prescaler.
 */
__STATIC_INLINE void CRU_TCLKPrescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR1, CRU_CLKCR1_TCLKDIVEN);
}

/**
 * @brief Checks if TCLK prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_TCLKPrescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR1, CRU_CLKCR1_TCLKDIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures TCLK source.
 * @param Source The TCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetTCLKSource(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_TCLKSEL, ((uint32_t)Source) << CRU_CLKSEL_TCLKSEL_Pos);
}

/**
 * @brief Returns TCLK source.
 * @returns The TCLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetTCLKSource(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_TCLKSEL) >> CRU_CLKSEL_TCLKSEL_Pos);
}

/**
 * @brief Sets CANx2CLK prescaler.
 * @param Prescaler The CANx2CLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE void CRU_SetCANx2CLKPrescaler(CRU_ClkDiv_TypeDef Prescaler)
{
    MODIFY_REG(CRU->CLKCR1, CRU_CLKCR1_CANCLKDIV, ((uint32_t)Prescaler) << CRU_CLKCR1_CANCLKDIV_Pos);
}

/**
 * @brief Returns CANx2CLK prescaler.
 * @returns The CANx2CLK prescaler. Can be one of CRU_ClkDiv_TypeDef values.
 */
__STATIC_INLINE CRU_ClkDiv_TypeDef CRU_GetCANx2CLKPrescaler(void)
{
    return (CRU_ClkDiv_TypeDef)(READ_BIT(CRU->CLKCR1, CRU_CLKCR1_CANCLKDIV) >> CRU_CLKCR1_CANCLKDIV_Pos);
}

/**
 * @brief Enables CANx2CLK prescaler.
 */
__STATIC_INLINE void CRU_CANx2CLKPrescaler_Enable(void)
{
    SET_BIT(CRU->CLKCR1, CRU_CLKCR1_CANCLKDIVEN);
}

/**
 * @brief Disables CANx2CLK prescaler.
 */
__STATIC_INLINE void CRU_CANx2CLKPrescaler_Disable(void)
{
    CLEAR_BIT(CRU->CLKCR1, CRU_CLKCR1_CANCLKDIVEN);
}

/**
 * @brief Checks if CANx2CLK prescaler is enabled.
 * @returns 1 if the prescaler is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_CANx2CLKPrescaler_IsEnabled(void)
{
    return ((READ_BIT(CRU->CLKCR1, CRU_CLKCR1_CANCLKDIVEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures CANx2CLK source.
 * @param Source The CANx2CLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE void CRU_SetCANx2CLKSource(CRU_ClkSrc_TypeDef Source)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_CANCLKSEL, ((uint32_t)Source) << CRU_CLKSEL_CANCLKSEL_Pos);
}

/**
 * @brief Returns CANx2CLK source.
 * @returns The CANx2CLK source. Can be one of CRU_ClkSrc_TypeDef values.
 */
__STATIC_INLINE CRU_ClkSrc_TypeDef CRU_GetCANx2CLKSource(void)
{
    return (CRU_ClkSrc_TypeDef)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_CANCLKSEL) >> CRU_CLKSEL_CANCLKSEL_Pos);
}

/**
 * @brief Sets CCLK 1MHz pulse prescaler.
 * @note The actual value of the prescaler in one more than the value written.
 * @note CCLK 1MHz pulse is used to drive CORE0 and CORE1 system timer.
 * @param Prescaler The prescaler value.
 */
__STATIC_INLINE void CRU_SetCCLK1MHzPrescaler(uint32_t Prescaler)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_CCLK1MHZ, (Prescaler << CRU_CLKSEL_CCLK1MHZ_Pos) & CRU_CLKSEL_CCLK1MHZ);
}

/**
 * @brief Returns CCLK 1MHz pulse prescaler.
 * @note The actual value of the prescaler in one more than the value returned.
 * @note CCLK 1MHz pulse is used to drive CORE0 and CORE1 system timer.
 * @returns The prescaler value.
 */
__STATIC_INLINE uint32_t CRU_GetCCLK1MHzPrescaler(void)
{
    return (uint32_t)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_CCLK1MHZ) >> CRU_CLKSEL_CCLK1MHZ_Pos);
}

/**
 * @brief Sets HCLK 1MHz pulse prescaler.
 * @note The actual value of the prescaler in one more than the value written.
 * @note HCLK 1MHz pulse is used to drive CORE2 system timer.
 * @param Prescaler The prescaler value.
 */
__STATIC_INLINE void CRU_SetHCLK1MHzPrescaler(uint32_t Prescaler)
{
    MODIFY_REG(CRU->CLKSEL, CRU_CLKSEL_HCLK1MHZ, (Prescaler << CRU_CLKSEL_HCLK1MHZ_Pos) & CRU_CLKSEL_HCLK1MHZ);
}

/**
 * @brief Returns HCLK 1MHz pulse prescaler.
 * @note The actual value of the prescaler in one more than the value returned.
 * @note HCLK 1MHz pulse is used to drive CORE2 system timer.
 * @returns The prescaler value.
 */
__STATIC_INLINE uint32_t CRU_GetHCLK1MHzPrescaler(void)
{
    return (uint32_t)(READ_BIT(CRU->CLKSEL, CRU_CLKSEL_HCLK1MHZ) >> CRU_CLKSEL_HCLK1MHZ_Pos);
}

/******************************************************************************/
/*                                Core control                                */
/******************************************************************************/

/**
 * @brief Enables Core 1 clock.
 */
__STATIC_INLINE void CRU_C1_EnableClock(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CLKEN);
}

/**
 * @brief Disables Core 1 clock.
 */
__STATIC_INLINE void CRU_C1_DisableClock(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CLKEN);
}

/**
 * @brief Checks if Core 1 clock is enabled.
 * @returns 1 if the clock is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C1_IsEnabledClock(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CLKEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 1 reset.
 */
__STATIC_INLINE void CRU_C1_ForceReset(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1RSTN);
}

/**
 * @brief Releases Core 1 reset.
 */
__STATIC_INLINE void CRU_C1_ReleaseReset(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1RSTN);
}

/**
 * @brief Checks if Core 1 is in reset state.
 * @returns 1 if Core 1 is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C1_IsReset(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1RSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 1 TCM arbitrator (Front Port) reset.
 */
__STATIC_INLINE void CRU_C1_ForceResetFP(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1FPRSTN);
}

/**
 * @brief Releases Core 1 TCM arbitrator (Front Port) reset.
 */
__STATIC_INLINE void CRU_C1_ReleaseResetFP(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1FPRSTN);
}

/**
 * @brief Checks if Core 1 TCM arbitrator (Front Port) is in reset state.
 * @returns 1 if Core 1 TCM arbitrator is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C1_IsResetFP(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1FPRSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 1 processor complex reset.
 */
__STATIC_INLINE void CRU_C1_ForceResetCx(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CXRSTN);
}

/**
 * @brief Releases Core 1 processor complex reset.
 */
__STATIC_INLINE void CRU_C1_ReleaseResetCx(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CXRSTN);
}

/**
 * @brief Checks if Core 1 processor complex is in reset state.
 * @returns 1 if Core 1 processor complex is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C1_IsResetCx(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CXRSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 1 and its entire subsystem reset.
 * @note Core 1 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 */
__STATIC_INLINE void CRU_C1_ForceResetAll(void)
{
    CLEAR_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE1CXRSTN |
                              CRU_SYSCR0_CORE1FPRSTN | 
                              CRU_SYSCR0_CORE1RSTN));
}

/**
 * @brief Releases Core 1 and its entire subsystem reset.
 * @note Core 1 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 */
__STATIC_INLINE void CRU_C1_ReleaseResetAll(void)
{
    SET_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE1CXRSTN |
                            CRU_SYSCR0_CORE1FPRSTN | 
                            CRU_SYSCR0_CORE1RSTN));
}

/**
 * @brief Checks if Core 1 and its entire subsystem is in reset state.
 * @note Core 1 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 * @returns 1 if Core 1 entire subsystem is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C1_IsResetAll(void)
{
    return ((READ_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE1CXRSTN |
                                      CRU_SYSCR0_CORE1FPRSTN |
                                      CRU_SYSCR0_CORE1RSTN)) == 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Starts Core 1.
 * @note Core 1 must NOT be running when calling this function.
 * @param RstVec Core 1 reset vector.
 */
__STATIC_INLINE void CRU_C1_Start(uint32_t RstVec)
{
    /* Set Core 1 reset vector */
    WRITE_REG(CRU->SYSCR1, RstVec);

    /* Enable Core 1 clock */
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE1CLKEN);

    /* Release Core 1 from reset state */
    SET_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE1CXRSTN |
                            CRU_SYSCR0_CORE1FPRSTN | 
                            CRU_SYSCR0_CORE1RSTN));
}

/**
 * @brief Stops Core 1.
 */
__STATIC_INLINE void CRU_C1_Stop(void)
{
    /* Reset Core 1 entire subsystem */
    CLEAR_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE1CXRSTN |
                              CRU_SYSCR0_CORE1FPRSTN | 
                              CRU_SYSCR0_CORE1RSTN));
}

/**
 * @brief Forces Core 2 reset.
 */
__STATIC_INLINE void CRU_C2_ForceReset(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2RSTN);
}

/**
 * @brief Releases Core 2 reset.
 */
__STATIC_INLINE void CRU_C2_ReleaseReset(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2RSTN);
}

/**
 * @brief Checks if Core 2 is in reset state.
 * @returns 1 if Core 2 is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C2_IsReset(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2RSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 2 TCM arbitrator (Front Port) reset.
 */
__STATIC_INLINE void CRU_C2_ForceResetFP(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2FPRSTN);
}

/**
 * @brief Releases Core 2 TCM arbitrator (Front Port) reset.
 */
__STATIC_INLINE void CRU_C2_ReleaseResetFP(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2FPRSTN);
}

/**
 * @brief Checks if Core 2 TCM arbitrator (Front Port) is in reset state.
 * @returns 1 if Core 2 TCM arbitrator is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C2_IsResetFP(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2FPRSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 2 processor complex reset.
 */
__STATIC_INLINE void CRU_C2_ForceResetCx(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2CXRSTN);
}

/**
 * @brief Releases Core 2 processor complex reset.
 */
__STATIC_INLINE void CRU_C2_ReleaseResetCx(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2CXRSTN);
}

/**
 * @brief Checks if Core 2 processor complex is in reset state.
 * @returns 1 if Core 2 processor complex is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C2_IsResetCx(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_CORE2CXRSTN) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Forces Core 2 and its entire subsystem reset.
 * @note Core 2 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 */
__STATIC_INLINE void CRU_C2_ForceResetAll(void)
{
    CLEAR_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE2CXRSTN |
                              CRU_SYSCR0_CORE2FPRSTN | 
                              CRU_SYSCR0_CORE2RSTN));
}

/**
 * @brief Releases Core 2 and its entire subsystem reset.
 * @note Core 2 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 */
__STATIC_INLINE void CRU_C2_ReleaseResetAll(void)
{
    SET_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE2CXRSTN |
                            CRU_SYSCR0_CORE2FPRSTN | 
                            CRU_SYSCR0_CORE2RSTN));
}

/**
 * @brief Checks if Core 2 and its entire subsystem is in reset state.
 * @note Core 2 subsystem includes the core itself, processor complex and
 *       TCM arbitrator (Front Port).
 * @returns 1 if Core 2 entire subsystem is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_C2_IsResetAll(void)
{
    return ((READ_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE2CXRSTN |
                                      CRU_SYSCR0_CORE2FPRSTN |
                                      CRU_SYSCR0_CORE2RSTN)) == 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Starts Core 2.
 * @note Core 2 must NOT be running when calling this function.
 * @param RstVec Core 2 reset vector.
 */
__STATIC_INLINE void CRU_C2_Start(uint32_t RstVec)
{
    /* Set Core 2 reset vector */
    WRITE_REG(CRU->SYSCR2, RstVec);

    /* Release Core 2 from reset state */
    SET_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE2CXRSTN |
                            CRU_SYSCR0_CORE2FPRSTN | 
                            CRU_SYSCR0_CORE2RSTN));
}

/**
 * @brief Stops Core 2.
 */
__STATIC_INLINE void CRU_C2_Stop(void)
{
    /* Reset Core 2 entire subsystem */
    CLEAR_BIT(CRU->SYSCR0, (CRU_SYSCR0_CORE2CXRSTN |
                              CRU_SYSCR0_CORE2FPRSTN | 
                              CRU_SYSCR0_CORE2RSTN));
}

/******************************************************************************/
/*                             Peripheral control                             */
/******************************************************************************/

/**
 * @brief Enables APB0 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB0Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB0_EnableClock(CRU_APB0Periph_TypeDef Periph)
{
    SET_BIT(CRU->PCLK0EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Disables APB0 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB0Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB0_DisableClock(CRU_APB0Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK0EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Checks if APB0 peripheral clock is enabled.
 * @param Periph The peripheral. Can be one of CRU_APB0Periph_TypeDef values.
 * @returns 1 if the peripheral clock is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_APB0_IsEnabledClock(CRU_APB0Periph_TypeDef Periph)
{
    return ((READ_BIT(CRU->PCLK0EN, (1UL << (int32_t)Periph) & 0xFFFFUL) != 0UL) ? 1UL : 0UL);
}

#if defined (BE_U1000)

/**
 * @brief Resets APB0 peripheral.
 * @param Periph The peripheral. Can be one of CRU_APB0Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB0_ResetPeriph(CRU_APB0Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK0EN, ((1UL << ((int32_t)Periph)) << 16));
    __ASM volatile("nop");
    SET_BIT(CRU->PCLK0EN, ((1UL << ((int32_t)Periph)) << 16));
}

#endif

/**
 * @brief Enables APB1 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB1Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB1_EnableClock(CRU_APB1Periph_TypeDef Periph)
{
    SET_BIT(CRU->PCLK1EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Disables APB1 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB1Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB1_DisableClock(CRU_APB1Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK1EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Checks if APB1 peripheral clock is enabled.
 * @param Periph The peripheral. Can be one of CRU_APB1Periph_TypeDef values.
 * @returns 1 if the peripheral clock is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_APB1_IsEnabledClock(CRU_APB1Periph_TypeDef Periph)
{
    return ((READ_BIT(CRU->PCLK1EN, (1UL << (int32_t)Periph) & 0xFFFFUL) != 0UL) ? 1UL : 0UL);
}

#if defined (BE_U1000)

/**
 * @brief Resets APB1 peripheral.
 * @param Periph The peripheral. Can be one of CRU_APB1Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB1_ResetPeriph(CRU_APB1Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK1EN, ((1UL << ((int32_t)Periph)) << 16));
    __ASM volatile("nop");
    SET_BIT(CRU->PCLK1EN, ((1UL << ((int32_t)Periph)) << 16));
}

#endif

/**
 * @brief Enables APB2 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB2Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB2_EnableClock(CRU_APB2Periph_TypeDef Periph)
{
    SET_BIT(CRU->PCLK2EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Disables APB2 peripheral clock.
 * @param Periph The peripheral. Can be one of CRU_APB2Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB2_DisableClock(CRU_APB2Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK2EN, (1UL << (int32_t)Periph) & 0xFFFFUL);
}

/**
 * @brief Checks if APB2 peripheral clock is enabled.
 * @param Periph The peripheral. Can be one of CRU_APB2Periph_TypeDef values.
 * @returns 1 if the peripheral clock is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_APB2_IsEnabledClock(CRU_APB2Periph_TypeDef Periph)
{
    return ((READ_BIT(CRU->PCLK2EN, (1UL << (int32_t)Periph) & 0xFFFFUL) != 0UL) ? 1UL : 0UL);
}

#if defined (BE_U1000)

/**
 * @brief Resets APB2 peripheral.
 * @param Periph The peripheral. Can be one of CRU_APB2Periph_TypeDef values.
 */
__STATIC_INLINE void CRU_APB2_ResetPeriph(CRU_APB2Periph_TypeDef Periph)
{
    CLEAR_BIT(CRU->PCLK2EN, ((1UL << ((int32_t)Periph)) << 16));
    __ASM volatile("nop");
    SET_BIT(CRU->PCLK2EN, ((1UL << ((int32_t)Periph)) << 16));
}

#endif

/******************************************************************************/
/*                              AXI Interconnect                              */
/******************************************************************************/

/*  AXI Interconnect commutation scheme

    BMCU_U

    +--------------------------------------------------------------------------------------------------------------------+
    |                         |  MP Core0  |  DIO Core0 |  MP Core1  |  DIO Core1 |    DMA0    |    DMA1    |    DMA2    |
    |                         |     M1     |     M2     |     M3     |     M4     |     M5     |     M6     |     M7     |
    +--------------------------------------------------------------------------------------------------------------------+
    |  FP Core0  |     S1     |     no     |     RW     |     no     |     RW     |     RW     |     no     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |  FP Core1  |     S2     |     no     |     RW     |     no     |     RW     |     no     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    SRAM    |     S3     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |   EFLASH   |     S4     |     R      |     RW     |     R      |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB0    |     S5     |     R      |     RW     |     R      |     RW     |     RW     |     no     |     no     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB1    |     S6     |     R      |     RW     |     R      |     RW     |     no     |     RW     |     no     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB2    |     S7     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    AHB     |     S8     |     no     |     RW     |     no     |     RW     |     no     |     no     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+

    BE_U1000

    +--------------------------------------------------------------------------------------------------------------------+
    |                         |  MP Core0  |  DIO Core0 |  MP Core1  |  DIO Core1 |    DMA0    |    DMA1    |   USB DMA  |
    |                         |     M1     |     M2     |     M3     |     M4     |     M5     |     M6     |     M7     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB0    |     S1     |     R      |     RW     |     R      |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB1    |     S2     |     R      |     RW     |     R      |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    APB2    |     S3     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    AHB     |     S4     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |  FP Core0  |     S5     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |    SRAM    |     S6     |     no     |     RW     |     no     |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+
    |   EFLASH   |     S7     |     R      |     RW     |     R      |     RW     |     RW     |     RW     |     RW     |
    +--------------------------------------------------------------------------------------------------------------------+

*/

/**
 * @brief Sets AXI master priority.
 * @param Master The AXI master. Can be one of CRU_AXIMaster_TypeDef values.
 * @param Priority The priority. Can be one of CRU_AXIPriority_TypeDef values.
 */
__STATIC_INLINE void CRU_AXI_SetMasterPriority(CRU_AXIMaster_TypeDef Master, CRU_AXIPriority_TypeDef Priority)
{
    MODIFY_REG(CRU->PRIOR0, (uint32_t)Master, ((uint32_t)Priority) << POSITION_VAL((uint32_t)Master));
}

/**
 * @brief Returns AXI master priority.
 * @param Master The AXI master. Can be one of CRU_AXIMaster_TypeDef values.
 * @returns The priority. Can be one of CRU_AXIPriority_TypeDef values.
 */
__STATIC_INLINE CRU_AXIPriority_TypeDef CRU_AXI_GetMasterPriority(CRU_AXIMaster_TypeDef Master)
{
    return (CRU_AXIPriority_TypeDef)(READ_BIT(CRU->PRIOR0, (uint32_t)Master) >> POSITION_VAL((uint32_t)Master));
}

/**
 * @brief Sets AXI slave priority.
 * @param Slave The AXI slave. Can be one of CRU_AXISlave_TypeDef values.
 * @param Priority The priority. Can be one of CRU_AXIPriority_TypeDef values.
 */
__STATIC_INLINE void CRU_AXI_SetSlavePriority(CRU_AXISlave_TypeDef Slave, CRU_AXIPriority_TypeDef Priority)
{
    MODIFY_REG(CRU->PRIOR1, (uint32_t)Slave, ((uint32_t)Priority) << POSITION_VAL((uint32_t)Slave));
}

/**
 * @brief Returns AXI slave priority.
 * @param Slave The AXI slave. Can be one of CRU_AXISlave_TypeDef values.
 * @returns The priority. Can be one of CRU_AXIPriority_TypeDef values.
 */
__STATIC_INLINE CRU_AXIPriority_TypeDef CRU_AXI_GetSlavePriority(CRU_AXISlave_TypeDef Slave)
{
    return (CRU_AXIPriority_TypeDef)(READ_BIT(CRU->PRIOR1, (uint32_t)Slave) >> POSITION_VAL((uint32_t)Slave));
}

/******************************************************************************/
/*                                Pin control                                 */
/******************************************************************************/

/**
 * @brief Configures pin pull-up or pull-down.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be a combination of CRU_PIN_x values.
 * @param Pull The pin pull state. Can be one of CRU_PinPull_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPinPull(uint8_t Port, uint16_t Pin, CRU_PinPull_TypeDef Pull)
{
    __IO uint32_t* PullUpRegAddr = (__IO uint32_t*)((uint32_t)&(CRU->IOPUCR0) + (Port >> 1) * 0x4UL);
    __IO uint32_t* PullDownRegAddr = (__IO uint32_t*)((uint32_t)&(CRU->IOPDCR0) + (Port >> 1) * 0x4UL);

    uint8_t Offset = 16U * (Port % 2);

    if (Pull == CRU_PIN_PULL_UP)
    {
        /* Pull-Up */
        SET_BIT(*PullUpRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
        CLEAR_BIT(*PullDownRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
    }
    else if (Pull == CRU_PIN_PULL_DOWN)
    {
        /* Pull-Down */
        CLEAR_BIT(*PullUpRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
        SET_BIT(*PullDownRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
    }
    else if (Pull == CRU_PIN_PULL_UP_DOWN)
    {
        /* Pull-Down */
        SET_BIT(*PullUpRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
        SET_BIT(*PullDownRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
    }
    else
    {
        /* No Pull */
        CLEAR_BIT(*PullUpRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
        CLEAR_BIT(*PullDownRegAddr, ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset);
    }
}

/**
 * @brief Returns pin pull-up or pull-down state.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @returns The pin pull state. Can be one of CRU_PinPull_TypeDef values.
 */
__STATIC_INLINE CRU_PinPull_TypeDef CRU_GetPinPull(uint8_t Port, uint16_t Pin)
{
    __IO uint32_t* PullUpRegAddr = (__IO uint32_t*)((uint32_t)&(CRU->IOPUCR0) + (Port >> 1) * 0x4UL);
    __IO uint32_t* PullDownRegAddr = (__IO uint32_t*)((uint32_t)&(CRU->IOPDCR0) + (Port >> 1) * 0x4UL);

    uint8_t Offset = 16U * (Port % 2);

    uint32_t Pull = (uint32_t)CRU_PIN_PULL_NO;
    uint32_t Mask = ((uint32_t)(Pin & CRU_PIN_ALL)) << Offset;

    /* Check for Pull-Up */
    if (READ_BIT(*PullUpRegAddr, Mask) == Mask)
    {
        Pull |= (uint32_t)CRU_PIN_PULL_UP;
    }
    
    /* Check for Pull-Down */
    if (READ_BIT(*PullDownRegAddr, Mask) == Mask)
    {
        Pull |= (uint32_t)CRU_PIN_PULL_DOWN;
    }

    return (CRU_PinPull_TypeDef)Pull;
}

/*  Pin alternate functions

    BMCU_U 88-pin

    +-------------------------------------------------------------------------------------------------------+
    |  Pin name  | Pin number |     AF0    |     AF1    |     AF2    |     AF3    |     AF4    |     AF5    |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[0]    |     61     | GPIO       | SSPI0_CSN  | QSPI0_CSN0 | QSPI0_CSN  | TCK        | I2C0_SCL   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[1]    |     60     | GPIO       | SSPI0_SCK  | QSPI0_SCK  | QSPI0_SCK  | TMS        | I2C0_SDA   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[2]    |     59     | GPIO       | SSPI0_MISO | QSPI0_MOSI | QSPI0_IO0  | TDO        | UART0_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[3]    |     58     | GPIO       | SSPI0_MOSI | QSPI0_MISO | QSPI0_IO1  | TDI        | UART0_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[4]    |     57     | GPIO       | I2C0_SCL   | QSPI0_CSN1 | QSPI0_IO2  | TRST       | UART1_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[5]    |     56     | GPIO       | I2C0_SDA   | QSPI0_CSN2 | QSPI0_IO3  | PWMA0_QA   | UART1_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[6]    |     53     | GPIO       | UART0_TX   | CAN0_TX    | I2C0_SCL   | PWMA0_QB   | PWMG0      |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[7]    |     52     | GPIO       | UART0_RX   | CAN0_RX    | I2C0_SDA   | PWMA0_INDX | PWMA0_ETR  |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[8]    |     51     | GPIO       | MSPI1_CSN  | I2C0_SCL   | UART0_TX   | PWMA0_CH0P | PWMA0_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[9]    |     50     | GPIO       | MSPI1_SCK  | I2C0_SDA   | UART0_RX   | PWMA0_CH0N | PWMA0_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[10]   |     46     | GPIO       | MSPI1_MOSI | UART1_TX   | I2S0_SDO   | PWMA0_CH1P | PWMA0_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[11]   |     45     | GPIO       | MSPI1_MISO | UART1_RX   | I2S0_SDI   | PWMA0_CH1N | PWMA0_CH3P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[12]   |     43     | GPIO       | I2C1_SCL   | UART2_TX   | I2S0_WS    | PWMA0_CH2P | TIMER0_PWM0|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[13]   |     42     | GPIO       | I2C1_SDA   | UART2_RX   | I2S0_SCLK  | PWMA0_CH2N | TIMER0_PWM1|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[14]   |     41     | GPIO       | UART1_TX   | I2C1_SCL   | CAN0_TX    | PWMA0_BRK  | TIMER0_PWM2|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[15]   |     39     | GPIO       | UART1_RX   | I2C1_SDA   | CAN0_RX    | PWMG0      | TIMER0_PWM3|
    +------------+------------------------------------------------------------------------------------------+
    |   PB[0]    |     12     | GPIO       | SSPI2_CSN  | QSPI1_CSN0 | QSPI1_CSN  | PIB_D[0]   | I2C2_SCL   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[1]    |     11     | GPIO       | SSPI2_SCK  | QSPI1_SCK  | QSPI1_SCK  | PIB_D[1]   | I2C2_SDA   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[2]    |     9      | GPIO       | SSPI2_MISO | QSPI1_MOSI | QSPI1_IO0  | PIB_D[2]   | UART3_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[3]    |     8      | GPIO       | SSPI2_MOSI | QSPI1_MISO | QSPI1_IO1  | PIB_D[3]   | UART3_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[4]    |     7      | GPIO       | I2C2_SCL   | QSPI1_CSN1 | QSPI1_IO2  | PIB_CLK    | UART4_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[5]    |     6      | GPIO       | I2C2_SDA   | QSPI1_CSN2 | QSPI1_IO3  | PWMA1_QA   | UART4_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[6]    |     5      | GPIO       | UART3_TX   | CAN1_TX    | I2C2_SCL   | PWMA1_QB   | PWMG1      |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[7]    |     4      | GPIO       | UART3_RX   | CAN1_RX    | I2C2_SDA   | PWMA1_INDX | PWMA1_ETR  |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[8]    |     3      | GPIO       | MSPI3_CSN  | I2C2_SCL   | UART3_TX   | PWMA1_CH0P | PWMA1_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[9]    |     2      | GPIO       | MSPI3_SCK  | I2C2_SDA   | UART3_RX   | PWMA1_CH0N | PWMA1_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[10]   |     88     | GPIO       | MSPI3_MOSI | UART4_TX   | I2S1_SDO   | PWMA1_CH1P | PWMA1_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[11]   |     87     | GPIO       | MSPI3_MISO | UART4_RX   | I2S1_SDI   | PWMA1_CH1N | PWMA1_CH3P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[12]   |     86     | GPIO       | I2C3_SCL   | UART5_TX   | I2S1_WS    | PWMA1_CH2P | TIMER1_PWM0|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[13]   |     85     | GPIO       | I2C3_SDA   | UART5_RX   | I2S1_SCLK  | PWMA1_CH2N | TIMER1_PWM1|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[14]   |     84     | GPIO       | UART4_TX   | I2C3_SCL   | CAN1_TX    | PWMA1_BRK  | TIMER1_PWM2|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[15]   |     83     | GPIO       | UART4_RX   | I2C3_SDA   | CAN1_RX    | PWMG1      | TIMER1_PWM3|
    +------------+------------------------------------------------------------------------------------------+
    |   PC[0]    |     81     | GPIO       | PIO[0]     | PWMA2_CH0P | TCK        | PWMA3_INDX | PIB_D[0]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[1]    |     80     | GPIO       | PIO[1]     | PWMA2_CH0N | TMS        | PWMA3_QB   | PIB_D[1]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[2]    |     79     | GPIO       | PIO[2]     | PWMA2_CH1P | TDO        | PWMA3_QA   | PIB_D[2]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[3]    |     78     | GPIO       | PIO[3]     | PWMA2_CH1N | TDI        | PWMA3_BRK  | PIB_D[3]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[4]    |     75     | GPIO       | PIO[4]     | PWMA2_CH2P | TRST       | PWMA3_CH3P | PIB_CLK    |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[5]    |     74     | GPIO       | PIO[5]     | PWMA2_CH2N | PWMA2_CH0P | PWMA3_ETR  | PWMA3_CH2N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[6]    |     73     | GPIO       | PIO[6]     | UART6_TX   | PWMA2_CH0N | UART7_DE   | PWMA3_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[7]    |     72     | GPIO       | PIO[7]     | UART6_RX   | PWMA2_CH1P | UART7_RE   | PWMA3_CH1N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[8]    |     70     | GPIO       | PIO[8]     | UART6_DE   | PWMA2_CH1N | UART7_TX   | PWMA3_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[9]    |     69     | GPIO       | PIO[9]     | UART6_RE   | PWMA2_CH2P | UART7_RX   | PWMA3_CH0N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[10]   |     68     | GPIO       | PIO[10]    | PWMA2_ETR  | PWMA2_CH2N | PWMA3_CH2N | PWMA3_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[11]   |     67     | GPIO       | PIO[11]    | PWMA2_CH3P | PIB_D[0]   | PWMA3_CH2P | TCK        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[12]   |     65     | GPIO       | PIO[12]    | PWMA2_BRK  | PIB_D[1]   | PWMA3_CH1N | TMS        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[13]   |     64     | GPIO       | PIO[13]    | PWMA2_QA   | PIB_D[2]   | PWMA3_CH1P | TDO        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[14]   |     63     | GPIO       | PIO[14]    | PWMA2_QB   | PIB_D[3]   | PWMA3_CH0N | TDI        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[15]   |     62     | GPIO       | PIO[15]    | PWMA2_INDX | PIB_CLK    | PWMA3_CH0P | DRVVBUS    |
    +------------+------------------------------------------------------------------------------------------+

    BE_U1000 88-pin

    +-------------------------------------------------------------------------------------------------------+
    |  Pin name  | Pin number |     AF0    |     AF1    |     AF2    |     AF3    |     AF4    |     AF5    |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[0]    |     61     | GPIO       | SSPI0_CSN  | QSPI0_CSN0 | QSPI0_CSN  | TCK        | I2C0_SCL   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[1]    |     60     | GPIO       | SSPI0_SCK  | QSPI0_SCK  | QSPI0_SCK  | TMS        | I2C0_SDA   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[2]    |     59     | GPIO       | SSPI0_MISO | QSPI0_MOSI | QSPI0_IO0  | TDO        | UART0_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[3]    |     58     | GPIO       | SSPI0_MOSI | QSPI0_MISO | QSPI0_IO1  | TDI        | UART0_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[4]    |     57     | GPIO       | I2C0_SCL   | QSPI0_CSN1 | QSPI0_IO2  | TRST       | UART1_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[5]    |     56     | GPIO       | I2C0_SDA   | QSPI0_CSN2 | QSPI0_IO3  | PWMA0_QA   | UART1_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[6]    |     53     | GPIO       | UART0_TX   | CAN0_TX    | I2C0_SCL   | PWMA0_QB   | PWMG0      |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[7]    |     52     | GPIO       | UART0_RX   | CAN0_RX    | I2C0_SDA   | PWMA0_INDX | PWMA0_ETR  |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[8]    |     51     | GPIO       | MSPI1_CSN  | I2C0_SCL   | UART0_TX   | PWMA0_CH0P | PWMA0_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[9]    |     50     | GPIO       | MSPI1_SCK  | I2C0_SDA   | UART0_RX   | PWMA0_CH0N | PWMA0_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[10]   |     46     | GPIO       | MSPI1_MOSI | UART1_TX   | I2S0_SDO   | PWMA0_CH1P | PWMA0_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[11]   |     45     | GPIO       | MSPI1_MISO | UART1_RX   | I2S0_SDI   | PWMA0_CH1N | PWMA0_CH3P |
    +-------------------------------------------------------------------------------------------------------+
    |   PA[12]   |     43     | GPIO       | I2C1_SCL   | UART2_TX   | I2S0_WS    | PWMA0_CH2P | TIMER0_PWM0|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[13]   |     42     | GPIO       | I2C1_SDA   | UART2_RX   | I2S0_SCLK  | PWMA0_CH2N | TIMER0_PWM1|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[14]   |     41     | GPIO       | UART1_TX   | I2C1_SCL   | CAN0_TX    | PWMA0_BRK  | TIMER0_PWM2|
    +-------------------------------------------------------------------------------------------------------+
    |   PA[15]   |     39     | GPIO       | UART1_RX   | I2C1_SDA   | CAN0_RX    | PWMG0      | TIMER0_PWM3|
    +------------+------------------------------------------------------------------------------------------+
    |   PB[0]    |     12     | GPIO       | SSPI2_CSN  | QSPI1_CSN0 | QSPI1_CSN  | PIB_D[0]   | I2C2_SCL   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[1]    |     11     | GPIO       | SSPI2_SCK  | QSPI1_SCK  | QSPI1_SCK  | PIB_D[1]   | I2C2_SDA   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[2]    |     9      | GPIO       | SSPI2_MISO | QSPI1_MOSI | QSPI1_IO0  | PIB_D[2]   | UART3_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[3]    |     8      | GPIO       | SSPI2_MOSI | QSPI1_MISO | QSPI1_IO1  | PIB_D[3]   | UART3_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[4]    |     7      | GPIO       | I2C2_SCL   | QSPI1_CSN1 | QSPI1_IO2  | PIB_CLK    | UART4_TX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[5]    |     6      | GPIO       | I2C2_SDA   | QSPI1_CSN2 | QSPI1_IO3  | PWMA1_QA   | UART4_RX   |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[6]    |     5      | GPIO       | UART3_TX   | CAN1_TX    | I2C2_SCL   | PWMA1_QB   | PWMG1      |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[7]    |     4      | GPIO       | UART3_RX   | CAN1_RX    | I2C2_SDA   | PWMA1_INDX | PWMA1_ETR  |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[8]    |     3      | GPIO       | MSPI3_CSN  | I2C2_SCL   | UART3_TX   | PWMA1_CH0P | PWMA1_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[9]    |     2      | GPIO       | MSPI3_SCK  | I2C2_SDA   | UART3_RX   | PWMA1_CH0N | PWMA1_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[10]   |     88     | GPIO       | MSPI3_MOSI | UART4_TX   | I2S1_SDO   | PWMA1_CH1P | PWMA1_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[11]   |     87     | GPIO       | MSPI3_MISO | UART4_RX   | I2S1_SDI   | PWMA1_CH1N | PWMA1_CH3P |
    +-------------------------------------------------------------------------------------------------------+
    |   PB[12]   |     86     | GPIO       | I2C3_SCL   | UART5_TX   | I2S1_WS    | PWMA1_CH2P | TIMER1_PWM0|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[13]   |     85     | GPIO       | I2C3_SDA   | UART5_RX   | I2S1_SCLK  | PWMA1_CH2N | TIMER1_PWM1|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[14]   |     84     | GPIO       | UART4_TX   | I2C3_SCL   | CAN1_TX    | PWMA1_BRK  | TIMER1_PWM2|
    +-------------------------------------------------------------------------------------------------------+
    |   PB[15]   |     83     | GPIO       | UART4_RX   | I2C3_SDA   | CAN1_RX    | PWMG1      | TIMER1_PWM3|
    +------------+------------------------------------------------------------------------------------------+
    |   PC[0]    |     81     | GPIO       | PIO[0]     | PWMA2_CH0P | TCK        | PWMA3_INDX | PIB_D[0]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[1]    |     80     | GPIO       | PIO[1]     | PWMA2_CH0N | TMS        | PWMA3_QB   | PIB_D[1]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[2]    |     79     | GPIO       | PIO[2]     | PWMA2_CH1P | TDO        | PWMA3_QA   | PIB_D[2]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[3]    |     78     | GPIO       | PIO[3]     | PWMA2_CH1N | TDI        | PWMA3_BRK  | PIB_D[3]   |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[4]    |     75     | GPIO       | PIO[4]     | PWMA2_CH2P | DRVVBUS    | PWMA3_CH3P | PIB_CLK    |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[5]    |     74     | GPIO       | PIO[5]     | PWMA2_CH2N | PWMA2_CH0P | PWMA3_ETR  | PWMA3_CH2N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[6]    |     73     | GPIO       | PIO[6]     | UART6_TX   | PWMA2_CH0N | UART7_DE   | PWMA3_CH2P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[7]    |     72     | GPIO       | PIO[7]     | UART6_RX   | PWMA2_CH1P | UART7_RE   | PWMA3_CH1N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[8]    |     70     | GPIO       | PIO[8]     | UART6_DE   | PWMA2_CH1N | UART7_TX   | PWMA3_CH1P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[9]    |     69     | GPIO       | PIO[9]     | UART6_RE   | PWMA2_CH2P | UART7_RX   | PWMA3_CH0N |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[10]   |     68     | GPIO       | PIO[10]    | PWMA2_ETR  | PWMA2_CH2N | PWMA3_CH2N | PWMA3_CH0P |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[11]   |     67     | GPIO       | PIO[11]    | PWMA2_CH3P | PIB_D[0]   | PWMA3_CH2P | TCK        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[12]   |     65     | GPIO       | PIO[12]    | PWMA2_BRK  | PIB_D[1]   | PWMA3_CH1N | TMS        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[13]   |     64     | GPIO       | PIO[13]    | PWMA2_QA   | PIB_D[2]   | PWMA3_CH1P | TDO        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[14]   |     63     | GPIO       | PIO[14]    | PWMA2_QB   | PIB_D[3]   | PWMA3_CH0N | TDI        |
    +-------------------------------------------------------------------------------------------------------+
    |   PC[15]   |     62     | GPIO       | PIO[15]    | PWMA2_INDX | PIB_CLK    | PWMA3_CH0P | DRVVBUS    |
    +------------+------------------------------------------------------------------------------------------+

*/

/**
 * @brief Configures pin alternate function.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @param Alternate The alternate function. Can be one of CRU_PinAF_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPinAF(uint8_t Port, uint16_t Pin, CRU_PinAF_TypeDef Alternate)
{
    uint8_t Offset = (POSITION_VAL(Pin) & 0x7U) * 4U;

    MODIFY_REG(*(__IO uint32_t*)((uint32_t)&(CRU->IOAFCR0) + Port * 0x8UL + (POSITION_VAL(Pin) >> 3) * 0x4UL),
               CRU_IOAFCR0_ALT0 << Offset,
               ((uint32_t)Alternate & CRU_IOAFCR0_ALT0) << Offset);
}

/**
 * @brief Returns pin alternate function.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @returns The alternate function. Can be one of CRU_PinAF_TypeDef values.
 */
__STATIC_INLINE CRU_PinAF_TypeDef CRU_GetPinAF(uint8_t Port, uint16_t Pin)
{
    uint8_t Offset = (POSITION_VAL(Pin) & 0x7U) * 4U;

    return (CRU_PinAF_TypeDef)(READ_BIT(*(__IO uint32_t*)((uint32_t)&(CRU->IOAFCR0) + Port * 0x8UL + (POSITION_VAL(Pin) >> 3) * 0x4UL),
                                        CRU_IOAFCR0_ALT0 << Offset) >> Offset);
}

/**
 * @brief Enables pin input.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE void CRU_EnablePinInput(uint8_t Port, uint16_t Pin)
{
    SET_BIT(*(__IO uint32_t*)((uint32_t)&(CRU->IOAFCR0) + Port * 0x8UL + (POSITION_VAL(Pin) >> 3) * 0x4UL),
            CRU_IOAFCR0_IE0 << ((POSITION_VAL(Pin) & 0x7U) * 4U));
}

/**
 * @brief Disables pin input.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE void CRU_DisablePinInput(uint8_t Port, uint16_t Pin)
{
    CLEAR_BIT(*(__IO uint32_t*)((uint32_t)&(CRU->IOAFCR0) + Port * 0x8UL + (POSITION_VAL(Pin) >> 3) * 0x4UL),
              CRU_IOAFCR0_IE0 << ((POSITION_VAL(Pin) & 0x7U) * 4U));
}

/**
 * @brief Checks if pin input is enabled.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @returns 1 if input is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsEnabledPinInput(uint8_t Port, uint16_t Pin)
{
    return ((READ_BIT(*(__IO uint32_t*)((uint32_t)&(CRU->IOAFCR0) + Port * 0x8UL + (POSITION_VAL(Pin) >> 3) * 0x4UL),
                      CRU_IOAFCR0_IE0 << ((POSITION_VAL(Pin) & 0x7U) * 4U)) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures pin drive strength.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @param DriveStrength The drive strength. Can be one of CRU_PinDriveStrength_TypeDef values.
 */
__STATIC_INLINE void CRU_SetPinDriveStrength(uint8_t Port, uint16_t Pin, CRU_PinDriveStrength_TypeDef DriveStrength)
{
    uint8_t Offset = POSITION_VAL(Pin) * 2U;

    MODIFY_REG(*(__IO uint32_t*)((uint32_t)&(CRU->IODSCR0) + Port * 0x4UL),
               CRU_IODSCR0_PADS0 << Offset,
               ((uint32_t)DriveStrength & CRU_IODSCR0_PADS0) << Offset);
}

/**
 * @brief Returns pin drive strength.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 * @returns The drive strength. Can be one of CRU_PinDriveStrength_TypeDef values.
 */
__STATIC_INLINE CRU_PinDriveStrength_TypeDef CRU_GetPinDriveStrength(uint8_t Port, uint16_t Pin)
{
    uint8_t Offset = POSITION_VAL(Pin) * 2U;

    return (CRU_PinDriveStrength_TypeDef)(READ_BIT(*(__IO uint32_t*)((uint32_t)&(CRU->IODSCR0) + Port * 0x4UL),
                                                   CRU_IODSCR0_PADS0 << Offset) >> Offset);
}

/******************************************************************************/
/*                                    EXTI                                    */
/******************************************************************************/

/**
 * @brief Selects an external interrupt source pin for CORE0 and CORE1.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE void CRU_C0_C1_EXTI_SetSource(uint32_t Port, uint32_t Pin)
{
    uint8_t Offset = Port * 4U;

#if defined (BMCU_U)
    MODIFY_REG(CRU->INTCR0,
               CRU_INTCR0_PADAINTSEL << Offset,
               (((uint32_t)POSITION_VAL(Pin) << CRU_INTCR0_PADAINTSEL_Pos) & CRU_INTCR0_PADAINTSEL) << Offset);
#endif

#if defined (BE_U1000)
    MODIFY_REG(CRU->INTCR0,
               CRU_INTCR0_PADAINTSEL0 << Offset,
               (((uint32_t)POSITION_VAL(Pin) << CRU_INTCR0_PADAINTSEL0_Pos) & CRU_INTCR0_PADAINTSEL0) << Offset);
#endif
}

/**
 * @brief Returns an external interrupt source pin for CORE0 and CORE1.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @returns The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE uint32_t CRU_C0_C1_EXTI_GetSource(uint32_t Port)
{
    uint32_t Offset = Port * 4U;

#if defined (BMCU_U)
    return (0x1UL << (READ_BIT(CRU->INTCR0, CRU_INTCR0_PADAINTSEL << Offset) >> (CRU_INTCR0_PADAINTSEL_Pos + Offset)));
#endif

#if defined (BE_U1000)
    return (0x1UL << (READ_BIT(CRU->INTCR0, CRU_INTCR0_PADAINTSEL0 << Offset) >> (CRU_INTCR0_PADAINTSEL0_Pos + Offset)));
#endif
}

#if defined (BE_U1000)

/**
 * @brief Selects an external interrupt source pin for CORE2.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @param Pin The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE void CRU_C2_EXTI_SetSource(uint32_t Port, uint32_t Pin)
{
    uint8_t Offset = Port * 4U;

    MODIFY_REG(CRU->INTCR0,
               CRU_INTCR0_PADAINTSEL2 << Offset,
               (((uint32_t)POSITION_VAL(Pin) << CRU_INTCR0_PADAINTSEL2_Pos) & CRU_INTCR0_PADAINTSEL2) << Offset);
}

/**
 * @brief Returns an external interrupt source pin for CORE2.
 * @note Only one pin can be passed as parameter.
 * @param Port The GPIO port. Can be one of CRU_PORT_x values.
 * @returns The GPIO pin. Can be one of CRU_PIN_0 to CRU_PIN_15 values.
 */
__STATIC_INLINE uint32_t CRU_C2_EXTI_GetSource(uint32_t Port)
{
    uint32_t Offset = Port * 4U;

    return (0x1UL << (READ_BIT(CRU->INTCR0, CRU_INTCR0_PADAINTSEL2 << Offset) >> (CRU_INTCR0_PADAINTSEL2_Pos + Offset)));
}

#endif

/******************************************************************************/
/*                                  USB PHY                                   */
/******************************************************************************/

/**
 * @brief Forces USB PHY reset.
 */
__STATIC_INLINE void CRU_USB_PHY_ForceReset(void)
{
#if defined (BMCU_U)
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYRESET);
#endif

#if defined (BE_U1000)
    SET_BIT(CRU->USBCR, CRU_USBCR_RESET);
#endif
}

/**
 * @brief Releases USB PHY reset.
 */
__STATIC_INLINE void CRU_USB_PHY_ReleaseReset(void)
{
#if defined (BMCU_U)
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYRESET);
#endif

#if defined (BE_U1000)
    CLEAR_BIT(CRU->USBCR, CRU_USBCR_RESET);
#endif
}

/**
 * @brief Checks if USB PHY is in reset state.
 * @returns 1 if USB PHY is in reset state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_USB_PHY_IsReset(void)
{
#if defined (BMCU_U)
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYRESET) != 0UL) ? 1UL : 0UL);
#endif

#if defined (BE_U1000)
    return ((READ_BIT(CRU->USBCR, CRU_USBCR_RESET) != 0UL) ? 1UL : 0UL);
#endif
}

/**
 * @brief Enables USB PHY PLL.
 */
__STATIC_INLINE void CRU_USB_PHY_PLL_Enable(void)
{
#if defined (BMCU_U)
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYPLLEN);
#endif

#if defined (BE_U1000)
    SET_BIT(CRU->USBCR, CRU_USBCR_PLL_EN);
#endif
}

/**
 * @brief Disables USB PHY PLL.
 */
__STATIC_INLINE void CRU_USB_PHY_PLL_Disable(void)
{
#if defined (BMCU_U)
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYPLLEN);
#endif

#if defined (BE_U1000)
    CLEAR_BIT(CRU->USBCR, CRU_USBCR_PLL_EN);
#endif
}

/**
 * @brief Checks if USB PHY PLL is enabled.
 * @returns 1 if PLL is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_USB_PHY_PLL_IsEnabled(void)
{
#if defined (BMCU_U)
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_USBPHYPLLEN) != 0UL) ? 1UL : 0UL);
#endif

#if defined (BE_U1000)
    return ((READ_BIT(CRU->USBCR, CRU_USBCR_PLL_EN) != 0UL) ? 1UL : 0UL);
#endif
}

/******************************************************************************/
/*                                   Memory                                   */
/******************************************************************************/

/**
 * @brief Enables access to EFLASH NVR.
 */
__STATIC_INLINE void CRU_EFLASH_NVR_Enable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_NVR);
}

/**
 * @brief Disables access to EFLASH NVR.
 */
__STATIC_INLINE void CRU_EFLASH_NVR_Disable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_NVR);
}

/**
 * @brief Disables access to EFLASH NVR permanently.
 * @note After being disabled access can be restored only on power up or
 *       hardware reset.
 */
__STATIC_INLINE void CRU_EFLASH_NVR_DisablePermanently(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_EFLASHNVRDIS);
}

/**
 * @brief Checks if access to EFLASH NVR is enabled.
 * @returns 1 if access is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_EFLASH_NVR_IsEnabled(void)
{
    return (((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_NVR) != 0UL) &&
             (READ_BIT(CRU->SYSCR0, CRU_SYSCR0_EFLASHNVRDIS) == 0UL)) ? 1UL : 0UL);
}

/**
 * @brief Disables EFLASH manual mode permanently.
 * @note After being disabled manual mode can be restored only on power up or
 *       hardware reset.
 */
__STATIC_INLINE void CRU_EFLASH_ManualMode_DisablePermanently(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_EFLASHMANDIS);
}

#if defined (BE_U1000)

/**
 * @brief Disables EFLASH NVR 1st sector modification.
 * @note This operation is irreversible until reset.
 */
__STATIC_INLINE void CRU_EFLASH_NVR1_Disable(void)
{
    SET_BIT(CRU->FLASHNVRCR, CRU_FLASHNVRCR);
}

/**
 * @brief Checks if EFLASH NVR 1st sector modification is enabled.
 * @returns 1 if modification is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_EFLASH_NVR1_IsEnabled(void)
{
    return ((READ_BIT(CRU->FLASHNVRCR, CRU_FLASHNVRCR) == 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables EFLASH DMA mode access.
 */
__STATIC_INLINE void CRU_EFLASH_DMAMode_Enable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_DMAMODDIS);
}

/**
 * @brief Disables EFLASH DMA mode access.
 */
__STATIC_INLINE void CRU_EFLASH_DMAMode_Disable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_DMAMODDIS);
}

/**
 * @brief Checks if EFLASH DMA mode access is enabled.
 * @returns 1 if access is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_EFLASH_DMAMode_IsEnabled(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_DMAMODDIS) == 0UL) ? 1UL : 0UL);
}

#endif

/**
 * @brief Enables memory Extra Margin Adjustment (EMA).
 */
__STATIC_INLINE void CRU_EMA_Enable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_EMEN);
}

/**
 * @brief Disables memory Extra Margin Adjustment (EMA).
 */
__STATIC_INLINE void CRU_EMA_Disable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_EMEN);
}

/**
 * @brief Checks if memory Extra Margin Adjustment (EMA) is enabled.
 * @returns 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_EMA_IsEnabled(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_EMEN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Configures Single Port (SP) memory EMA parameter.
 * @note Higher Extra Margin Adjustment (EMA) parameter increases the length
 *       of the internal memory control signals. This improves reliability but
 *       decreases performance.
 * @param Param The EMA parameter value. Can be in the range of 0 to 7.
 */
__STATIC_INLINE void CRU_EMA_SetSPParam(uint8_t Param)
{
    MODIFY_REG(CRU->SYSCR0,
               CRU_SYSCR0_EMSPVAL,
               ((uint32_t)Param << CRU_SYSCR0_EMSPVAL_Pos) & CRU_SYSCR0_EMSPVAL);
}

 /**
 * @brief Returns Single Port (SP) memory EMA parameter.
 * @note Higher Extra Margin Adjustment (EMA) parameter increases the length
 *       of the internal memory control signals. This improves reliability but
 *       decreases performance.
 * @returns The EMA parameter value. Can be in the range of 0 to 7.
 */
__STATIC_INLINE uint8_t CRU_EMA_GetSPParam(void)
{
    return (uint8_t)(READ_BIT(CRU->SYSCR0, CRU_SYSCR0_EMSPVAL) >> CRU_SYSCR0_EMSPVAL_Pos);
}

/**
 * @brief Configures Dual Port (DP) memory EMA parameter.
 * @note Higher Extra Margin Adjustment (EMA) parameter increases the length
 *       of the internal memory control signals. This improves reliability but
 *       decreases performance.
 * @param Param The EMA parameter value. Can be in the range of 0 to 15.
 */
__STATIC_INLINE void CRU_EMA_SetDPParam(uint8_t Param)
{
    MODIFY_REG(CRU->SYSCR0,
               CRU_SYSCR0_EMDPVAL,
               ((uint32_t)Param << CRU_SYSCR0_EMDPVAL_Pos) & CRU_SYSCR0_EMDPVAL);
}

 /**
 * @brief Returns Dual Port (SP) memory EMA parameter.
 * @note Higher Extra Margin Adjustment (EMA) parameter increases the length
 *       of the internal memory control signals. This improves reliability but
 *       decreases performance.
 * @returns The EMA parameter value. Can be in the range of 0 to 15.
 */
__STATIC_INLINE uint8_t CRU_EMA_GetDPParam(void)
{
    return (uint8_t)(READ_BIT(CRU->SYSCR0, CRU_SYSCR0_EMDPVAL) >> CRU_SYSCR0_EMDPVAL_Pos);
}

/******************************************************************************/
/*                                    XIP                                     */
/******************************************************************************/

/**
 * @brief Enables XIP mode on QSPI0 interface.
 */
__STATIC_INLINE void CRU_QSPI0_XIP_Enable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP0EN);
}

/**
 * @brief Disables XIP mode on QSPI0 interface.
 */
__STATIC_INLINE void CRU_QSPI0_XIP_Disable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP0EN);
}

/**
 * @brief Checks if XIP mode on QSPI0 interface is enabled.
 * @returns 1 if XIP mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_QSPI0_XIP_IsEnabled(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP0EN) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables XIP mode on QSPI1 interface.
 */
__STATIC_INLINE void CRU_QSPI1_XIP_Enable(void)
{
    SET_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP1EN);
}

/**
 * @brief Disables XIP mode on QSPI1 interface.
 */
__STATIC_INLINE void CRU_QSPI1_XIP_Disable(void)
{
    CLEAR_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP1EN);
}

/**
 * @brief Checks if XIP mode on QSPI1 interface is enabled.
 * @returns 1 if XIP mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_QSPI1_XIP_IsEnabled(void)
{
    return ((READ_BIT(CRU->SYSCR0, CRU_SYSCR0_XIP1EN) != 0UL) ? 1UL : 0UL);
}

/******************************************************************************/
/*                                   Power                                    */
/******************************************************************************/

/**
 * @brief Configures core LDO output voltage level.
 * @param Level The voltage level. Can be one of CRU_LDOC_Adj_TypeDef values.
 */
__STATIC_INLINE void CRU_SetLDOCAdjLevel(CRU_LDOC_Adj_TypeDef Level)
{
    MODIFY_REG(CRU->LDOCR, CRU_LDOCR_LDOC_ADJ, (uint32_t)Level);
}

/**
 * @brief Returns core LDO output voltage level.
 * @returns The voltage level. Can be one of CRU_LDOC_Adj_TypeDef values.
 */
__STATIC_INLINE CRU_LDOC_Adj_TypeDef CRU_GetLDOCAdjLevel(void)
{
    return (CRU_LDOC_Adj_TypeDef)(READ_BIT(CRU->LDOCR, CRU_LDOCR_LDOC_ADJ));
}

/**
 * @brief Configures core LDO protection current level.
 * @param Level The protection current level. Can be one of CRU_LDOC_PI_TypeDef values.
 */
__STATIC_INLINE void CRU_SetLDOCPILevel(CRU_LDOC_PI_TypeDef Level)
{
    MODIFY_REG(CRU->LDOCR, CRU_LDOCR_LDOC_LDO_PI, (uint32_t)Level);
}

/**
 * @brief Returns core LDO protection current level.
 * @returns The protection current level. Can be one of CRU_LDOC_PI_TypeDef values.
 */
__STATIC_INLINE CRU_LDOC_PI_TypeDef CRU_GetLDOCPILevel(void)
{
    return (CRU_LDOC_PI_TypeDef)(READ_BIT(CRU->LDOCR, CRU_LDOCR_LDOC_LDO_PI));
}

/**
 * @brief Configures embedded flash LDO output voltage level.
 * @param Level The voltage level. Can be one of CRU_LDOF_Adj_TypeDef values.
 */
__STATIC_INLINE void CRU_SetLDOFAdjLevel(CRU_LDOF_Adj_TypeDef Level)
{
    MODIFY_REG(CRU->LDOCR, CRU_LDOCR_LDOF_ADJ, (uint32_t)Level);
}

/**
 * @brief Returns embedded flash LDO output voltage level.
 * @returns The voltage level. Can be one of CRU_LDOF_Adj_TypeDef values.
 */
__STATIC_INLINE CRU_LDOF_Adj_TypeDef CRU_GetLDOFAdjLevel(void)
{
    return (CRU_LDOF_Adj_TypeDef)(READ_BIT(CRU->LDOCR, CRU_LDOCR_LDOF_ADJ));
}

/**
 * @brief Configures embedded flash LDO protection current level.
 * @param Level The protection current level. Can be one of CRU_LDOF_PI_TypeDef values.
 */
__STATIC_INLINE void CRU_SetLDOFPILevel(CRU_LDOF_PI_TypeDef Level)
{
    MODIFY_REG(CRU->LDOCR, CRU_LDOCR_LDOF_LDO_PI, (uint32_t)Level);
}

/**
 * @brief Returns embedded flash LDO protection current level.
 * @returns The protection current level. Can be one of CRU_LDOF_PI_TypeDef values.
 */
__STATIC_INLINE CRU_LDOF_PI_TypeDef CRU_GetLDOFPILevel(void)
{
    return (CRU_LDOF_PI_TypeDef)(READ_BIT(CRU->LDOCR, CRU_LDOCR_LDOF_LDO_PI));
}

#if defined (BE_U1000)

/**
 * @brief Enables embedded flash LDO Power Down mode.
 */
__STATIC_INLINE void CRU_LDOF_PowerDown_Enable(void)
{
    SET_BIT(CRU->LDOCR, CRU_LDOCR_LDOF_PD);
}

/**
 * @brief Disables embedded flash LDO Power Down mode.
 */
__STATIC_INLINE void CRU_LDOF_PowerDown_Disable(void)
{
    CLEAR_BIT(CRU->LDOCR, CRU_LDOCR_LDOF_PD);
}

/**
 * @brief Checks if embedded flash LDO Power Down mode is enabled.
 * @returns 1 if Power Down mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_LDOF_PowerDown_IsEnabled(void)
{
    return ((READ_BIT(CRU->LDOCR, CRU_LDOCR_LDOF_PD) != 0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if NPOR_LDOF flag is in active state.
 * @note NPOR_LDOF is in active state if EFLASH LDO power supply is ready.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_LDOF_IsActiveFlag_NPOR(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_NPOR_LDOF) != 0x0UL) ? 1UL : 0UL);
}

#endif

/******************************************************************************/
/*                                   Status                                   */
/******************************************************************************/

/**
 * @brief Checks if flag is in active state.
 * @param Flag The flag. Can be one of CRU_Flag_TypeDef values.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag(CRU_Flag_TypeDef Flag)
{
    return ((READ_BIT(CRU->SYSSR0, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

#if defined (BMCU_U)

/**
 * @brief Checks if SECUREDIS flag is in active state.
 * @note If SECUREDIS is in active state, boot clock is selected with strap input.
 *       Otherwise boot clock is taken from RC generator.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_SECUREDIS(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_SECUREDIS) != 0x0UL) ? 1UL : 0UL);
}

#endif

#if defined (BE_U1000)

/**
 * @brief Checks if NOSECURE flag is in active state.
 * @note If NOSECURE is in active state, boot clock is selected with strap input.
 *       Otherwise boot clock is taken from RC generator.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_NOSECURE(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_NOSECURE) != 0x0UL) ? 1UL : 0UL);
}

#endif

/**
 * @brief Checks if EFLASH_OK flag is in active state.
 * @note EFLASH_OK is in active state if EFLASH is good.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_EFLASH_OK(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_EFLASH_OK) != 0x0UL) ? 1UL : 0UL);
}

#if defined (BMCU_U)

/**
 * @brief Checks if PACK flag is in active state.
 * @note PACK=0: 32 pin, PACK=1: 48 pin.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_PACK(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_PACK) != 0x0UL) ? 1UL : 0UL);
}

#endif

#if defined (BE_U1000)

/**
 * @brief Checks if POR_OK flag is in active state.
 * @note POR_OK is in active state if PoR circuit is good.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_POR_OK(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_POR_OK) != 0x0UL) ? 1UL : 0UL);
}

#endif

/**
 * @brief Checks if WDTHIT flag is in active state.
 * @note WDTHIT is in active state if a watchdog timeout has occured.
 * @retval 1 if the flag is in active state, otherwise 0.
 */
__STATIC_INLINE uint32_t CRU_IsActiveFlag_WDTHIT(void)
{
    return ((READ_BIT(CRU->SYSSR0, CRU_SYSSR0_WDTHIT) != 0x0UL) ? 1UL : 0UL);
}

ErrorStatus CRU_DeInit(void);
void CRU_GetSystemClocksFreq(CRU_Clocks_TypeDef * CRU_Clocks);

ErrorStatus CRU_PIN_Init(CRU_PIN_InitStruct_TypeDef *CRU_PIN_InitStruct);
void CRU_PIN_StructInit(CRU_PIN_InitStruct_TypeDef *CRU_PIN_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_CRU_H */

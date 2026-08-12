/**
  ******************************************************************************
  * @file    MDR32FxQI_port.c
  * @author  Milandr Application Team
  * @version V2.2.0i
  * @date    19/11/2024
  * @brief   This file contains all the PORT firmware functions.
  ******************************************************************************
  * <br><br>
  *
  * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
  * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_port.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup PORT PORT
  * @{
  */

/** @defgroup PORT_Private_Defines PORT Private Defines
  * @{
  */

#define IS_PORT_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_PORTA) || \
                                    ((PERIPH) == MDR_PORTB) || \
                                    ((PERIPH) == MDR_PORTC) || \
                                    ((PERIPH) == MDR_PORTD) || \
                                    ((PERIPH) == MDR_PORTE) || \
                                    ((PERIPH) == MDR_PORTF))

/** @} */ /* End of group PORT_Private_Defines */

/** @defgroup PORT_Exported_Functions PORT Exported Functions
  * @{
  */

/**
  * @brief  Resets the MDR_PORTx peripheral registers to their default reset values.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @retval None
  */
void PORT_DeInit(MDR_PORT_TypeDef* MDR_PORTx)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));

    MDR_PORTx->OE     = 0;
    MDR_PORTx->FUNC   = 0;
    MDR_PORTx->ANALOG = 0;
    MDR_PORTx->PWR    = 0;
    MDR_PORTx->PD     = 0;
    MDR_PORTx->RXTX   = 0;
    MDR_PORTx->PULL   = 0;
    MDR_PORTx->GFEN   = 0;
}


/**
  * @brief  Initializes the MDR_PORTx peripheral according to the specified
  *         parameters in the PORT_InitStruct.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PORT_InitStruct: pointer to a @ref PORT_InitTypeDef structure that
  *         contains the configuration information for the specified PORT peripheral.
  * @retval None
  */
void PORT_Init(MDR_PORT_TypeDef* MDR_PORTx, const PORT_InitTypeDef* PORT_InitStruct)
{
    uint32_t tmpreg_OE;
    uint32_t tmpreg_FUNC;
    uint32_t tmpreg_ANALOG;
    uint32_t tmpreg_PULL;
    uint32_t tmpreg_PD;
    uint32_t tmpreg_PWR;
    uint32_t tmpreg_GFEN;
    uint32_t portpin, pos, mask_s, mask_l, mask_d, mask_func;

    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_PORT_PIN(PORT_InitStruct->PORT_Pin));
    assert_param(IS_PORT_OE(PORT_InitStruct->PORT_OE));
    assert_param(IS_PORT_PULL_UP(PORT_InitStruct->PORT_PULL_UP));
    assert_param(IS_PORT_PULL_DOWN(PORT_InitStruct->PORT_PULL_DOWN));
    assert_param(IS_PORT_PD_SHM(PORT_InitStruct->PORT_PD_SHM));
    assert_param(IS_PORT_PD(PORT_InitStruct->PORT_PD));
    assert_param(IS_PORT_GFEN(PORT_InitStruct->PORT_GFEN));
    assert_param(IS_PORT_FUNC(PORT_InitStruct->PORT_FUNC));
    assert_param(IS_PORT_SPEED(PORT_InitStruct->PORT_SPEED));
    assert_param(IS_PORT_MODE(PORT_InitStruct->PORT_MODE));

    /* Get current PORT register values */
    tmpreg_OE     = MDR_PORTx->OE;
    tmpreg_FUNC   = MDR_PORTx->FUNC;
    tmpreg_ANALOG = MDR_PORTx->ANALOG;
    tmpreg_PULL   = MDR_PORTx->PULL;
    tmpreg_PD     = MDR_PORTx->PD;
    tmpreg_PWR    = MDR_PORTx->PWR;
    tmpreg_GFEN   = MDR_PORTx->GFEN;

    /* Form new values */
    pos = 0;
    mask_s = 0x0001;
    mask_l = 0x00000003;
    mask_d = 0x00010001;
    mask_func = 0;

    for (portpin = PORT_InitStruct->PORT_Pin; portpin; portpin >>= 1)
    {
        if (portpin & 0x1)
        {
            tmpreg_OE     = (tmpreg_OE     & ~mask_s) | (PORT_InitStruct->PORT_OE        <<  pos);
            tmpreg_FUNC   = (tmpreg_FUNC   & ~mask_l) | (PORT_InitStruct->PORT_FUNC      << (pos * 2));
            tmpreg_ANALOG = (tmpreg_ANALOG & ~mask_s) | (PORT_InitStruct->PORT_MODE      <<  pos);
            tmpreg_PULL   = (tmpreg_PULL   & ~mask_d) | (PORT_InitStruct->PORT_PULL_UP   << (pos + 16))
                                                      | (PORT_InitStruct->PORT_PULL_DOWN <<  pos);
            tmpreg_PD     = (tmpreg_PD     & ~mask_d) | (PORT_InitStruct->PORT_PD_SHM    << (pos + 16))
                                                      | (PORT_InitStruct->PORT_PD        <<  pos);
            tmpreg_PWR    = (tmpreg_PWR    & ~mask_l) | (PORT_InitStruct->PORT_SPEED     << (pos * 2));
            tmpreg_GFEN   = (tmpreg_GFEN   & ~mask_s) | (PORT_InitStruct->PORT_GFEN      <<  pos);
            mask_func |= mask_l;
        }
        mask_s <<= 1;
        mask_l <<= 2;
        mask_d <<= 1;
        pos++;
    }

    portpin = (PORT_InitStruct->PORT_Pin);

#if (defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)) && (defined (USE_JTAG_A) || defined (USE_JTAG_B))
    /* JTAG pins protection. */
    portpin   |= JTAG_PINS(MDR_PORTx);
    mask_func |= JTAG_PINS2(MDR_PORTx);

    tmpreg_OE     &= ~JTAG_PINS(MDR_PORTx);
    tmpreg_ANALOG &= ~JTAG_PINS(MDR_PORTx);
    tmpreg_FUNC   &= ~JTAG_PINS2(MDR_PORTx);
    tmpreg_PULL   &= ~JTAG_PINS3(MDR_PORTx);
    tmpreg_PD     &= ~JTAG_PINS3(MDR_PORTx);
    tmpreg_PWR    &= ~JTAG_PINS2(MDR_PORTx);
    tmpreg_GFEN   &= ~JTAG_PINS(MDR_PORTx);
#endif

    /* Partial deinitialization for all pins which will be configured. */
    MDR_PORTx->OE     &= ~portpin;
    MDR_PORTx->FUNC   &= ~mask_func;
    MDR_PORTx->ANALOG &= ~portpin;

    /* Configure PORT registers with new values. */
    MDR_PORTx->PULL   = tmpreg_PULL;
    MDR_PORTx->PWR    = tmpreg_PWR;
    MDR_PORTx->PD     = tmpreg_PD;
    MDR_PORTx->GFEN   = tmpreg_GFEN;
    MDR_PORTx->ANALOG = tmpreg_ANALOG;
    MDR_PORTx->FUNC   = tmpreg_FUNC;
    MDR_PORTx->OE     = tmpreg_OE;
}

/**
  * @brief  Fills each PORT_InitStruct member with its default value.
  * @param  PORT_InitStruct: pointer to a @ref PORT_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void PORT_StructInit(PORT_InitTypeDef* PORT_InitStruct)
{
    /* Reset PORT initialization structure parameters values */
    PORT_InitStruct->PORT_Pin       = PORT_Pin_All;
    PORT_InitStruct->PORT_OE        = PORT_OE_IN;
    PORT_InitStruct->PORT_PULL_UP   = PORT_PULL_UP_OFF;
    PORT_InitStruct->PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
    PORT_InitStruct->PORT_PD_SHM    = PORT_PD_SHM_OFF;
    PORT_InitStruct->PORT_PD        = PORT_PD_DRIVER;
    PORT_InitStruct->PORT_GFEN      = PORT_GFEN_OFF;
    PORT_InitStruct->PORT_FUNC      = PORT_FUNC_PORT;
    PORT_InitStruct->PORT_SPEED     = PORT_OUTPUT_OFF;
    PORT_InitStruct->PORT_MODE      = PORT_MODE_ANALOG;
}

/**
  * @brief  Reads the specified input port pin.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PORT_Pin - @ref PORT_Pin_TypeDef - specifies the port bit to read.
  *         This parameter can be PORT_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
uint8_t PORT_ReadInputDataBit(MDR_PORT_TypeDef* MDR_PORTx, PORT_Pin_TypeDef PORT_Pin)
{
    uint8_t bitstatus;

    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_GET_PORT_PIN(PORT_Pin));

    if ((MDR_PORTx->RXTX & (uint32_t)PORT_Pin) != RESET)
    {
        bitstatus = (uint8_t)SET;
    }
    else
    {
        bitstatus = (uint8_t)RESET;
    }

    return bitstatus;
}

/**
  * @brief  Reads the specified PORT input data port.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @retval PORT input data port value. Bits (16..31) are always 0.
  */
uint32_t PORT_ReadInputData(MDR_PORT_TypeDef* MDR_PORTx)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));

    return (MDR_PORTx->RXTX);
}

/**
  * @brief  Sets the selected data port bits.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PORT_Pin - @ref PORT_Pin_TypeDef - specifies the port bits to be written.
  *         This parameter can be any combination of PORT_Pin_x where x can be (0..15).
  * @retval None
  */
void PORT_SetBits(MDR_PORT_TypeDef* MDR_PORTx, uint32_t PORT_Pin)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_PORT_PIN(PORT_Pin));
    assert_param(IS_NOT_JTAG_PIN(MDR_PORTx, PORT_Pin));

#if defined (USE_K1986VE9xI)
    MDR_PORTx->RXTX = (PORT_Pin | MDR_PORTx->RXTX) & (~JTAG_PINS(MDR_PORTx));
#elif defined (USE_MDR32FG16S1QI)
    MDR_PORTx->SETTX = (PORT_Pin & ~JTAG_PINS(MDR_PORTx));
#elif defined (USE_K1986VE1xI)
    MDR_PORTx->SETTX = PORT_Pin;
#endif
}

/**
  * @brief  Clears the selected data port bits.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PORT_Pin - @ref PORT_Pin_TypeDef - specifies the port bits to be written.
  *         This parameter can be any combination of PORT_Pin_x where x can be (0..15).
  * @retval None
  */
void PORT_ResetBits(MDR_PORT_TypeDef* MDR_PORTx, uint32_t PORT_Pin)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_PORT_PIN(PORT_Pin));
    assert_param(IS_NOT_JTAG_PIN(MDR_PORTx, PORT_Pin));

#if defined (USE_K1986VE9xI)
    MDR_PORTx->RXTX &= ~(PORT_Pin | JTAG_PINS(MDR_PORTx));
#elif defined (USE_MDR32FG16S1QI)
    MDR_PORTx->CLRTX = (PORT_Pin & ~JTAG_PINS(MDR_PORTx));
#elif defined (USE_K1986VE1xI)
    MDR_PORTx->CLRTX = PORT_Pin;
#endif
}

/**
  * @brief  Sets or clears the selected data port bit.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PORT_Pin - @ref PORT_Pin_TypeDef - specifies the port bit to be written.
  *         This parameter can be one of PORT_Pin_x where x can be (0..15).
  * @param  BitVal: specifies the value to be written to the selected bit.
  *         This parameter can be one of the @ref BitStatus enum values:
  *             @arg RESET: to clear the port pin
  *             @arg SET: to set the port pin
  * @retval None
  */
void PORT_WriteBit(MDR_PORT_TypeDef* MDR_PORTx, uint32_t PORT_Pin, BitStatus BitVal)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_GET_PORT_PIN(PORT_Pin));
    assert_param(IS_BIT_STATUS(BitVal));
    assert_param(IS_NOT_JTAG_PIN(MDR_PORTx, PORT_Pin));

    if (BitVal != 0)
    {
#if defined (USE_K1986VE9xI)
        MDR_PORTx->RXTX = (PORT_Pin | MDR_PORTx->RXTX) & (~JTAG_PINS(MDR_PORTx));
#elif defined (USE_MDR32FG16S1QI)
        MDR_PORTx->SETTX = (PORT_Pin & ~JTAG_PINS(MDR_PORTx));
#elif defined (USE_K1986VE1xI)
        MDR_PORTx->SETTX = PORT_Pin;
#endif
    }
    else
    {
#if defined (USE_K1986VE9xI)
        MDR_PORTx->RXTX &= ~(PORT_Pin | JTAG_PINS(MDR_PORTx));
#elif defined (USE_MDR32FG16S1QI)
        MDR_PORTx->CLRTX = (PORT_Pin & ~JTAG_PINS(MDR_PORTx));
#elif defined (USE_K1986VE1xI)
        MDR_PORTx->CLRTX = PORT_Pin;
#endif
    }
}

/**
  * @brief  Writes data to the specified PORT data port.
  * @param  MDR_PORTx: Select MDR_PORTx peripheral. x can be (A..F).
  * @param  PortVal: specifies the value to be written to the port output data register.
  *         Bits (16..31) are ignored.
  * @retval None
  */
void PORT_Write(MDR_PORT_TypeDef* MDR_PORTx, uint32_t PortVal)
{
    /* Check the parameters */
    assert_param(IS_PORT_ALL_PERIPH(MDR_PORTx));
    assert_param(IS_NOT_JTAG_PIN(MDR_PORTx, PortVal));

    MDR_PORTx->RXTX = PortVal & (~JTAG_PINS(MDR_PORTx));
}

/** @} */ /* End of group PORT_Exported_Functions */

/** @} */ /* End of group PORT */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_port.c */



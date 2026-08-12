/**
 * *****************************************************************************
 *  @file       BSP/Components/tusb320/tusb320.c
 *  @author     Baikal electronics SDK team
 *  @brief      TUSB320 driver source file
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

#include "tusb320.h"
#include "bmcu_common.h"
#include "bmcu_cru.h"
#include "bmcu_gpio.h"
#include "i2c2.h"
#include <string.h>

#if defined(EVU_BA_2_5)
#include "../../EVU_BA_2_5/bsp.h"
#else
#error "The selected development board does not support TUSB320"
#endif

/**
 * @brief TUSB320 I2C address.
 * @note The address depends on the state of ADDR pin:
 *       - Low: I2C is enabled and I2C 7-bit address is 0x60.
 *       - High: I2C is enabled and I2C 7-bit address is 0x61.
 *       - Not Connected: GPIO mode (I2C is disabled).
 */
#define TUSB320_I2C_ADDRESS (0x60U)

/**
 * @brief TUSB320 I2C device ID.
 */
const uint8_t TUSB320_ID[8] = {0x30U, 0x32U, 0x33U, 0x42U, 0x53U, 0x55U, 0x54U, 0x00U};

/**
 * @brief Initialize TUSB320 device.
 */
void TUSB320_Init(void) {
    GPIO_InitStruct_TypeDef GPIO_InitStruct;
    CRU_PIN_InitStruct_TypeDef CRU_PIN_InitStruct;

    /* Initialize I2C interface */
    bsp_i2c2_init(TUSB320_I2C_ADDRESS);

    /* Initialize interrupt pin */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.PinMask = BSP_TUSB320_INT_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_Init(BSP_TUSB320_INT_GPIO_PORT, &GPIO_InitStruct);

    CRU_PIN_StructInit(&CRU_PIN_InitStruct);
    CRU_PIN_InitStruct.Port = BSP_TUSB320_INT_CRU_PORT;
    CRU_PIN_InitStruct.Pin = BSP_TUSB320_INT_CRU_PIN;
    CRU_PIN_InitStruct.Pull = CRU_PIN_PULL_NO;
    CRU_PIN_Init(&CRU_PIN_InitStruct);

    CRU_C0_C1_EXTI_SetSource(BSP_TUSB320_INT_CRU_PORT, BSP_TUSB320_INT_CRU_PIN);

    CLIC_ConfigIRQ(BSP_TUSB320_INT_CLIC_IRQN,   /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,   /* Privilege mode */
                   1U,                          /* Level */
                   1U,                          /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,   /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_EDGE, /* Type */
                   CLIC_INTATTR_TRIG_POL_N);    /* Polarity */

    /* Wait for T_I2C_EN = 100ms after the device power up */
    __delay_ms(100UL);
}

/**
 * @brief Enable TUSB320 interrupt.
 */
void TUSB320_IntEnable(void) {
    CLIC_EnableIRQ(BSP_TUSB320_INT_CLIC_IRQN);
}

/**
 * @brief Disable TUSB320 interrupt.
 */
void TUSB320_IntDisable(void) {
    CLIC_DisableIRQ(BSP_TUSB320_INT_CLIC_IRQN);
}

/**
 * @brief Write TUSB320 register.
 * @param RegAddr The register address.
 * @param RegVal The register value.
 * @returns The operation status.
 */
ErrorStatus TUSB320_WriteReg(uint8_t RegAddr, uint8_t RegVal) {
    ErrorStatus Status = SUCCESS;

    uint8_t Buffer[] = {RegAddr, RegVal};
    const uint32_t Datalen = sizeof(Buffer) / sizeof(*Buffer);

    if (bsp_i2c2_write(Buffer, Datalen) != Datalen) {
        Status = ERROR;
    }

    return Status;
}

/**
 * @brief Read TUSB320 register.
 * @param RegAddr The register address.
 * @param RegVal The buffer for the register value.
 * @returns The operation status.
 */
ErrorStatus TUSB320_ReadReg(uint8_t RegAddr, uint8_t *RegVal) {
    ErrorStatus Status = SUCCESS;

    const uint32_t Datalen = sizeof(RegAddr);

    /* Set register address */
    if (bsp_i2c2_write(&RegAddr, Datalen) != Datalen) {
        Status = ERROR;
    }

    /* Read register value */
    if (Status == SUCCESS) {
        /* Flush FIFO */
        bsp_i2c2_flush();

        /* Read register value */
        if (bsp_i2c2_read(RegVal, Datalen) != Datalen) {
            Status = ERROR;
        }
    }

    return Status;
}

/**
 * @brief Get TUSB320 device ID.
 * @param ID The buffer for the ID. Its size must be 8 bytes.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetDeviceID(uint8_t *ID) {
    ErrorStatus Status = SUCCESS;

    uint8_t Addr = TUSB320_CSR00_DEVICE_ID;
    uint32_t Datalen = sizeof(Addr);

    /* Set register address */
    if (bsp_i2c2_write(&Addr, Datalen) != Datalen) {
        Status = ERROR;
    }

    if (Status == SUCCESS) {
        /* Flush */
        bsp_i2c2_flush();

        /* Read device ID */
        Datalen = sizeof(TUSB320_ID);
        if (bsp_i2c2_read(ID, Datalen) != Datalen) {
            Status = ERROR;
        }
    }

    return Status;
}

/**
 * @brief Check if the TUSB320 device ID is correct.
 * @param ID The buffer for the ID. Its size must be 8 bytes.
 * @returns The operation status.
 */
ErrorStatus TUSB320_CheckDeviceID(uint8_t *ID) {
    return ((memcmp(ID, TUSB320_ID, sizeof(TUSB320_ID)) == 0) ? SUCCESS : ERROR);
}

/**
 * @brief Get active cable detection state.
 * @param State The pointer to the active cable detection state.
 *              Can be one of TUSB320_ActiveCable_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetActiveCableDetectionState(TUSB320_ActiveCable_TypeDef *State) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR08, &RegVal);

    if (Status == SUCCESS) {
        *State = (TUSB320_ActiveCable_TypeDef)(RegVal & TUSB320_CSR08_ACTIVE_CABLE_DETECTION);
    }

    return Status;
}

/**
 * @brief Get accessory connection state.
 * @param State The pointer to the accessory connection state.
 *              Can be one of TUSB320_AccessoryConnected_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetAccessoryConnectionState(TUSB320_AccessoryConnected_TypeDef *State) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR08, &RegVal);

    if (Status == SUCCESS) {
        *State = (TUSB320_AccessoryConnected_TypeDef)(RegVal & TUSB320_CSR08_ACCESSORY_CONNECTED);
    }

    return Status;
}

/**
 * @brief Get the detected current mode advertised by the attached DFP.
 * @param Mode The pointer to the detected current mode.
 *             Can be one of TUSB320_CurrentModeDetect_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetDetectedCurrentMode(TUSB320_CurrentModeDetect_TypeDef *Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR08, &RegVal);

    if (Status == SUCCESS) {
        *Mode = (TUSB320_CurrentModeDetect_TypeDef)(RegVal & TUSB320_CSR08_CURRENT_MODE_DETECT);
    }

    return Status;
}

/**
 * @brief Set the advertised current mode.
 * @param Mode The advertised current mode.
 *             Can be one of TUSB320_CurrentModeAdvertise_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SetAdvertisedCurrentMode(TUSB320_CurrentModeAdvertise_TypeDef Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR08, &RegVal);

    if (Status == SUCCESS) {
        MODIFY_REG(RegVal, TUSB320_CSR08_CURRENT_MODE_ADVERTISE, (uint8_t)Mode & TUSB320_CSR08_CURRENT_MODE_ADVERTISE);

        Status = TUSB320_WriteReg(TUSB320_CSR08, RegVal);
    }

    return Status;
}

/**
 * @brief Get the advertised current mode.
 * @param Mode The pointer to the advertised current mode.
 *             Can be one of TUSB320_CurrentModeAdvertise_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetAdvertisedCurrentMode(TUSB320_CurrentModeAdvertise_TypeDef *Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR08, &RegVal);

    if (Status == SUCCESS) {
        *Mode = (TUSB320_CurrentModeAdvertise_TypeDef)(RegVal & TUSB320_CSR08_CURRENT_MODE_ADVERTISE);
    }

    return Status;
}

/**
 * @brief Set the DRP advertisement duty cycle.
 * @param DutyCycle The DRP advertisement duty cycle.
 *                  Can be one of TUSB320_DRPDutyCycle_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SetDRPDutyCycle(TUSB320_DRPDutyCycle_TypeDef DutyCycle) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        MODIFY_REG(RegVal, TUSB320_CSR09_INTERRUPT_STATUS | TUSB320_CSR09_DRP_DUTY_CYCLE,
                   (uint8_t)DutyCycle & TUSB320_CSR09_DRP_DUTY_CYCLE);

        Status = TUSB320_WriteReg(TUSB320_CSR09, RegVal);
    }

    return Status;
}

/**
 * @brief Get the DRP advertisement duty cycle.
 * @param DutyCycle The pointer to the DRP advertisement duty cycle.
 *                  Can be one of TUSB320_DRPDutyCycle_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetDRPDutyCycle(TUSB320_DRPDutyCycle_TypeDef *DutyCycle) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        *DutyCycle = (TUSB320_DRPDutyCycle_TypeDef)(RegVal & TUSB320_CSR09_DRP_DUTY_CYCLE);
    }

    return Status;
}

/**
 * @brief Get the cable direction.
 * @param Direction The pointer to the cable direction.
 *                  Can be one of TUSB320_CableDir_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetCableDirection(TUSB320_CableDir_TypeDef *Direction) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        *Direction = (TUSB320_CableDir_TypeDef)(RegVal & TUSB320_CSR09_CABLE_DIR);
    }

    return Status;
}

/**
 * @brief Get the attachment state.
 * @note It defines the attachment state of Type C controller itself.
 * @param State The pointer to the attachment state.
 *              Can be one of TUSB320_AttachedState_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetAttachmentState(TUSB320_AttachedState_TypeDef *State) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        *State = (TUSB320_AttachedState_TypeDef)(RegVal & TUSB320_CSR09_ATTACHED_STATE);
    }

    return Status;
}

/**
 * @brief Set the operation mode.
 * @param Mode The operation mode.
 *             Can be one of TUSB320_ModeSelect_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SetMode(TUSB320_ModeSelect_TypeDef Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR0A, &RegVal);

    if (Status == SUCCESS) {
        MODIFY_REG(RegVal, TUSB320_CSR0A_MODE_SELECT | TUSB320_CSR0A_I2C_SOFT_RESET,
                   (uint8_t)Mode & TUSB320_CSR0A_MODE_SELECT);

        Status = TUSB320_WriteReg(TUSB320_CSR0A, RegVal);
    }

    return Status;
}

/**
 * @brief Get the operation mode.
 * @param Mode The pointer to the operation mode.
 *             Can be one of TUSB320_ModeSelect_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetMode(TUSB320_ModeSelect_TypeDef *Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR0A, &RegVal);

    if (Status == SUCCESS) {
        *Mode = (TUSB320_ModeSelect_TypeDef)(RegVal & TUSB320_CSR0A_MODE_SELECT);
    }

    return Status;
}

/**
 * @brief Set the CC pins voltage debounce time.
 * @param Debounce The debounce time.
 *                 Can be one of TUSB320_Debounce_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SetDebounceTime(TUSB320_Debounce_TypeDef Debounce) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR0A, &RegVal);

    if (Status == SUCCESS) {
        MODIFY_REG(RegVal, TUSB320_CSR0A_DEBOUNCE | TUSB320_CSR0A_I2C_SOFT_RESET,
                   (uint8_t)Debounce & TUSB320_CSR0A_DEBOUNCE);

        Status = TUSB320_WriteReg(TUSB320_CSR0A, RegVal);
    }

    return Status;
}

/**
 * @brief Get the CC pins voltage debounce time.
 * @param Debounce The pointer to the debounce time.
 *                 Can be one of TUSB320_Debounce_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetDebounceTime(TUSB320_Debounce_TypeDef *Debounce) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR0A, &RegVal);

    if (Status == SUCCESS) {
        *Debounce = (TUSB320_Debounce_TypeDef)(RegVal & TUSB320_CSR0A_DEBOUNCE);
    }

    return Status;
}

/**
 * @brief Set the Rd and Rp operation mode.
 * @param Mode The Rd and Rp operation mode.
 *             Can be one of TUSB320_RdRp_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SetRdRpMode(TUSB320_RdRp_TypeDef Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR45, &RegVal);

    if (Status == SUCCESS) {
        MODIFY_REG(RegVal, TUSB320_CSR45_DISABLE_RD_RP, (uint8_t)Mode & TUSB320_CSR45_DISABLE_RD_RP);

        Status = TUSB320_WriteReg(TUSB320_CSR45, RegVal);
    }

    return Status;
}

/**
 * @brief Get the Rd and Rp operation mode.
 * @param Mode The pointer to the Rd and Rp operation mode.
 *             Can be one of TUSB320_RdRp_TypeDef values.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetRdRpMode(TUSB320_RdRp_TypeDef *Mode) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR45, &RegVal);

    if (Status == SUCCESS) {
        *Mode = (TUSB320_RdRp_TypeDef)(RegVal & TUSB320_CSR45_DISABLE_RD_RP);
    }

    return Status;
}

/**
 * @brief Perform soft reset.
 * @note The maximal duration of soft reset is 95ms.
 * @returns The operation status.
 */
ErrorStatus TUSB320_SoftReset(void) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR0A, &RegVal);

    if (Status == SUCCESS) {
        SET_BIT(RegVal, TUSB320_CSR0A_I2C_SOFT_RESET);

        Status = TUSB320_WriteReg(TUSB320_CSR0A, RegVal);
    }

    return Status;
}

/**
 * @brief Get interrupt state.
 * @param IT The pointer to the interrupt state.
 * @returns The operation status.
 */
ErrorStatus TUSB320_GetITState(ITStatus *IT) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        *IT = ((RegVal & TUSB320_CSR09_INTERRUPT_STATUS) != 0U) ? SET : RESET;
    }

    return Status;
}

/**
 * @brief Clear interrupt.
 * @returns The operation status.
 */
ErrorStatus TUSB320_ClearIT(void) {
    uint8_t RegVal;
    uint32_t Status = TUSB320_ReadReg(TUSB320_CSR09, &RegVal);

    if (Status == SUCCESS) {
        SET_BIT(RegVal, TUSB320_CSR09_INTERRUPT_STATUS);

        Status = TUSB320_WriteReg(TUSB320_CSR09, RegVal);
    }

    return Status;
}

/**
 * @brief Interrupt handler callback.
 *        Invoked if no strong implementation is available.
 */
__WEAK void TUSB320_IRQHandler(void) {}

/**
 * @brief TUSB320 interrupt handler.
 */
__attribute__((interrupt)) void BSP_TUSB320_INT_ISR(void) {
    TUSB320_IRQHandler();
}

/**
 * *****************************************************************************
 *  @file       BSP/Components/tusb320/tusb320.h
 *  @author     Baikal electronics SDK team
 *  @brief      TUSB320 driver header file
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

#ifndef __TUSB320_H
#define __TUSB320_H

#include "bmcu_def.h"

/* TUSB320 CSR register definitions */
#define TUSB320_CSR00_DEVICE_ID (0x00U) /*!< Device ID (Registers 0x00 - 0x07) */
#define TUSB320_CSR08 (0x08U)           /*!< Register 0x08 */
#define TUSB320_CSR09 (0x09U)           /*!< Register 0x09 */
#define TUSB320_CSR0A (0x0AU)           /*!< Register 0x0A */
#define TUSB320_CSR45 (0x45U)           /*!< Register 0x45 */

/* CSR register 0x08 bit definitions */

/** @brief CSR08[0]: An active cable has been plugged */
#define TUSB320_CSR08_ACTIVE_CABLE_DETECTION (0x01U)

/** @brief CSR08[3:1]: An accessory was attached */
#define TUSB320_CSR08_ACCESSORY_CONNECTED (0x0EU)

#define TUSB320_CSR08_ACCESSORY_CONNECTED_NONE (0x00U)  /*!< No accessory attached (default) */
#define TUSB320_CSR08_ACCESSORY_CONNECTED_AUDIO (0x08U) /*!< Audio accessory */
#define TUSB320_CSR08_ACCESSORY_CONNECTED_ACHRG (0x0AU) /*!< Audio charged thru accessory */
#define TUSB320_CSR08_ACCESSORY_CONNECTED_DEBUG (0x0CU) /*!< Debug accessory */

/** @brief CSR08[5:4]: Current mode */
#define TUSB320_CSR08_CURRENT_MODE_DETECT (0x30U)

#define TUSB320_CSR08_CURRENT_MODE_DETECT_DEFAULT (0x00U) /*!< Default (value at start up) */
#define TUSB320_CSR08_CURRENT_MODE_DETECT_MEDIUM (0x10U)  /*!< Medium */
#define TUSB320_CSR08_CURRENT_MODE_DETECT_ACC (0x20U)     /*!< Charge through accessory - 500mA */
#define TUSB320_CSR08_CURRENT_MODE_DETECT_HIGH (0x30U)    /*!< High */

/** @brief CSR08[7:6]: Current advertisement */
#define TUSB320_CSR08_CURRENT_MODE_ADVERTISE (0xC0U)

#define TUSB320_CSR08_CURRENT_MODE_ADVERTISE_DEFAULT (0x00U) /*!< Default (500mA / 900mA) initial value at startup */
#define TUSB320_CSR08_CURRENT_MODE_ADVERTISE_MEDIUM (0x40U)  /*!< Medium (1.5A) */
#define TUSB320_CSR08_CURRENT_MODE_ADVERTISE_HIGH (0x80U)    /*!< High (3A) */

/* CSR register 0x09 bit definitions */

/** @brief CSR09[2:1]: Percentage of time that a DRP advertises DFP during tDRP */
#define TUSB320_CSR09_DRP_DUTY_CYCLE (0x06U)

#define TUSB320_CSR09_DRP_DUTY_CYCLE_30 (0x00U) /*!< 30% (default) */
#define TUSB320_CSR09_DRP_DUTY_CYCLE_40 (0x02U) /*!< 40% */
#define TUSB320_CSR09_DRP_DUTY_CYCLE_50 (0x04U) /*!< 50% */
#define TUSB320_CSR09_DRP_DUTY_CYCLE_60 (0x06U) /*!< 60% */

/** @brief CSR09[4]: CSR changed interrupt status */
#define TUSB320_CSR09_INTERRUPT_STATUS (0x10U)

#define TUSB320_CSR09_INTERRUPT_STATUS_NOT_ACTIVE (0x00U) /*!< Interrupt is not active (CSR hasn't changed) */
#define TUSB320_CSR09_INTERRUPT_STATUS_ACTIVE (0x10U)     /*!< Interrupt is active (CSR has changed) */

/** @brief CSR09[5]: Cable orientation */
#define TUSB320_CSR09_CABLE_DIR (0x20U)

#define TUSB320_CSR09_CABLE_DIR_CC1 (0x00U) /*!< CC1 */
#define TUSB320_CSR09_CABLE_DIR_CC2 (0x20U) /*!< CC2 (default) */

/** @brief CSR09[7:6]: Attached state */
#define TUSB320_CSR09_ATTACHED_STATE (0xC0U)

#define TUSB320_CSR09_ATTACHED_STATE_NONE (0x00U) /*!< Not attached (default) */
#define TUSB320_CSR09_ATTACHED_STATE_DFP (0x40U)  /*!< Attached SRC (DFP) */
#define TUSB320_CSR09_ATTACHED_STATE_UFP (0x80U)  /*!< Attached SNK (UFP) */
#define TUSB320_CSR09_ATTACHED_STATE_ACC (0xC0U)  /*!< Attached to an accessory */

/* CSR register 0x0A bit definitions */

/** @brief CSR0A[3]: Reset digital logic */
#define TUSB320_CSR0A_I2C_SOFT_RESET (0x08U)

/** @brief CSR0A[5:4]: Operation mode select */
#define TUSB320_CSR0A_MODE_SELECT (0x30U)

#define TUSB320_CSR0A_MODE_SELECT_PORT (0x00U) /*!< Maintain mode according to PORT pin selection (default) */
#define TUSB320_CSR0A_MODE_SELECT_UFP (0x10U)  /*!< Unattached SNK (UFP) */
#define TUSB320_CSR0A_MODE_SELECT_DFP (0x20U)  /*!< Unattached SRC (DFP) */
#define TUSB320_CSR0A_MODE_SELECT_DRP (0x30U)  /*!< Start from unattached SNK (DRP) */

/** @brief CSR0A[7:6]: The nominal amount of time the TUSB320 device debounces the voltages on the CC pins */
#define TUSB320_CSR0A_DEBOUNCE (0xC0U)

#define TUSB320_CSR0A_DEBOUNCE_133MS (0x00U) /*!< 133 ms (default) */
#define TUSB320_CSR0A_DEBOUNCE_116MS (0x40U) /*!< 116 ms */
#define TUSB320_CSR0A_DEBOUNCE_151MS (0x80U) /*!< 151 ms */
#define TUSB320_CSR0A_DEBOUNCE_168MS (0xC0U) /*!< 168 ms */

/* CSR register 0x45 bit definitions */

/** @brief CSR45[2]: Disable Rd and Rp */
#define TUSB320_CSR45_DISABLE_RD_RP (0x04U)

/** @brief Active cable state */
typedef enum {
    TUSB320_ACTIVE_CABLE_NOT_DETECTED = 0x0U,                            /*!< An active cable is not detected */
    TUSB320_ACTIVE_CABLE_DETECTED = TUSB320_CSR08_ACTIVE_CABLE_DETECTION /*!< An active cable is detected */
} TUSB320_ActiveCable_TypeDef;

/** @brief Accessory connection state */
typedef enum {
    TUSB320_ACCESSORY_CONNECTED_NONE = TUSB320_CSR08_ACCESSORY_CONNECTED_NONE,   /*!< No accessory attached (default) */
    TUSB320_ACCESSORY_CONNECTED_AUDIO = TUSB320_CSR08_ACCESSORY_CONNECTED_AUDIO, /*!< Audio accessory */
    TUSB320_ACCESSORY_CONNECTED_ACHRG = TUSB320_CSR08_ACCESSORY_CONNECTED_ACHRG, /*!< Audio charged thru accessory */
    TUSB320_ACCESSORY_CONNECTED_DEBUG = TUSB320_CSR08_ACCESSORY_CONNECTED_DEBUG  /*!< Debug accessory */
} TUSB320_AccessoryConnected_TypeDef;

/** @brief Current mode detection */
typedef enum {
    TUSB320_CURRENT_MODE_DETECT_DEFAULT = TUSB320_CSR08_CURRENT_MODE_DETECT_DEFAULT, /*!< Default (value at start up) */
    TUSB320_CURRENT_MODE_DETECT_MEDIUM = TUSB320_CSR08_CURRENT_MODE_DETECT_MEDIUM,   /*!< Medium */
    TUSB320_CURRENT_MODE_DETECT_ACC = TUSB320_CSR08_CURRENT_MODE_DETECT_ACC,  /*!< Charge through accessory - 500mA */
    TUSB320_CURRENT_MODE_DETECT_HIGH = TUSB320_CSR08_CURRENT_MODE_DETECT_HIGH /*!< High */
} TUSB320_CurrentModeDetect_TypeDef;

/** @brief Current mode advertisement */
typedef enum {
    TUSB320_CURRENT_MODE_ADVERTISE_DEFAULT =
        TUSB320_CSR08_CURRENT_MODE_ADVERTISE_DEFAULT, /*!< Default (500mA / 900mA) initial value at startup */
    TUSB320_CURRENT_MODE_ADVERTISE_MEDIUM = TUSB320_CSR08_CURRENT_MODE_ADVERTISE_MEDIUM, /*!< Medium (1.5A) */
    TUSB320_CURRENT_MODE_ADVERTISE_HIGH = TUSB320_CSR08_CURRENT_MODE_ADVERTISE_HIGH      /*!< High (3A) */
} TUSB320_CurrentModeAdvertise_TypeDef;

/** @brief DRP advertisement duty cycle */
typedef enum {
    TUSB320_DRP_DUTY_CYCLE_30 = TUSB320_CSR09_DRP_DUTY_CYCLE_30, /*!< 30% (default) */
    TUSB320_DRP_DUTY_CYCLE_40 = TUSB320_CSR09_DRP_DUTY_CYCLE_40, /*!< 40% */
    TUSB320_DRP_DUTY_CYCLE_50 = TUSB320_CSR09_DRP_DUTY_CYCLE_50, /*!< 50% */
    TUSB320_DRP_DUTY_CYCLE_60 = TUSB320_CSR09_DRP_DUTY_CYCLE_60  /*!< 60% */
} TUSB320_DRPDutyCycle_TypeDef;

/** @brief Cable orientation */
typedef enum {
    TUSB320_CABLE_DIR_CC1 = TUSB320_CSR09_CABLE_DIR_CC1, /*!< CC1 */
    TUSB320_CABLE_DIR_CC2 = TUSB320_CSR09_CABLE_DIR_CC2  /*!< CC2 (default) */
} TUSB320_CableDir_TypeDef;

/** @brief Attachment state */
typedef enum {
    TUSB320_ATTACHED_STATE_NONE = TUSB320_CSR09_ATTACHED_STATE_NONE, /*!< Not attached (default) */
    TUSB320_ATTACHED_STATE_DFP = TUSB320_CSR09_ATTACHED_STATE_DFP,   /*!< Attached SRC (DFP) */
    TUSB320_ATTACHED_STATE_UFP = TUSB320_CSR09_ATTACHED_STATE_UFP,   /*!< Attached SNK (UFP) */
    TUSB320_ATTACHED_STATE_ACC = TUSB320_CSR09_ATTACHED_STATE_ACC    /*!< Attached to an accessory */
} TUSB320_AttachedState_TypeDef;

/** @brief Operation mode select */
typedef enum {
    TUSB320_MODE_SELECT_PORT =
        TUSB320_CSR0A_MODE_SELECT_PORT, /*!< Maintain mode according to PORT pin selection (default) */
    TUSB320_MODE_SELECT_UFP = TUSB320_CSR0A_MODE_SELECT_UFP, /*!< Unattached SNK (UFP) */
    TUSB320_MODE_SELECT_DFP = TUSB320_CSR0A_MODE_SELECT_DFP, /*!< Unattached SRC (DFP) */
    TUSB320_MODE_SELECT_DRP = TUSB320_CSR0A_MODE_SELECT_DRP  /*!< Start from unattached SNK (DRP) */
} TUSB320_ModeSelect_TypeDef;

/** @brief Debounce */
typedef enum {
    TUSB320_DEBOUNCE_133MS = TUSB320_CSR0A_DEBOUNCE_133MS, /*!< 133 ms (default) */
    TUSB320_DEBOUNCE_116MS = TUSB320_CSR0A_DEBOUNCE_116MS, /*!< 116 ms */
    TUSB320_DEBOUNCE_151MS = TUSB320_CSR0A_DEBOUNCE_151MS, /*!< 151 ms */
    TUSB320_DEBOUNCE_168MS = TUSB320_CSR0A_DEBOUNCE_168MS  /*!< 168 ms */
} TUSB320_Debounce_TypeDef;

/** @brief Rd and Rp mode */
typedef enum {
    TUSB320_RD_RP_NORMAL = 0x0U,                        /*!< Normal operation (default) */
    TUSB320_RD_RP_DISABLE = TUSB320_CSR45_DISABLE_RD_RP /*!< Disable Rd and Rp */
} TUSB320_RdRp_TypeDef;

/** @brief Initialize TUSB320 device. */
void TUSB320_Init(void);

/** @brief Enable TUSB320 interrupt. */
void TUSB320_IntEnable(void);

/** @brief Disable TUSB320 interrupt. */
void TUSB320_IntDisable(void);

/** @brief Write TUSB320 register. */
ErrorStatus TUSB320_WriteReg(uint8_t RegAddr, uint8_t RegVal);

/** @brief Read TUSB320 register. */
ErrorStatus TUSB320_ReadReg(uint8_t RegAddr, uint8_t *RegVal);

/** @brief Get TUSB320 device ID. */
ErrorStatus TUSB320_GetDeviceID(uint8_t *ID);

/** @brief Check if the TUSB320 device ID is correct. */
ErrorStatus TUSB320_CheckDeviceID(uint8_t *ID);

/** @brief Get active cable detection state. */
ErrorStatus TUSB320_GetActiveCableDetectionState(TUSB320_ActiveCable_TypeDef *State);

/** @brief Get accessory connection state. */
ErrorStatus TUSB320_GetAccessoryConnectionState(TUSB320_AccessoryConnected_TypeDef *State);

/** @brief Get the detected current mode advertised by the attached DFP. */
ErrorStatus TUSB320_GetDetectedCurrentMode(TUSB320_CurrentModeDetect_TypeDef *Mode);

/** @brief Set the advertised current mode. */
ErrorStatus TUSB320_SetAdvertisedCurrentMode(TUSB320_CurrentModeAdvertise_TypeDef Mode);

/** @brief Get the advertised current mode. */
ErrorStatus TUSB320_GetAdvertisedCurrentMode(TUSB320_CurrentModeAdvertise_TypeDef *Mode);

/** @brief Set the DRP advertisement duty cycle. */
ErrorStatus TUSB320_SetDRPDutyCycle(TUSB320_DRPDutyCycle_TypeDef DutyCycle);

/** @brief Get the DRP advertisement duty cycle. */
ErrorStatus TUSB320_GetDRPDutyCycle(TUSB320_DRPDutyCycle_TypeDef *DutyCycle);

/** @brief Get the cable direction. */
ErrorStatus TUSB320_GetCableDirection(TUSB320_CableDir_TypeDef *Direction);

/** @brief Get the attachment state. */
ErrorStatus TUSB320_GetAttachmentState(TUSB320_AttachedState_TypeDef *State);

/** @brief Set the operation mode. */
ErrorStatus TUSB320_SetMode(TUSB320_ModeSelect_TypeDef Mode);

/** @brief Get the operation mode. */
ErrorStatus TUSB320_GetMode(TUSB320_ModeSelect_TypeDef *Mode);

/** @brief Set the CC pins voltage debounce time. */
ErrorStatus TUSB320_SetDebounceTime(TUSB320_Debounce_TypeDef Debounce);

/** @brief Get the CC pins voltage debounce time. */
ErrorStatus TUSB320_GetDebounceTime(TUSB320_Debounce_TypeDef *Debounce);

/** @brief Set the Rd and Rp operation mode. */
ErrorStatus TUSB320_SetRdRpMode(TUSB320_RdRp_TypeDef Mode);

/** @brief Get the Rd and Rp operation mode. */
ErrorStatus TUSB320_GetRdRpMode(TUSB320_RdRp_TypeDef *Mode);

/** @brief Perform soft reset. */
ErrorStatus TUSB320_SoftReset(void);

/** @brief Get interrupt state. */
ErrorStatus TUSB320_GetITState(ITStatus *IT);

/** @brief Clear interrupt. */
ErrorStatus TUSB320_ClearIT(void);

/** @brief Interrupt handler callback. */
void TUSB320_IRQHandler(void);

#endif /* __TUSB320_H */

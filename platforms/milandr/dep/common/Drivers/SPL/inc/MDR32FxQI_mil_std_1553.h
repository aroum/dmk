/**
  ******************************************************************************
  * @file    MDR32FxQI_mil_std_1553.h
  * @author  Milandr Application Team
  * @version V2.0.1i
  * @date    24/03/2024
  * @brief   This file contains all the functions prototypes for the MIL STD 1553
  *          firmware library.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MDR32FxQI_MIL_STD_1553_H_
#define MDR32FxQI_MIL_STD_1553_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE1xI)
/** @addtogroup MIL_STD_1553
 *  @warning This module can be used only for MCU K1986VE1xI.
  * @{
  */

/** @defgroup MIL_STD_1553_Exported_Types MIL_STD_1553 Exported Types
  * @{
  */

#define IS_MIL_STD_1553_ALL_PERIPH(PERIPH)  (((PERIPH) == MDR_MIL_STD_15531) || \
                                             ((PERIPH) == MDR_MIL_STD_15532))

#define IS_MIL_STD_1553_DIVIDER(DIVIDER)    ((DIVIDER) <= 0x78)

#define IS_MIL_STD_1553_RTA(RTA)            ((RTA) <= 0x1F)

/**
  * @brief MIL_STD_1553 Mode
  */
typedef enum
{
    MIL_STD_1553_ModeBusController = ((uint32_t)0x01),
    MIL_STD_1553_ModeTerminal      = ((uint32_t)0x02),
    MIL_STD_1553_ModeMonitor       = ((uint32_t)0x03)
} MIL_STD_1553_Mode_Def;

#define IS_MIL_STD_1553_MODE(MODE)  (((MODE) == MIL_STD_1553_ModeBusController) || \
                                     ((MODE) == MIL_STD_1553_ModeMonitor)       || \
                                     ((MODE) == MIL_STD_1553_ModeTerminal))

/**
  * @brief MIL_STD_1553 READ WRITE Bit
  */
typedef enum
{
    MIL_STD_1553_TD_TO_BC = ((uint32_t)0x01),
    MIL_STD_1553_BC_TO_TD = ((uint32_t)0x00)
} MIL_STD_1553_RW_Bit;

#define IS_MIL_STD_READ_WRITE_BIT(BIT)  (((BIT) == MIL_STD_1553_TD_TO_BC) || \
                                         ((BIT) == MIL_STD_1553_BC_TO_TD))

/**
  * @brief MIL_STD_1553 Flags
  */
typedef enum
{
    MIL_STD_1553_FLAG_IDLE    = ((uint32_t)0x01),
    MIL_STD_1553_FLAG_RFLAGN  = ((uint32_t)0x02),
    MIL_STD_1553_FLAG_VALMESS = ((uint32_t)0x04),
    MIL_STD_1553_FLAG_ERR     = ((uint32_t)0x08),
    MIL_STD_1553_FLAG_RCVA    = ((uint32_t)0x10),
    MIL_STD_1553_FLAG_RCVB    = ((uint32_t)0x20)
} MIL_STD_1553_Flags;

#define IS_MIL_STD_1553_FLAG(FLAG)  (((FLAG) == MIL_STD_1553_FLAG_IDLE)    || \
                                     ((FLAG) == MIL_STD_1553_FLAG_RFLAGN)  || \
                                     ((FLAG) == MIL_STD_1553_FLAG_VALMESS) || \
                                     ((FLAG) == MIL_STD_1553_FLAG_ERR)     || \
                                     ((FLAG) == MIL_STD_1553_FLAG_RCVA)    || \
                                     ((FLAG) == MIL_STD_1553_FLAG_RCVB))

/**
  * @brief MIL_STD_1553 Interrupt defintion
  */
typedef enum
{
    MIL_STD_1553_IT_IDLEIE    = ((uint32_t)0x01),
    MIL_STD_1553_IT_RFLAGNIE  = ((uint32_t)0x02),
    MIL_STD_1553_IT_VALMESSIE = ((uint32_t)0x04),
    MIL_STD_1553_IT_ERRIE     = ((uint32_t)0x08)
} MIL_STD_1553_IT_Def;

#define MIL_STD_1553_IT_MASK    (MIL_STD_1553_IT_IDLEIE    | \
                                 MIL_STD_1553_IT_RFLAGNIE  | \
                                 MIL_STD_1553_IT_VALMESSIE | \
                                 MIL_STD_1553_IT_ERRIE)

#define IS_MIL_STD_1553_ITS(IT)  (((IT) & (~MIL_STD_1553_IT_MASK)) == 0)


#define IS_MIL_STD_1553_IT(IT)  (((IT) == MIL_STD_1553_IT_IDLEIE)    || \
                                 ((IT) == MIL_STD_1553_IT_RFLAGNIE)  || \
                                 ((IT) == MIL_STD_1553_IT_VALMESSIE) || \
                                 ((IT) == MIL_STD_1553_IT_ERRIE))

/**
  * @brief MIL_STD_1553 Number Command Word
  */
typedef enum
{
    MIL_STD_1553_COMMAND_WORD1 = ((uint32_t)0x00),
    MIL_STD_1553_COMMAND_WORD2 = ((uint32_t)0x01)
} MIL_STD_1553_Number_CommandWord;

#define IS_MIL_STD_1553_COMMAND_WORD(COMMAND_WORD)    ((COMMAND_WORD) < 0x02)

/**
  * @brief MIL_STD_1553 Number Status Word
  */
typedef enum
{
    MIL_STD_1553_STATUS_WORD1 = ((uint32_t)0x00),
    MIL_STD_1553_STATUS_WORD2 = ((uint32_t)0x01)
} MIL_STD_1553_Number_StatusWord;

#define IS_MIL_STD_1553_STATUS_WORD(STATUS_WORD)     ((STATUS_WORD) < 0x02)

/**
  * @brief MIL_STD_1553 Number Transmitter
  */
typedef enum
{
    MIL_STD_1553_TRANSMITTER_MAIN    = ((uint32_t)0x10),
    MIL_STD_1553_TRANSMITTER_RESERVE = ((uint32_t)0x20)
} MIL_STD_1553_Number_Transmitter;

#define IS_MIL_STD_1553_TRANSMITTER_CHANNEL(CHANNEL)  (((CHANNEL) == MIL_STD_1553_TRANSMITTER_MAIN) || \
                                                       ((CHANNEL) == MIL_STD_1553_TRANSMITTER_RESERVE))

/**
  * @brief MIL_STD_1553 Clock BRG
  */
typedef enum
{
    MIL_STD_1553_HCLKdiv1   = ((uint32_t)0x00),
    MIL_STD_1553_HCLKdiv2   = ((uint32_t)0x01),
    MIL_STD_1553_HCLKdiv4   = ((uint32_t)0x02),
    MIL_STD_1553_HCLKdiv8   = ((uint32_t)0x03),
    MIL_STD_1553_HCLKdiv16  = ((uint32_t)0x04),
    MIL_STD_1553_HCLKdiv32  = ((uint32_t)0x05),
    MIL_STD_1553_HCLKdiv64  = ((uint32_t)0x06),
    MIL_STD_1553_HCLKdiv128 = ((uint32_t)0x07)
} MIL_STD_1553_BRG;

#define IS_MIL_STD_1553_BRG(BRG)   ((BRG) <= 7)

/**
  * @brief MIL_STD_1553 Init Structure definition
  */
typedef struct
{
    MIL_STD_1553_Mode_Def MIL_STD_1553_Mode;                 /*!< This member configures the MIL_STD_1553 mode.
                                                                  This parameter is one of @ref MIL_STD_1553_Mode_Def values. */
    FunctionalState       MIL_STD_1553_RERR;                 /*!< This member configures the resetting of errors in TD mode.
                                                                  This parameter can be a value of @ref FunctionalState. */
    uint32_t              MIL_STD_1553_DIV;                  /*!< This member configures the MIL_STD_1553 clock divider.
                                                                  This parameter is number from 0 to 120.
                                                                  If MAN_CLK not a multiple of 8 then DIV[6:3]=(MAN_CLK/8)+1, DIV[2:0]=0,
                                                                  but the stability of reception is not guaranteed.*/
    uint32_t              MIL_STD_1553_RTA;                  /*!< This member configures the address of the terminal device.
                                                                  This parameter is value from 0 to 0x1E */
    FunctionalState       MIL_STD_1553_TRB;                  /*!< This member configures the Transmission block the backup channel.
                                                                  This parameter can be a value of @ref FunctionalState. */
    FunctionalState       MIL_STD_1553_TRA;                  /*!< This member configures the Transmission block the main channel.
                                                                  This parameter can be a value of @ref FunctionalState. */
    FunctionalState       MIL_STD_1553_Inversion_Signals;    /*!< This member enables or disables the inversion signals
                                                                  PRD_PRMA, PRD_PRMB, PRD_PRMC, PRD_PRMD.
                                                                  This parameter can be a value of @ref FunctionalState. */
#if defined  (USE_MDR32F1QI_REV3_4)
    /**
     * @note This members may be used only for MDR32F1QI rev. 3 and 4.
     */
    FunctionalState       MIL_STD_1553_Filtration_Flow;      /*!< This option enables or disables the filtering flow NRZ, in the case
                                                                  of drivers with not correct duty cycle and pulse width of NRZ code.
                                                                  This parameter can be a value of @ref FunctionalState. */
    FunctionalState       MIL_STD_1553_Automatic_Adjustment; /*!< This member enables or disables the automatic adjustment of the mid bit intervals.
                                                                  This parameter can be a value of @ref FunctionalState. */
#elif defined (USE_MDR32F1QI_REV6) || (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
    /**
     * @note This members may be used only for MDR32F1QI rev. 6 and later and for K1986VE1xI.
     */
    FunctionalState       MIL_STD_1553_EN_FLT;               /*!< This option enables or disables impulse noise filtering.
                                                                  It is recommended to set to zero (RESET).
                                                                  This parameter can be a value of @ref FunctionalState. */
    FunctionalState       MIL_STD_1553_INPINV;               /*!< This option enables or disables inversion of the receiver inputs PRMx+ and PRMx-
                                                                  The PRMx+ input is inverted and switched to the negative input of the Manchester decoder.
                                                                  The PRMx- input is inverted and switched to the positive input of the Manchester decoder.
                                                                  This parameter can be a value of @ref FunctionalState. */
    FunctionalState       MIL_STD_1553_RT_HW;                /*!< This option enables or disables hardware support
                                                                  GOST 51765-2001 for TerminalDevice mode.
                                                                  This parameter can be a value of @ref FunctionalState. */
#endif
} MIL_STD_1553_InitTypeDef;

/**
  * @brief Data Fields Union definition
  */
typedef union
{
    uint32_t NumberDataWords :5; /*!< Specifies the number of data word in a message. */
    uint32_t Cmd             :5; /*!< Specifies the command type.*/
} Data_FieldsTypeDef;

/**
  * @brief MIL_STD_1553 CommandWord Bit Fields definition
  */
typedef struct
{
    /*Data_FieldsTypeDef Data;*/
    uint32_t Data                      :5; /*!< Specifies the number of data word in a message or command type.
                                                This parameter is number from 0 to 0x1F. */
    uint32_t Subaddress                :5; /*!< Specifies the code subaddress OU or code attribute control mode 00000 or 11111.
                                                This parameter is number from 0 to 0x1F. */
    MIL_STD_1553_RW_Bit ReadWriteBit   :1; /*!< Specifies the action to execute TD (receive or transmit DW).
                                                This parameter is one of @ref MIL_STD_1553_RW_Bit values. */
    uint32_t TerminalDeviceAddress     :5; /*!< Specifies the address of TD, which is intended CW.
                                                This parameter is number from 0 to 0x1F. */
} MIL_STD_1553_CommandWordBitFields;

/**
  * @brief MIL_STD_1553 CommandWord Unioun definition
  */
typedef union
{
    uint32_t                          CommandWord; /*!< Specifies the fully Command Word. */
    MIL_STD_1553_CommandWordBitFields Fields;      /*!< Specifies the particular field of the Command Word.*/

} MIL_STD_1553_CommandWordTypeDef;

/**
  * @brief MIL_STD_1553 StatusWord Bit Fields definition
  */
typedef struct
{
    uint32_t FaultTDBit                  :1; /*!< Specifies the fault terminal device.
                                                  This parameter can be: SET or RESET. */
    uint32_t AdoptionControlInterfaceBit :1; /*!< Specifies the adoption control interface.
                                                  This parameter can be: SET or RESET. */
    uint32_t AbonentFaultBit             :1; /*!< Specifies the fault abonent.
                                                  This parameter can be: SET or RESET. */
    uint32_t BusyBit                     :1; /*!< Specifies the busy abonent.
                                                  This parameter can be: SET or RESET. */
    uint32_t GroupCommandBit             :1; /*!< Specifies the group command.
                                                  This parameter can be: SET or RESET. */
    uint32_t Reserved                    :3; /*!< Reserved */
    uint32_t ServiceRequestBit           :1; /*!< Specifies the service request.
                                                  This parameter can be: SET or RESET. */
    uint32_t TransferReplyWordBit        :1; /*!< Specifies the transfer replay word.
                                                  This parameter can be: SET or RESET. */
    uint32_t ErrorBit                    :1; /*!< Specifies the error in the message.
                                                  This parameter can be: SET or RESET. */
    uint32_t TerminalDeviceAddress       :5; /*!< Specifies the termnal device address.
                                                  This parameter is number from 0x00 to 0x1F. */
} MIL_STD_1553_StatusWordBitFields;

/**
  * @brief MIL_STD_1553 StatusWord Structure definition
  */
typedef union
{
    uint32_t                         StatusWord; /*!< Specifies the fully Status Word. */
    MIL_STD_1553_StatusWordBitFields Fields;     /*!< Specifies the particular field of the Status Word. */
} MIL_STD_1553_StatusWordTypeDef;


/** @} */ /* End of group MIL_STD_1553_Exported_Types */


/** @defgroup MIL_STD_1553_Exported_Functions MIL_STD_1553 Exported Functions
  * @{
  */

void MIL_STD_1553_BRGInit(MIL_STD_1553_BRG MIL_STD_1553_BRG_Value);
void MIL_STD_1553_DeInit(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);
void MIL_STD_1553_Init(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_InitTypeDef * MIL_STD_1553_InitStruct);
void MIL_STD_1553xStructInit(MIL_STD_1553_InitTypeDef * MIL_STD_1553_InitStruct);
void MIL_STD_1553_Cmd(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, FunctionalState NewState);
uint32_t MIL_STD_1553_GetCommandWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_CommandWord COMMAND_WORDx);
void MIL_STD_1553_SetCommandWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_CommandWord COMMAND_WORDx, MIL_STD_1553_CommandWordTypeDef * CommandWord);
uint32_t MIL_STD_1553_GetModeData(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);
uint32_t MIL_STD_1553_GetMSG(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);
void MIL_STD_1553_SetStatusWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_StatusWord STATUS_WORDx, MIL_STD_1553_StatusWordTypeDef * StatusWord);
uint32_t MIL_STD_1553_GetStatusWord(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_StatusWord STATUS_WORDx);
uint32_t MIL_STD_1553_GetStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);
FlagStatus MIL_STD_1553_GetFlagStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Flags flag);
void MIL_STD_1553_ITConfig(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t MIL_STD_1553_IT, FunctionalState NewState);
void MIL_STD_1553_StartTransmission(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);

void MIL_STD_1553_ReceiveData(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t Subaddress, uint32_t NumberDataWords, uint32_t * ptr_Dest);
void MIL_STD_1553_WriteDataToSendBuffer(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, uint32_t Subaddress, uint32_t NumberDataWords, uint32_t * ptr_Src);

void MIL_STD_1553_Transmitter_CMD(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, MIL_STD_1553_Number_Transmitter TRANSMITTERx, FunctionalState NewState);
uint32_t MIL_STD_1553_GetErrorStatus(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x);
void MIL_STD_1553_RERRCmd(MDR_MIL_STD_1553_TypeDef * MIL_STD_1553x, FunctionalState NewState);

/** @} */ /* End of group MIL_STD_1553_Exported_Functions */

/** @} */ /* End of group MIL_STD_1553 */
#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* MDR32FxQI_MIL_STD_1553_H_ */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_mil_std_1553.h */



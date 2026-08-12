/**
  ******************************************************************************
  * @file    MDR32FxQI_eeprom.c
  * @author  Milandr Application Team
  * @version V2.3.0i
  * @date    09/12/2024
  * @brief   This file contains all the EEPROM firmware functions.
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
#include "MDR32FxQI_eeprom.h"
#include "MDR32FxQI_utils.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup EEPROM EEPROM
  * @{
  */

/** @addtogroup EEPROM_Private_Defines EEPROM Private Defines
  * @{
  */

#define EEPROM_REG_ACCESS_KEY            ((uint32_t)0x8AAA5551)

#define EEPROM_ADDR_BYTE_MASK            ((uint32_t)0x3)
#define EEPROM_ADDR_HALFWORD_MASK        ((uint32_t)0x2)

#define EEPROM_ADDR_TWO_BYTE_ALIGN_MASK  ((uint32_t)0x1)
#define EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK ((uint32_t)0x3)

#define EEPROM_ADDR_SECTOR_POS           2
#define EEPROM_ADDR_SECTOR_MASK          ((uint32_t)0x3 << EEPROM_ADDR_SECTOR_POS)

#define EEPROM_ADDR_INFO_WORD_MASK       ((uint32_t)0xFFC)

/** @} */ /* End of group EEPROM_Private_Defines */

/** @addtogroup EEPROM_Private_Macros EEPROM Private Macros
  * @{
  */

#define IS_EEPROM_TWO_BYTE_ALIGNED(ADDR)  (((ADDR) & EEPROM_ADDR_TWO_BYTE_ALIGN_MASK) == 0)
#define IS_EEPROM_FOUR_BYTE_ALIGNED(ADDR) (((ADDR) & EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK) == 0)

#define EEPROM_GET_SECTOR_NUMBER(ADDR)    (((ADDR) & EEPROM_ADDR_SECTOR_MASK) >> EEPROM_ADDR_SECTOR_POS)

/** @} */ /* End of group EEPROM_Private_Macros */

/** @addtogroup EEPROM_Private_Function_Prototypes EEPROM Private Functions Prototypes
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
__RAMFUNC __STATIC_INLINE void EEPROM_UpdateDCacheC(void);
#elif defined (USE_K1986VE1xI)
__RAMFUNC void EEPROM_UpdateDCacheAsm(void);
#endif
__RAMFUNC static void EEPROM_MassErase(EEPROM_Mem_Bank BankSelector);

/** @} */ /* End of group EEPROM_Private_Function_Prototypes */


/** @addtogroup EEPROM_Exported_Functions EEPROM Exported Functions
  * @{
  */

/** @addtogroup EEPROM_Functions_allowed_to_be_located_in_Flash Functions allowed to be located in Flash
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets instructions and/or data cache usage.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   EEPROM_Cache - @ref EEPROM_Cache_Type - specifies the cache type to be used.
  * @retval  None
  */
void EEPROM_SetCacheState(EEPROM_Cache_Type EEPROM_Cache)
{
    /* Check the parameters */
    assert_param(IS_EEPROM_CACHE(EEPROM_Cache));

    /* Set the selected cache */
    MDR_EEPROM->CTRL = EEPROM_Cache;
}

/**
  * @brief   Gets instructions and/or data cache usage.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   None
  * @retval  @ref EEPROM_Cache_Type: used cache type.
  */
EEPROM_Cache_Type EEPROM_GetCacheState(void)
{
    return (EEPROM_Cache_Type)(MDR_EEPROM->CTRL & EEPROM_Cache_Msk);
}

#endif /* #if defined (USE_MDR32FG16S1QI) */

/**
  * @brief   Sets the code latency value.
  * @warning EEPROM latency cycles should be set before increase of clock frequency
  *          or after clock frequency reduction.
  *          EEPROM latency should not be changed in programming mode.
  * @param   EEPROM_Latency - @ref EEPROM_Latency_Cycles - specifies the EEPROM Latency value.
  * @retval  None
  */
void EEPROM_SetLatency(EEPROM_Latency_Cycles EEPROM_Latency)
{
    /* Check the parameters */
    assert_param(IS_EEPROM_LATENCY(EEPROM_Latency));

    /* Set the new latency value */
    MDR_EEPROM->CMD = EEPROM_Latency;
}

/**
  * @brief  Gets the code latency value.
  * @param  None
  * @retval @ref EEPROM_Latency_Cycles - EEPROM Latency cycles value.
  */
EEPROM_Latency_Cycles EEPROM_GetLatency(void)
{
    /* Get current latency value */
    return (EEPROM_Latency_Cycles)(MDR_EEPROM->CMD & EEPROM_Latency_MSK);
}

/** @} */ /* End of group EEPROM_Functions_allowed_to_be_located_in_Flash */

/** @addtogroup EEPROM_Functions_not_allowed_to_be_located_in_Flash Functions not allowed to be located in Flash
  * @{
  */

/**
  * @brief  Reads the 8-bit EEPROM memory value.
  * @note   Hardware supports only 32-bit word reading.
  * @param  Address: The EEPROM memory byte address.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @retval The selected EEPROM memory value.
  */
__RAMFUNC uint8_t EEPROM_ReadByte(uint32_t Address, EEPROM_Mem_Bank BankSelector)
{
    uint32_t Data;
    uint32_t Shift;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));

    Data = EEPROM_ReadWord(Address & ~EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK, BankSelector);

    Shift = (Address & EEPROM_ADDR_BYTE_MASK) * 8;
    Data >>= Shift;

    return (uint8_t)Data;
}

/**
  * @brief  Reads the 16-bit EEPROM memory value.
  * @note   Hardware supports only 32-bit word reading.
  * @param  Address: The EEPROM memory half word address (two byte aligned).
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @retval The selected EEPROM memory value.
  */
__RAMFUNC uint16_t EEPROM_ReadHalfWord(uint32_t Address, EEPROM_Mem_Bank BankSelector)
{
    uint32_t Data;
    uint32_t Shift;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
    assert_param(IS_EEPROM_TWO_BYTE_ALIGNED(Address));

    Data = EEPROM_ReadWord(Address & ~EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK, BankSelector);

    Shift = (Address & EEPROM_ADDR_HALFWORD_MASK) * 8;
    Data >>= Shift;

    return (uint16_t)Data;
}

/**
  * @brief  Reads the 32-bit EEPROM memory value.
  * @param  Address: The EEPROM memory word address (four byte aligned).
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @retval The selected EEPROM memory value.
  */
__RAMFUNC uint32_t EEPROM_ReadWord(uint32_t Address, EEPROM_Mem_Bank BankSelector)
{
  volatile uint32_t Command;
  volatile uint32_t Data;

  assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
  assert_param(IS_EEPROM_FOUR_BYTE_ALIGNED(Address));

  MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
  Command = (MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk) | EEPROM_CMD_CON;
  Command |= (BankSelector == EEPROM_Info_Bank_Select) ? EEPROM_CMD_IFREN : 0;
  MDR_EEPROM->CMD = Command;
  MDR_EEPROM->ADR = Address;
  MDR_EEPROM->CMD = Command | EEPROM_CMD_XE | EEPROM_CMD_YE | EEPROM_CMD_SE;
  MDR_EEPROM->DO; /* Idle Reading for Delay */
  MDR_EEPROM->DO; /* Idle Reading for Delay */
  MDR_EEPROM->DO; /* Idle Reading for Delay */
  Data = MDR_EEPROM->DO;
  Command &= EEPROM_CMD_DELAY_Msk;
  MDR_EEPROM->CMD = Command;
  MDR_EEPROM->KEY = 0;

  return Data;
}

/**
  * @brief  Read the array of the 32-bit EEPROM memory values in the packet mode.
  * @param  PtrAddressArray: The pointer to the array with EEPROM memory words addresses to read (four bytes aligned).
  * @param  PtrDataArray: The pointer to the array where the words read from the EEPROM memory will be stored.
  * @param  ArraySize: The size of address and data arrays.
  * @param  BankSelector: @ref EEPROM_Mem_Bank - select a EEPROM bank (the main bank or the information bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select: The EEPROM main bank;
  *             @arg EEPROM_Info_Bank_Select: The EEPROM information bank.
  * @return None.
  */
__RAMFUNC void EEPROM_ReadWordArrayBurst(const uint32_t* PtrAddressArray, uint32_t* PtrDataArray, uint32_t ArraySize, EEPROM_Mem_Bank BankSelector)
{
    uint32_t Command;
    uint32_t Index;

    /* Check the parameters. */
    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
    assert_param(ArraySize > 0);
#if (USE_ASSERT_INFO != 0)
    for (Index = 0; Index < ArraySize; Index++)
    {
        assert_param(IS_EEPROM_FOUR_BYTE_ALIGNED(PtrAddressArray[Index]));
    }
#endif

    MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
    Command = (MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk) | EEPROM_CMD_CON;
    Command |= (BankSelector == EEPROM_Info_Bank_Select) ? EEPROM_CMD_IFREN : 0;
    MDR_EEPROM->CMD = Command;

    for (Index = 0; Index < ArraySize; Index++)
    {
        MDR_EEPROM->ADR = PtrAddressArray[Index];
        MDR_EEPROM->CMD = Command | EEPROM_CMD_XE | EEPROM_CMD_YE | EEPROM_CMD_SE;
        MDR_EEPROM->DO;   /* Idle Reading for Delay */
        MDR_EEPROM->DO;   /* Idle Reading for Delay */
        MDR_EEPROM->DO;   /* Idle Reading for Delay */
        PtrDataArray[Index] = MDR_EEPROM->DO;
    }
    Command &= EEPROM_CMD_DELAY_Msk;
    MDR_EEPROM->CMD = Command;
    MDR_EEPROM->KEY = 0;
}

/**
  * @brief  Erases one page of the selected EEPROM memory bank.
  * @note   For K1986VE9xI, K1986VE1xI sector A of the EEPROM Information Bank containing the factory data is not erased.
  * @param  Address: Page Address in the EEPROM memory.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @retval None
  */
__RAMFUNC void EEPROM_ErasePage(uint32_t Address, EEPROM_Mem_Bank BankSelector)
{
    uint32_t Command, DelayConstUs;
    uint32_t Sector;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));

    DelayConstUs = DELAY_PROGRAM_GET_CONST_US((uint32_t)(FLASH_PROG_FREQ_MHZ*1E6));
    Address = Address & ~EEPROM_ADDR_SECTOR_MASK;
    Sector = 0;

#if !defined (USE_MDR32F9xI) && !defined (USE_MDR32F1QI) && !defined (USE_MDR32FG16S1QI)
    if (BankSelector == EEPROM_Info_Bank_Select)
    {
        Sector = 1;
    }
#endif

    MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
    Command = (MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk) | EEPROM_CMD_CON;
    Command |= (BankSelector == EEPROM_Info_Bank_Select) ? EEPROM_CMD_IFREN : 0;
    MDR_EEPROM->CMD = Command;

    for (; Sector < 4; Sector++)
    {
        MDR_EEPROM->ADR = Address | (Sector << EEPROM_ADDR_SECTOR_POS); /* Page Address */
        MDR_EEPROM->DI = 0;
        Command |= EEPROM_CMD_XE | EEPROM_CMD_ERASE;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVS_US, DelayConstUs));
        Command |= EEPROM_CMD_NVSTR;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TERASE_US, DelayConstUs));
        Command &= ~EEPROM_CMD_ERASE;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVH_US, DelayConstUs));
        Command &= ~(EEPROM_CMD_XE | EEPROM_CMD_NVSTR);
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TRCV_US, DelayConstUs));
    }

    Command &= EEPROM_CMD_DELAY_Msk;
    MDR_EEPROM->CMD = Command;
    MDR_EEPROM->KEY = 0;

    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_THV_US, DelayConstUs));
}

/**
  * @brief  Erases all pages of the selected EEPROM memory bank.
  * @note   Erasing all pages of Information Bank automatically erases Main Bank.
  * @note   For K1986VE9xI, K1986VE1xI sector A of the EEPROM Information Bank containing the factory data is not erased.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Main and Information Banks).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_All_Banks_Select:      The EEPROM Main and Information Banks selector.
  * @retval None
  */
__RAMFUNC void EEPROM_EraseAllPages(EEPROM_Mem_Bank BankSelector)
{
#if !defined (USE_MDR32F9xI) && !defined (USE_MDR32F1QI) && !defined (USE_MDR32FG16S1QI)
    EEPROM_MassErase(EEPROM_Main_Bank_Select);
    EEPROM_ErasePage(EEPROM_BASE, EEPROM_Info_Bank_Select);
#else
    EEPROM_MassErase(BankSelector);
#endif
}

/**
  * @brief  Programs the 8-bit EEPROM memory value.
  * @note   Hardware supports only 32-bit word programming.
  * @note   For K1986VE9xI, K1986VE1xI the memory area containing the factory data
  *         (EEPROM Information Bank, sector A, 0xF80-0xFF0) is not programmed.
  * @param  Address: The EEPROM memory byte address.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @param  Data: The data value to be programmed.
  * @retval None
  */
__RAMFUNC void EEPROM_ProgramByte(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data)
{
    uint32_t Mask;
    uint32_t Tmp;
    uint32_t Shift;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));

    Shift = (Address & EEPROM_ADDR_BYTE_MASK) * 8;
    Data = Data << Shift;
    Mask = (uint32_t)0xFF << Shift;
    Address &= ~EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK;

    Tmp = EEPROM_ReadWord(Address, BankSelector);

    EEPROM_ProgramWord(Address, BankSelector, (Tmp & ~Mask) | (Data & Mask));
}

/**
  * @brief  Programs the 16-bit EEPROM memory value.
  * @note   Hardware supports only 32-bit word programming.
  * @note   For K1986VE9xI, K1986VE1xI the memory area containing the factory data
  *         (EEPROM Information Bank, sector A, 0xF80-0xFF0) is not programmed.
  * @param  Address: The EEPROM memory half word address (two byte aligned).
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @param  Data: The data value to be programmed.
  * @retval None
  */
__RAMFUNC void EEPROM_ProgramHalfWord(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data)
{
    uint32_t Mask;
    uint32_t Tmp;
    uint32_t Shift;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
    assert_param(IS_EEPROM_TWO_BYTE_ALIGNED(Address));

    Shift = (Address & EEPROM_ADDR_HALFWORD_MASK) * 8;
    Data = Data << Shift;
    Mask = (uint32_t)0xFFFF << Shift;
    Address &= ~EEPROM_ADDR_FOUR_BYTE_ALIGN_MASK;

    Tmp = EEPROM_ReadWord(Address, BankSelector);

    EEPROM_ProgramWord(Address, BankSelector, (Tmp & ~Mask) | (Data & Mask));
}

/**
  * @brief  Programs the 32-bit EEPROM memory value.
  * @param  Address: The EEPROM memory word address (four byte aligned).
  * @note   For K1986VE9xI, K1986VE1xI the memory area containing the factory data
  *         (EEPROM Information Bank, sector A, 0xF80-0xFF0) is not programmed.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Information Bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_Info_Bank_Select:      The EEPROM Information Bank selector.
  * @param  Data: The data value to be programmed.
  * @retval None
  */
__RAMFUNC void EEPROM_ProgramWord(uint32_t Address, EEPROM_Mem_Bank BankSelector, uint32_t Data)
{
    uint32_t Command, DelayConstUs, DelayProgUs;

    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
    assert_param(IS_EEPROM_FOUR_BYTE_ALIGNED(Address));

#if !defined (USE_MDR32F9xI) && !defined (USE_MDR32F1QI) && !defined (USE_MDR32FG16S1QI)
    if (BankSelector == EEPROM_Info_Bank_Select)
    {
        if (EEPROM_GET_SECTOR_NUMBER(Address) == 0)
        {
            if ((Address & EEPROM_ADDR_INFO_WORD_MASK) >= (EEPROM_FACTORY_DATA_BASE & EEPROM_ADDR_INFO_WORD_MASK))
            {
                return;
            }
        }
    }
#endif

    DelayConstUs = DELAY_PROGRAM_GET_CONST_US((uint32_t)(FLASH_PROG_FREQ_MHZ*1E6));
    DelayProgUs = DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TPROG_US, DelayConstUs);

    MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
    Command = MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk;
    Command |= EEPROM_CMD_CON;
    Command |= (BankSelector == EEPROM_Info_Bank_Select) ? EEPROM_CMD_IFREN : 0;
    MDR_EEPROM->CMD = Command;
    MDR_EEPROM->ADR = Address;
    MDR_EEPROM->DI  = Data;
    Command |= EEPROM_CMD_XE | EEPROM_CMD_PROG;
    MDR_EEPROM->CMD = Command;
    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVS_US, DelayConstUs));
    Command |= EEPROM_CMD_NVSTR;
    MDR_EEPROM->CMD = Command;
    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TPGS_US, DelayConstUs));
    Command |= EEPROM_CMD_YE;
    MDR_EEPROM->CMD = Command;
    DELAY_PROGRAM_WaitLoopsAsm(DelayProgUs);
    Command &= ~EEPROM_CMD_YE;
    MDR_EEPROM->CMD = Command;
    Command &= ~EEPROM_CMD_PROG;
    MDR_EEPROM->CMD = Command;
    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVH_US, DelayConstUs));
    Command &= ~(EEPROM_CMD_XE | EEPROM_CMD_NVSTR);
    MDR_EEPROM->CMD = Command;
    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TRCV_US, DelayConstUs));

    MDR_EEPROM->CMD = Command & EEPROM_CMD_DELAY_Msk;
    MDR_EEPROM->KEY = 0;
}

/**
  * @brief  Program an array of the 32-bit EEPROM memory values in the packet mode.
  * @note   For MDR1986VE9x_002TU, MDR1986VE1x_002TU:
  *         If USE_EEPROM_FACTORY_DATA_PROTECTION is defined, the memory area containing the factory data
  *         (EEPROM Information Bank, sector A, 0xF80-0xFF0) is not programmed.
  * @param  PtrAddressArray: The pointer to the array with EEPROM memory words addresses to write (four bytes aligned).
  * @param  PtrDataArray: The pointer to the data array to program.
  * @param  ArraySize: The size of address and data arrays.
  * @param  BankSelector: @ref EEPROM_Mem_Bank - select a EEPROM bank (the main bank or the information bank).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select: The EEPROM main bank;
  *             @arg EEPROM_Info_Bank_Select: The EEPROM information bank.
  * @return None.
  */
__RAMFUNC void EEPROM_ProgramWordArrayBurst(const uint32_t* PtrAddressArray, const uint32_t* PtrDataArray, uint32_t ArraySize, EEPROM_Mem_Bank BankSelector)
{
    uint32_t Command;
    uint32_t DelayConstUs;
    uint32_t DelayProgUs;
    uint32_t Index;

    /* Check the parameters. */
    assert_param(IS_EEPROM_BANK_SELECTOR(BankSelector));
    assert_param(ArraySize > 0);
#if (USE_ASSERT_INFO != 0)
    for (Index = 0; Index < ArraySize; Index++) {
        assert_param(IS_EEPROM_FOUR_BYTE_ALIGNED(PtrAddressArray[Index]));
    }
#endif

    DelayConstUs = DELAY_PROGRAM_GET_CONST_US((uint32_t)(FLASH_PROG_FREQ_MHZ*1E6));
    DelayProgUs = DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TPROG_US, DelayConstUs);

    MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
    Command = (MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk) | EEPROM_CMD_CON;
    Command |= (BankSelector == EEPROM_Info_Bank_Select) ? EEPROM_CMD_IFREN : 0;
    MDR_EEPROM->CMD = Command;

    Index = 0;
    while (Index < ArraySize)
    {
#if !defined (USE_MDR32F9xI) && !defined (USE_MDR32F1QI) && !defined (USE_MDR32FG16S1QI)
        if (BankSelector == EEPROM_Info_Bank_Select)
            {
                if (EEPROM_GET_SECTOR_NUMBER(PtrAddressArray[Index]) == 0)
                {
                    if ((PtrAddressArray[Index] & EEPROM_ADDR_INFO_WORD_MASK) >= (EEPROM_FACTORY_DATA_BASE & EEPROM_ADDR_INFO_WORD_MASK))
                    {
                        Index++;
                        continue;
                    }
                }
            }
#endif
        MDR_EEPROM->ADR = PtrAddressArray[Index];
        MDR_EEPROM->DI  = PtrDataArray[Index];

        Command |= EEPROM_CMD_XE | EEPROM_CMD_PROG;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVS_US, DelayConstUs));

        Command |= EEPROM_CMD_NVSTR;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TPGS_US, DelayConstUs));


        Command |= EEPROM_CMD_YE;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DelayProgUs);
        Command &= ~EEPROM_CMD_YE;
        MDR_EEPROM->CMD = Command;

        Index++;
        /* Can be programmed with reduced delays if inside single page. */
        while ((Index < ArraySize) && ((PtrAddressArray[Index - 1] & ~((uint32_t)0xFF0)) == (PtrAddressArray[Index] & ~((uint32_t)0xFF0))))
        {
#if !defined (USE_MDR32F9xI) && !defined (USE_MDR32F1QI) && !defined (USE_MDR32FG16S1QI)
            if (BankSelector == EEPROM_Info_Bank_Select)
                {
                    if (EEPROM_GET_SECTOR_NUMBER(PtrAddressArray[Index]) == 0)
                    {
                        if ((PtrAddressArray[Index] & EEPROM_ADDR_INFO_WORD_MASK) >= (EEPROM_FACTORY_DATA_BASE & EEPROM_ADDR_INFO_WORD_MASK))
                        {
                            Index++;
                            continue;
                        }
                    }
                }
#endif
            MDR_EEPROM->ADR = PtrAddressArray[Index];
            MDR_EEPROM->DI  = PtrDataArray[Index];

            Command |= EEPROM_CMD_YE;
            MDR_EEPROM->CMD = Command;
            DELAY_PROGRAM_WaitLoopsAsm(DelayProgUs);
            Command &= ~EEPROM_CMD_YE;
            MDR_EEPROM->CMD = Command;

            Index++;
        }
        Command &= ~EEPROM_CMD_PROG;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVH_US, DelayConstUs));

        Command &= ~(EEPROM_CMD_XE | EEPROM_CMD_NVSTR);
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TRCV_US, DelayConstUs));
    }

    MDR_EEPROM->CMD = Command & EEPROM_CMD_DELAY_Msk;
    MDR_EEPROM->KEY = 0;
}

/**
  * @brief   Updates data cache.
  * @warning C implementation is used for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  *          ASM implementation is used for MCU MDR32F1QI, K1986VE1xI (errata 0007).
  * @param   None
  * @retval  None
  */
__RAMFUNC void EEPROM_UpdateDCache(void)
{
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    EEPROM_UpdateDCacheC();
#elif defined (USE_K1986VE1xI)
    EEPROM_UpdateDCacheAsm();
#endif
}

/** @} */ /* End of group EEPROM_Functions_not_allowed_to_be_located_in_Flash */

/** @} */ /* End of group EEPROM_Exported_Functions */

/** @addtogroup EEPROM_Private_Functions EEPROM Private Functions
  * @{
  */

/** @addtogroup EEPROM_Private_Functions_not_allowed_to_be_located_in_Flash Functions not allowed to be located in Flash
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Updates data cache.
  * @warning This C-style function is only used for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None
  * @retval  None
  */
__RAMFUNC __STATIC_INLINE void EEPROM_UpdateDCacheC(void)
{
    volatile uint32_t i, tmp;
    volatile uint32_t *ptr_eeprom = (uint32_t *) (EEPROM_BASE);
    
#if defined (USE_MDR32FG16S1QI)
    if(MDR_EEPROM->CTRL & EEPROM_Cache_Data)
    {
        i = 40;
    }
    else
    {
        i = 5;
    }
#else
    i = 5;
#endif /* #if defined (USE_MDR32FG16S1QI) */
    
    /* Reading EEPROM memory to update the data cache */
    for(; i > 0; i--)
    {
        tmp = ptr_eeprom[i << 2];
    }
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Erases Main Bank or Main and Information Banks.
  * @param  BankSelector - @ref EEPROM_Mem_Bank - Selects EEPROM Bank (Main Bank or Main and Information Banks).
  *         This parameter can be one of the following values:
  *             @arg EEPROM_Main_Bank_Select:      The EEPROM Main Bank selector.
  *             @arg EEPROM_All_Banks_Select:      The EEPROM Main and Information Banks selector.
  * @retval None
  */
__RAMFUNC static void EEPROM_MassErase(EEPROM_Mem_Bank BankSelector)
{
    uint32_t Command, DelayConstUs;
    uint32_t Sector;

    assert_param(IS_EEPROM_ERASE_SELECTOR(BankSelector));

    DelayConstUs = DELAY_PROGRAM_GET_CONST_US((uint32_t)(FLASH_PROG_FREQ_MHZ*1E6));

    MDR_EEPROM->KEY = EEPROM_REG_ACCESS_KEY;
    Command = (MDR_EEPROM->CMD & EEPROM_CMD_DELAY_Msk) | EEPROM_CMD_CON;
    Command |= (BankSelector == EEPROM_All_Banks_Select) ? EEPROM_CMD_IFREN : 0;

    MDR_EEPROM->CMD = Command;

    for (Sector = 0; Sector < 4; Sector++)
    {
        MDR_EEPROM->ADR = Sector << EEPROM_ADDR_SECTOR_POS;
        MDR_EEPROM->DI = 0;
        Command |= EEPROM_CMD_XE | EEPROM_CMD_MAS1 | EEPROM_CMD_ERASE;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVS_US, DelayConstUs));
        Command |= EEPROM_CMD_NVSTR;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TME_US, DelayConstUs));
        Command &= ~EEPROM_CMD_ERASE;
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TNVH1_US, DelayConstUs));
        Command &= ~(EEPROM_CMD_XE | EEPROM_CMD_MAS1 | EEPROM_CMD_NVSTR);
        MDR_EEPROM->CMD = Command;
        DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_TRCV_US, DelayConstUs));
    }

    Command &= EEPROM_CMD_DELAY_Msk;
    MDR_EEPROM->CMD = Command;
    MDR_EEPROM->KEY = 0;

    DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(EEPROM_THV_US, DelayConstUs));
}

/** @} */ /* End of group EEPROM_Private_Functions_not_allowed_to_be_located_in_Flash */

/** @} */ /* End of group EEPROM_Private_Functions */

/** @} */ /* End of group EEPROM */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_eeprom.c */



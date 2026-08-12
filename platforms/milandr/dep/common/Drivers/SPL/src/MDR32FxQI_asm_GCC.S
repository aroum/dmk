/**
        !!!!!!!!!THIS FILE WAS MODIFIED BY AI USE CAUTIOUSLY!!!!!!!!!
  ******************************************************************************
  * @file    MDR32FxQI_asm.S
  * @author  Milandr Application Team
  * @version V2.0.4i
  * @date    24/07/2024
  * @brief   This file contains assembler functions.
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
  * <h2><center>&copy; COPYRIGHT 2024 Milandr</center></h2>
  ******************************************************************************
  */

.syntax unified
.thumb

.section EXECUTABLE_MEMORY_SECTION, "ax"
.align 2

/**
  * @brief   Updates data cache.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI (errata 0007).
  *          To call the function use the name EEPROM_UpdateDCache().
  *          Registers r0-r1 are not stored on the stack, as they are non preserved registers.
  * @param   None
  * @retval  None
  */
.global EEPROM_UpdateDCacheAsm
.thumb_func
.type EEPROM_UpdateDCacheAsm, %function
EEPROM_UpdateDCacheAsm:
        MOVS    r0,#0x50        @ r0 = 0x00000050
        NOP                     @ Align next LDR instruction
EEPROM_ReadFlash:
        LDR     r1,[r0,#0x00]   @ Read EEPROM
        SUBS    r0,r0,#0x10     @ r0 -= 0x10
        CMP     r0,#0x00
        BNE     EEPROM_ReadFlash

        BX      LR
.size EEPROM_UpdateDCacheAsm, .-EEPROM_UpdateDCacheAsm

.align 2

/**
  * @brief   Performs a blocking program delay in loops.
  * @note    Register r0 is not stored on the stack because it is non preserved register.
  * @param   Loops: specifies the delay time in loops.
  * @retval  None.
  */
.global DELAY_PROGRAM_WaitLoopsAsm
.thumb_func
.type DELAY_PROGRAM_WaitLoopsAsm, %function
DELAY_PROGRAM_WaitLoopsAsm:
        CMP     r0,#0x00
        BEQ     DELAY_PROGRAM_Exit

DELAY_PROGRAM_Loop:
        SUBS    r0,r0,#1
        BNE     DELAY_PROGRAM_Loop

DELAY_PROGRAM_Exit:
        BX      LR
.size DELAY_PROGRAM_WaitLoopsAsm, .-DELAY_PROGRAM_WaitLoopsAsm

.end
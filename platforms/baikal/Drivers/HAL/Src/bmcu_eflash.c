/**
 * *****************************************************************************
 *  @file       bmcu_eflash.c
 *  @author     Baikal electronics SDK team
 *  @brief      MCU embedded flash memory driver
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

#include "bmcu_eflash.h"
#include "bmcu_def.h"

#define EFLASH_MAX_DELAY                0xFFFFFFFFUL

/* EFLASH operation timeouts */
#define EFLASH_RESET_TIMEOUT            EFLASH_MAX_DELAY
#define EFLASH_CFG_TIMEOUT              EFLASH_MAX_DELAY
#define EFLASH_PROGRAM_TIMEOUT          EFLASH_MAX_DELAY
#define EFLASH_READ_TIMEOUT             EFLASH_MAX_DELAY
#define EFLASH_ERASE_TIMEOUT            EFLASH_MAX_DELAY

/* EFLASH timing parameters */
#define EFLASH_TIME0_DS_T_NS            15UL        /*!< Time DS_T, ns */
#define EFLASH_TIME0_SCE_T_NS           8000000UL   /*!< Time SCE_T, ns */
#define EFLASH_TIME0_AA_T_NS            25UL        /*!< Time AA_T, ns */
#define EFLASH_TIME1_AS_T_NS            2UL         /*!< Time AS_T, ns */
#define EFLASH_TIME1_RHR_T_NS           10000UL     /*!< Time RHR_T, ns */
#define EFLASH_TIME1_RCV_T_NS           50000UL     /*!< Time RCV_T, ns */
#define EFLASH_TIME2_RW_T_NS            100UL       /*!< Time RW_T, ns */
#define EFLASH_TIME2_PROG_T_NS          8000UL      /*!< Time PROG_T, ns */
#define EFLASH_TIME2_NVS_T_NS           4000UL      /*!< Time NVS_T, ns */
#define EFLASH_TIME3_PREPROG_T_NS       2500UL      /*!< Time PREPROG_T, ns */
#define EFLASH_TIME3_CBD_T_NS           20UL        /*!< Time CBD_T, ns */
#define EFLASH_TIME3_PGS_T_NS           20000UL     /*!< Time PGS_T, ns */
#define EFLASH_TIME4_RS_T_NS            10UL        /*!< Time RS_T, ns */
#define EFLASH_TIME4_SBE_T_NS           4000000UL   /*!< Time SBE_T, ns */
#define EFLASH_TIME4_TEST_T_NS          50UL        /*!< Time TEST_T, ns */
#define EFLASH_TIME5_ERASE_T_NS         3200000UL   /*!< Time ERASE_T, ns */
#define EFLASH_TIME5_RCR_T_NS           200UL       /*!< Time RCR_T, ns */
#define EFLASH_TIME6_PREPGS_T_NS        5000UL      /*!< Time PREPGS_T, ns */

/**
 * @brief Calculates embedded flash timing parameters.
 * @param T_NS Timing parameter at 25MHz, ns.
 * @param F_HZ Actual frequency, Hz.
 * @returns The value to be stored in the corresponding register.
 */
#define EFLASH_CALC_TIME(T_NS, F_HZ)    ((uint32_t)(((uint64_t)(T_NS) * (F_HZ) + (1000000000UL - 1UL)) / 1000000000UL))

/** @brief Waits for the last EFLASH operation to complete. */
static EFLASH_Status_TypeDef EFLASH_WaitForLastOperation(uint32_t Timeout);

/**
 * @brief Initializes EFLASH registers.
 * @param CCLK_Freq CCLK clock frequency (Hz). This value is used to
 *        calculate EFLASH timing values.
 * @note  This function should be called before reconfiguration in case if
 *        the new CCLK frequency is higher than the current one.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_Init(uint32_t CCLK_Freq)
{
    EFLASH_Status_TypeDef status = EFLASH_BUSY;

    /* Wait for reset to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_RESET_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        MODIFY_REG(EFLASH->TIME0, EFLASH_TIME0_DS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME0_DS_T_NS, CCLK_Freq) << EFLASH_TIME0_DS_T_Pos) & EFLASH_TIME0_DS_T_Msk);
        MODIFY_REG(EFLASH->TIME0, EFLASH_TIME0_SCE_T,
                (EFLASH_CALC_TIME(EFLASH_TIME0_SCE_T_NS, CCLK_Freq) << EFLASH_TIME0_SCE_T_Pos) & EFLASH_TIME0_SCE_T_Msk);
        MODIFY_REG(EFLASH->TIME0, EFLASH_TIME0_AA_T,
                (EFLASH_CALC_TIME(EFLASH_TIME0_AA_T_NS, CCLK_Freq) << EFLASH_TIME0_AA_T_Pos) & EFLASH_TIME0_AA_T_Msk);
        MODIFY_REG(EFLASH->TIME1, EFLASH_TIME1_AS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME1_AS_T_NS, CCLK_Freq) << EFLASH_TIME1_AS_T_Pos) & EFLASH_TIME1_AS_T_Msk);
        MODIFY_REG(EFLASH->TIME1, EFLASH_TIME1_RHR_T,
                (EFLASH_CALC_TIME(EFLASH_TIME1_RHR_T_NS, CCLK_Freq) << EFLASH_TIME1_RHR_T_Pos) & EFLASH_TIME1_RHR_T_Msk);
        MODIFY_REG(EFLASH->TIME1, EFLASH_TIME1_RCV_T,
                (EFLASH_CALC_TIME(EFLASH_TIME1_RCV_T_NS, CCLK_Freq) << EFLASH_TIME1_RCV_T_Pos) & EFLASH_TIME1_RCV_T_Msk);
        MODIFY_REG(EFLASH->TIME2, EFLASH_TIME2_RW_T,
                (EFLASH_CALC_TIME(EFLASH_TIME2_RW_T_NS, CCLK_Freq) << EFLASH_TIME2_RW_T_Pos) & EFLASH_TIME2_RW_T_Msk);
        MODIFY_REG(EFLASH->TIME2, EFLASH_TIME2_PROG_T,
                (EFLASH_CALC_TIME(EFLASH_TIME2_PROG_T_NS, CCLK_Freq) << EFLASH_TIME2_PROG_T_Pos) & EFLASH_TIME2_PROG_T_Msk);
        MODIFY_REG(EFLASH->TIME2, EFLASH_TIME2_NVS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME2_NVS_T_NS, CCLK_Freq) << EFLASH_TIME2_NVS_T_Pos) & EFLASH_TIME2_NVS_T_Msk);
        MODIFY_REG(EFLASH->TIME3, EFLASH_TIME3_PREPROG_T,
                (EFLASH_CALC_TIME(EFLASH_TIME3_PREPROG_T_NS, CCLK_Freq) << EFLASH_TIME3_PREPROG_T_Pos) & EFLASH_TIME3_PREPROG_T_Msk);
        MODIFY_REG(EFLASH->TIME3, EFLASH_TIME3_CBD_T,
                (EFLASH_CALC_TIME(EFLASH_TIME3_CBD_T_NS, CCLK_Freq) << EFLASH_TIME3_CBD_T_Pos) & EFLASH_TIME3_CBD_T_Msk);
        MODIFY_REG(EFLASH->TIME3, EFLASH_TIME3_PGS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME3_PGS_T_NS, CCLK_Freq) << EFLASH_TIME3_PGS_T_Pos) & EFLASH_TIME3_PGS_T_Msk);
        MODIFY_REG(EFLASH->TIME4, EFLASH_TIME4_RS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME4_RS_T_NS, CCLK_Freq) << EFLASH_TIME4_RS_T_Pos) & EFLASH_TIME4_RS_T_Msk);
        MODIFY_REG(EFLASH->TIME4, EFLASH_TIME4_SBE_T,
                (EFLASH_CALC_TIME(EFLASH_TIME4_SBE_T_NS, CCLK_Freq) << EFLASH_TIME4_SBE_T_Pos) & EFLASH_TIME4_SBE_T_Msk);
        MODIFY_REG(EFLASH->TIME4, EFLASH_TIME4_TEST_T,
                (EFLASH_CALC_TIME(EFLASH_TIME4_TEST_T_NS, CCLK_Freq) << EFLASH_TIME4_TEST_T_Pos) & EFLASH_TIME4_TEST_T_Msk);
        MODIFY_REG(EFLASH->TIME5, EFLASH_TIME5_ERASE_T,
                (EFLASH_CALC_TIME(EFLASH_TIME5_ERASE_T_NS, CCLK_Freq) << EFLASH_TIME5_ERASE_T_Pos) & EFLASH_TIME5_ERASE_T_Msk);
        MODIFY_REG(EFLASH->TIME5, EFLASH_TIME5_RCR_T,
                (EFLASH_CALC_TIME(EFLASH_TIME5_RCR_T_NS, CCLK_Freq) << EFLASH_TIME5_RCR_T_Pos) & EFLASH_TIME5_RCR_T_Msk);
        MODIFY_REG(EFLASH->TIME6, EFLASH_TIME6_PREPGS_T,
                (EFLASH_CALC_TIME(EFLASH_TIME6_PREPGS_T_NS, CCLK_Freq) << EFLASH_TIME6_PREPGS_T_Pos) & EFLASH_TIME6_PREPGS_T_Msk);

        /* Run configuration command */
        EFLASH->CR = EFLASH_CR_OP_CODE_CFG | EFLASH_CR_RUN;

        /* Wait for the configuration operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_CFG_TIMEOUT);

        if (status == EFLASH_COMPLETE)
        {
            /* Check for errors */
            if (EFLASH->CFG_FLAGS & (EFLASH_CFG_FLAGS_CFG_READ_ERR0 | EFLASH_CFG_FLAGS_CFG_READ_ERR1))
            {
                status = EFLASH_ERROR_CFG;
            }
        }
    }

    return status;
}

/**
 * @brief Writes a word at the specified address.
 * @note Enable access to NVR in CRU SYSCR0 register before programming it.
 * @param Address The address to program.
 * @param Data The data to program.
 * @param Region The memory region. Can be one of EFLASH_Region_TypeDef values.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_ProgramWord(uint32_t Address, uint32_t Data, EFLASH_Region_TypeDef Region)
{
    EFLASH_Status_TypeDef status;

    /* Wait for the last operation to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_PROGRAM_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        /* Set address to write */
        EFLASH->ADDR = (uint16_t)((Address & 0x3FFFFUL) >> 2);

        /* Set data to write */
        EFLASH->WDATA = Data;

        /* Start WRITE operation */
        EFLASH->CR = ((uint32_t)Region) | EFLASH_CR_OP_CODE_WRITE | EFLASH_CR_RUN;

        /* Wait for the program operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_PROGRAM_TIMEOUT);
    }

    return status;
}

/**
 * @brief Reads a word at the specified address.
 * @note Enable access to NVR in CRU SYSCR0 register before reading it.
 * @param Address The address to read.
 * @param Data The buffer for the data to read.
 * @param Region The memory region. Can be one of EFLASH_Region_TypeDef values.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_ReadWord(uint32_t Address, uint32_t * Data, EFLASH_Region_TypeDef Region)
{
    EFLASH_Status_TypeDef status;

    /* Wait for the last operation to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_READ_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        /* Set address to read */
        EFLASH->ADDR = (uint16_t)((Address & 0x3FFFFUL) >> 2);

        /* Start READ operation */
        EFLASH->CR = ((uint32_t)Region) | EFLASH_CR_OP_CODE_READ | EFLASH_CR_RUN;

        /* Wait for the read operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_READ_TIMEOUT);
    }

    if (status == EFLASH_COMPLETE)
    {
        /* Read data from register */
        *Data = EFLASH->RDATA;
    }

    return status;
}

/**
 * @brief Erases the specified sector.
 * @note Enable access to NVR in CRU SYSCR0 register before erasing it.
 * @param Sector_Address The address of the sector to erase.
 * @param Region The memory region. Can be one of EFLASH_Region_TypeDef values.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_EraseSector(uint32_t Sector_Address, EFLASH_Region_TypeDef Region)
{
    EFLASH_Status_TypeDef status;

    /* Wait for the last operation to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        /* Set the address of the sector to erase */
        EFLASH->ADDR = (uint16_t)((Sector_Address & 0x3FFFFUL) >> 2);

        /* Start SECTOR_ERASE operation */
        EFLASH->CR = ((uint32_t)Region) | EFLASH_CR_OP_CODE_SECTOR_ERASE | EFLASH_CR_RUN;

        /* Wait for the erase operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);
    }

    return status;
}

/**
 * @brief Erases the specified block.
 * @note Enable access to NVR in CRU SYSCR0 register before erasing it.
 * @param Region The memory region. Can be one of EFLASH_Region_TypeDef values.
 * @param Block_Address The address of the block to erase.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_EraseBlock(uint32_t Block_Address, EFLASH_Region_TypeDef Region)
{
    EFLASH_Status_TypeDef status;

    /* Wait for the last operation to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        /* Set the address of the block to erase */
        EFLASH->ADDR = (uint16_t)((Block_Address & 0x3FFFFUL) >> 2);

        /* Start BLOCK_ERASE operation */
        EFLASH->CR = ((uint32_t)Region) | EFLASH_CR_OP_CODE_BLOCK_ERASE | EFLASH_CR_RUN;

        /* Wait for the erase operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);
    }

    return status;
}

/**
 * @brief Erases all sectors.
 * @note Enable access to NVR in CRU SYSCR0 register before erasing it.
 * @param Region The memory region. Can be one of EFLASH_Region_TypeDef values.
 * @returns The execution status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_EraseAll(EFLASH_Region_TypeDef Region)
{
    EFLASH_Status_TypeDef status;

    /* Wait for the last operation to complete */
    status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);

    if (status == EFLASH_COMPLETE)
    {
        /* Start CHIP_ERASE operation */
        EFLASH->CR = ((uint32_t)Region) | EFLASH_CR_OP_CODE_CHIP_ERASE | EFLASH_CR_RUN;

        /* Wait for the erase operation to complete */
        status = EFLASH_WaitForLastOperation(EFLASH_ERASE_TIMEOUT);
    }

    return status;
}

/**
 * @brief Waits for the last EFLASH operation to complete.
 * @param Timeout The timeout.
 * @returns The EFLASH status. Can be one of EFLASH_Status_TypeDef values.
 */
EFLASH_Status_TypeDef EFLASH_WaitForLastOperation(uint32_t Timeout)
{
    EFLASH_Status_TypeDef status = EFLASH_COMPLETE;

    while ((EFLASH_IsActiveFlag_BUSY() != 0UL) && (Timeout != 0UL))
    {
        Timeout--;
    }

    if (Timeout == 0UL)
    {
        status = EFLASH_TIMEOUT;
    }

    return status;
}

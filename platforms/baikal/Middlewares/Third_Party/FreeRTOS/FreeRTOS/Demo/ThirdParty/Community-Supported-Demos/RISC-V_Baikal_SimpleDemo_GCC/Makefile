# See Tools/build/Makefile_template for more information
SDK_DIR = $(realpath $(CURDIR)/../../../../../../../..)
TC_DIR = $(SDK_DIR)/Tools/toolchain/riscv32-none-elf
TC_PREFIX = $(TC_DIR)/bin/riscv32-none-elf-

OPT = 0

AS = $(TC_PREFIX)gcc
ASFLAGS += -mcmodel=$(CMODEL)
ASFLAGS += -O$(OPT)
ASFLAGS += -g$(DEBUG)
ASFLAGS += -specs=$(SPECS)
ASFLAGS += -x assembler-with-cpp

RTOS_SOURCE_DIR = $(realpath $(CURDIR)/../../../../Source)

BOARD = EVU_BA_2_5
include $(SDK_DIR)/BSP/$(BOARD)/bsp.mk

INC_DIR += -I $(RTOS_SOURCE_DIR)/include
INC_DIR += -I $(RTOS_SOURCE_DIR)/portable/ThirdParty/GCC/RISC-V/Baikal
INC_DIR += -I $(RTOS_SOURCE_DIR)/portable/ThirdParty/GCC/RISC-V/Baikal/chip_specific_extensions/RISCV_no_extensions

CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_cru.c
CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_uart.c
CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_tim.c
CSRC += $(RTOS_SOURCE_DIR)/list.c
CSRC += $(RTOS_SOURCE_DIR)/queue.c
CSRC += $(RTOS_SOURCE_DIR)/tasks.c
CSRC += $(RTOS_SOURCE_DIR)/timers.c
CSRC += $(RTOS_SOURCE_DIR)/portable/MemMang/heap_4.c
CSRC += $(RTOS_SOURCE_DIR)/portable/ThirdParty/GCC/RISC-V/Baikal/port.c

ASRC += startup.S
ASRC += $(RTOS_SOURCE_DIR)/portable/ThirdParty/GCC/RISC-V/Baikal/portASM.S
ASRC_EXCLUDE += $(SDK_DIR)/Tools/build/platform/$(MCU_MODEL)/startup_c0_c1.S

# Include the main makefile
include $(SDK_DIR)/Tools/build/common.mk

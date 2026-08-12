# Platform-specific configuration for Baikal
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR riscv)

find_program(RISCV_GCC_SYSTEM NAMES riscv32-none-elf-gcc riscv64-unknown-elf-gcc riscv64-elf-gcc riscv-none-elf-gcc)

if(RISCV_GCC_SYSTEM)
    get_filename_component(TOOLCHAIN_BIN_DIR "${RISCV_GCC_SYSTEM}" DIRECTORY)
    get_filename_component(COMPILER_NAME "${RISCV_GCC_SYSTEM}" NAME)
    string(REGEX REPLACE "gcc$" "" TOOLCHAIN_PREFIX_NAME "${COMPILER_NAME}")
    set(TOOLCHAIN_PREFIX "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX_NAME}")
else()
    message(FATAL_ERROR "RISC-V GCC toolchain (riscv32-none-elf-gcc) not found in PATH. Please install riscv-none-elf-gcc or riscv32-unknown-elf-gcc.")
endif()

set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_AR           "${TOOLCHAIN_PREFIX}ar")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE         "${TOOLCHAIN_PREFIX}size")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

add_compile_options(
    -march=rv32imac_zicsr
    -mabi=ilp32
    -mcmodel=medlow
)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_definitions(
    CORE_BR310S14
    BE_U1000
    configCPU_CLOCK_HZ=80000000
    configMTIME_BASE_ADDRESS=0
    configMTIMECMP_BASE_ADDRESS=0
)

set(LINKER_SCRIPT "${PLATFORM_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Demo/ThirdParty/Community-Supported-Demos/RISC-V_Baikal_SimpleDemo_GCC/linker.ld")
add_link_options(-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medlow -nostartfiles)
add_link_options("-T${LINKER_SCRIPT}")
add_link_options(-Wl,--gc-sections,--print-memory-usage)
add_link_options(-specs=nosys.specs -specs=nano.specs)

set(BAIKAL_DIR "${PLATFORM_DIR}")

# Default memory sizes
if(NOT MEMORY)
    set(MEMORY "1MB")
endif()

# Convert memory to bytes
if(MEMORY STREQUAL "1MB")
    set(FLASH_SIZE "0x100000")
    set(RAM_SIZE "0x40000")
elseif(MEMORY STREQUAL "2MB")
    set(FLASH_SIZE "0x200000")
    set(RAM_SIZE "0x80000")
elseif(MEMORY STREQUAL "4MB")
    set(FLASH_SIZE "0x400000")
    set(RAM_SIZE "0x100000")
elseif(MEMORY STREQUAL "8MB")
    set(FLASH_SIZE "0x800000")
    set(RAM_SIZE "0x100000")
elseif(MEMORY STREQUAL "16MB")
    set(FLASH_SIZE "0x1000000")
    set(RAM_SIZE "0x100000")
else()
    set(FLASH_SIZE "0x100000")
    set(RAM_SIZE "0x40000")
endif()

# Platform includes
set(PLATFORM_INC
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/include"
    "${PLATFORM_DIR}/freertos/include"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/chip_specific_extensions/RISCV_no_extensions"
    "${BAIKAL_DIR}/Drivers/BASIS/Core/Include"
    "${BAIKAL_DIR}/Drivers/BASIS/Device/BMCU/Include"
    "${BAIKAL_DIR}/Drivers/HAL/Inc"
    "${BAIKAL_DIR}/BSP/Components"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/include"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/hw"
)

# Platform sources
set(PLATFORM_SRC
    "${PLATFORM_DIR}/freertos/croutine.c"
    "${PLATFORM_DIR}/freertos/event_groups.c"
    "${PLATFORM_DIR}/freertos/list.c"
    "${PLATFORM_DIR}/freertos/queue.c"
    "${PLATFORM_DIR}/freertos/stream_buffer.c"
    "${PLATFORM_DIR}/freertos/tasks.c"
    "${PLATFORM_DIR}/freertos/timers.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/port.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/portASM.S"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Demo/ThirdParty/Community-Supported-Demos/RISC-V_Baikal_SimpleDemo_GCC/startup.S"
    "${PLATFORM_DIR}/freertos/portable/MemMang/heap_4.c"
    "${PLATFORM_DIR}/hal_gpio.c"
    "${PLATFORM_DIR}/split.c"
    "${PLATFORM_DIR}/ws2812.c"
    "${PLATFORM_DIR}/app_usb_hid.c"
    "${DMK_ROOT}/dmk_core/src/usb_descriptors.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_gpio.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_cru.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/tusb.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/common/tusb_fifo.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/device/usbd_control.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/device/usbd.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/class/hid/hid_device.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/portable/bmcu/dcd_bmcu.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/hw/bsp/bmcu/family.c"
)

# Platform libraries
set(PLATFORM_LIBS "")

# Platform-specific post-build command (placeholder)
function(platform_post_build TARGET_NAME)
    # TODO: Add binary generation for Baikal
endfunction()

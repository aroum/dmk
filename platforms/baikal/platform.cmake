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

# Default memory sizes (eFlash is 256KB total on BE-U1000)
# Bootloader uses first 64KB (0xA0000000), App gets remaining 192KB (0xA0010000)
option(BOOTLOADER "Build for TinyUF2 bootloader" ON)
if(BOOTLOADER)
    set(LINKER_SCRIPT "${PLATFORM_DIR}/linker_uf2.ld")
    set(FLASH_SIZE "0x30000") # 192 KB
    set(FLASH_BASE_ADDR "0xA0010000")
else()
    set(LINKER_SCRIPT "${PLATFORM_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Demo/ThirdParty/Community-Supported-Demos/RISC-V_Baikal_SimpleDemo_GCC/linker.ld")
    set(FLASH_SIZE "0x40000") # 256 KB
    set(FLASH_BASE_ADDR "0xA0000000")
endif()
set(RAM_SIZE "0x8000") # 32 KB SRAM

add_link_options(-march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medlow -nostartfiles)
add_link_options("-T${LINKER_SCRIPT}")
add_link_options(-Wl,--gc-sections,--print-memory-usage)
add_link_options(-specs=nosys.specs -specs=nano.specs)

set(BAIKAL_DIR "${PLATFORM_DIR}")

# Platform includes
set(PLATFORM_INC
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/include"
    "${DMK_ROOT}/lib/freertos/include"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/chip_specific_extensions/RISCV_no_extensions"
    "${BAIKAL_DIR}/Drivers/BASIS/Core/Include"
    "${BAIKAL_DIR}/Drivers/BASIS/Device/BMCU/Include"
    "${BAIKAL_DIR}/Drivers/HAL/Inc"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/hw"
)

# Platform sources
set(PLATFORM_SRC
    ${FREERTOS_COMMON_SRC}
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/port.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RISC-V/Baikal/portASM.S"
    "${BAIKAL_DIR}/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Demo/ThirdParty/Community-Supported-Demos/RISC-V_Baikal_SimpleDemo_GCC/startup.S"
    "${PLATFORM_DIR}/ws2812.c"
    "${PLATFORM_DIR}/split.c"
    "${DMK_ROOT}/dmk_core/drivers/app_usb_tinyusb.c"
    "${DMK_ROOT}/dmk_core/drivers/usb_descriptors.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_gpio.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_cru.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_adc.c"
    "${BAIKAL_DIR}/Drivers/HAL/Src/bmcu_uart.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/tusb.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/common/tusb_fifo.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/device/usbd_control.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/device/usbd.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/class/hid/hid_device.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/src/portable/bmcu/dcd_bmcu.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/hw/bsp/bmcu/family.c"
    "${BAIKAL_DIR}/Middlewares/Third_Party/TinyUSB/hw/bsp/board.c"
)

# Platform libraries
set(PLATFORM_LIBS "")

# Platform-specific post-build command: generates hex, bin and uf2
function(platform_post_build TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMAND uv run --project ${DMK_ROOT}/tools/uf2 python ${DMK_ROOT}/tools/uf2/uf2conv.py -c -b ${FLASH_BASE_ADDR} -f 0x6ecb67a8 -o ${CMAKE_BINARY_DIR}/${TARGET_NAME}.uf2 ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMENT "Generating hex, bin and uf2 for Baikal BE-U1000"
    )
endfunction()

# Platform configuration for STM32F072 (ARM Cortex-M0)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(ARM_GCC_SYSTEM NAMES arm-none-eabi-gcc)
if(NOT ARM_GCC_SYSTEM)
    message(FATAL_ERROR "ARM GCC toolchain (arm-none-eabi-gcc) not found in PATH.")
endif()

get_filename_component(TOOLCHAIN_BIN_DIR "${ARM_GCC_SYSTEM}" DIRECTORY)
set(TOOLCHAIN_PREFIX "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-")

set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_AR           "${TOOLCHAIN_PREFIX}ar")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE         "${TOOLCHAIN_PREFIX}size")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

add_compile_options(
    -mcpu=cortex-m0
    -mthumb
    -mfloat-abi=soft
    -Os
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-builtin
)

add_compile_definitions(
    STM32F072xB
    USE_FULL_LL_DRIVER
    CFG_TUSB_MCU=OPT_MCU_STM32F0
)

# Flash layout: 128 KB starting at 0x08000000
set(LINKER_SCRIPT "${PLATFORM_DIR}/stm32f072xb.ld")
set(FLASH_BASE_ADDR "0x08000000")

set(DEP_DIR "${PLATFORM_DIR}/dep")

# TinyUSB directory resolution
if(EXISTS "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb")
elseif(EXISTS "${DMK_ROOT}/platforms/milandr/dep/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/platforms/milandr/dep/tinyusb")
else()
    set(TINYUSB_DIR "${DMK_ROOT}/platforms/baikal/Middlewares/Third_Party/TinyUSB")
endif()

set(PLATFORM_INC
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/include"
    "${DEP_DIR}/cmsis_core/Include"
    "${DEP_DIR}/cmsis_device_f0/Include"
    "${DEP_DIR}/stm32f0xx_hal_driver/Inc"
    "${DMK_ROOT}/lib/freertos/include"
    "${PLATFORM_DIR}/freertos_port"
    "${TINYUSB_DIR}/src"
    "${TINYUSB_DIR}/hw"
)

set(PLATFORM_SRC
    ${FREERTOS_COMMON_SRC}
    "${PLATFORM_DIR}/freertos_port/port.c"
    "${PLATFORM_DIR}/platform.c"
    "${PLATFORM_DIR}/syscalls.c"
    "${DEP_DIR}/cmsis_device_f0/Source/Templates/gcc/startup_stm32f072xb.s"
    "${DEP_DIR}/cmsis_device_f0/Source/Templates/system_stm32f0xx.c"
    # STM32F0 Low-Layer Drivers
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_rcc.c"
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_gpio.c"
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_adc.c"
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_pwr.c"
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_utils.c"
    "${DEP_DIR}/stm32f0xx_hal_driver/Src/stm32f0xx_ll_crs.c"
    # TinyUSB stack
    "${DMK_ROOT}/dmk_core/drivers/app_usb_tinyusb.c"
    "${DMK_ROOT}/dmk_core/drivers/usb_descriptors.c"
    "${TINYUSB_DIR}/src/tusb.c"
    "${TINYUSB_DIR}/src/common/tusb_fifo.c"
    "${TINYUSB_DIR}/src/device/usbd.c"
    "${TINYUSB_DIR}/src/device/usbd_control.c"
    "${TINYUSB_DIR}/src/class/hid/hid_device.c"
    "${TINYUSB_DIR}/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c"
)

set(PLATFORM_LIBS
    -mcpu=cortex-m0
    -mthumb
    -mfloat-abi=soft
    --specs=nosys.specs
    --specs=nano.specs
    -T${LINKER_SCRIPT}
)

function(platform_post_build TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
        COMMENT "Generating .bin and .hex for ${TARGET_NAME}"
    )

    add_custom_target(flash
        COMMAND dfu-util -a 0 -s 0x08000000:leave -D ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        DEPENDS ${TARGET_NAME}
        COMMENT "Flashing ${TARGET_NAME}.bin via dfu-util..."
    )
endfunction()

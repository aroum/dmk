# Platform configuration for STM32L072 (ARM Cortex-M0+)
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
    -mcpu=cortex-m0plus
    -mthumb
    -mfloat-abi=soft
    -Os
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-builtin
)

add_compile_definitions(
    STM32L072xx
    USE_FULL_LL_DRIVER
    HSE_VALUE=8000000
    HSI_VALUE=16000000
    CFG_TUSB_MCU=OPT_MCU_STM32L0
    USER_VECT_TAB_ADDRESS
    VECT_TAB_OFFSET=0x10000U
)

# Flash layout: Bank 2 starts at 0x08010000 (64 KB)
set(LINKER_SCRIPT "${PLATFORM_DIR}/stm32l072_bank2.ld")
set(FLASH_BASE_ADDR "0x08010000")

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
    "${DEP_DIR}/cmsis_device_l0/Include"
    "${DEP_DIR}/stm32l0xx_hal_driver/Inc"
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
    "${DEP_DIR}/cmsis_device_l0/Source/Templates/gcc/startup_stm32l072xx.s"
    "${DEP_DIR}/cmsis_device_l0/Source/Templates/system_stm32l0xx.c"
    # STM32L0 Low-Layer Drivers
    "${DEP_DIR}/stm32l0xx_hal_driver/Src/stm32l0xx_ll_rcc.c"
    "${DEP_DIR}/stm32l0xx_hal_driver/Src/stm32l0xx_ll_gpio.c"
    "${DEP_DIR}/stm32l0xx_hal_driver/Src/stm32l0xx_ll_adc.c"
    "${DEP_DIR}/stm32l0xx_hal_driver/Src/stm32l0xx_ll_pwr.c"
    "${DEP_DIR}/stm32l0xx_hal_driver/Src/stm32l0xx_ll_utils.c"
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
    -mcpu=cortex-m0plus
    -mthumb
    -mfloat-abi=soft
    --specs=nosys.specs
    --specs=nano.specs
    -T${LINKER_SCRIPT}
)

# Option to compile for Classic Type-S (AHHS01, version A8.xx)
option(HHKB_TYPE_S "Build firmware container for HHKB Classic Type-S (AHHS01)" OFF)

option(HHKB_RAPID_TRIGGER "Enable HHKB Rapid Trigger" OFF)
if(DEFINED DEFINE)
    string(REPLACE "," ";" HHKB_BUILD_DEFINES "${DEFINE}")
    set(HHKB_RAPID_TRIGGER OFF)
    set(HHKB_TYPE_S OFF)
    if("HHKB_RAPID_TRIGGER" IN_LIST HHKB_BUILD_DEFINES)
        set(HHKB_RAPID_TRIGGER ON)
    endif()
    if("HHKB_TYPE_S" IN_LIST HHKB_BUILD_DEFINES)
        set(HHKB_TYPE_S ON)
    endif()
endif()
if(HHKB_RAPID_TRIGGER AND NOT DEFINED DEFINE)
    add_compile_definitions(HHKB_RAPID_TRIGGER=1)
endif()

# Post-build packaging into .bin and HHKB .hfb container
function(platform_post_build TARGET_NAME)
    if(HHKB_TYPE_S)
        set(HFB_MODEL_ARG "--model" "AHHS01" "--version" "A8.50")
        set(MODEL_COMMENT "Type-S (AHHS01)")
    else()
        set(HFB_MODEL_ARG "--model" "AHHX01" "--version" "A4.50")
        set(MODEL_COMMENT "Classic (AHHX01)")
    endif()

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMAND python3 ${DMK_ROOT}/tools/hhkb/make_hfb.py ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin ${CMAKE_BINARY_DIR}/${TARGET_NAME}.hfb ${HFB_MODEL_ARG}
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
        COMMENT "Generating binary and HHKB ${MODEL_COMMENT} .hfb update package for ${TARGET_NAME}"
    )

    # Custom target to flash directly via USB HID: cmake --build <dir> --target flash
    add_custom_target(flash
        COMMAND uv run --with hidapi python3 ${DMK_ROOT}/tools/hhkb/hhkb_flasher.py ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        DEPENDS ${TARGET_NAME}
        COMMENT "Flashing ${TARGET_NAME}.bin to HHKB via USB HID..."
    )
endfunction()

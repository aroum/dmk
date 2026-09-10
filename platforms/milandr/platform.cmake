# Platform-specific configuration for Milandr
set(DEP_DIR "${PLATFORM_DIR}/dep")
set(COMMON_DIR "${DEP_DIR}/common")
set(CMAKE_DIR "${COMMON_DIR}/cmake")

# TinyUSB root
if(EXISTS "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb")
elseif(EXISTS "${DMK_ROOT}/lib/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/lib/tinyusb")
else()
    message(FATAL_ERROR "TinyUSB not found")
endif()

# Default memory sizes
if(NOT MEMORY)
    set(MEMORY "256KB")
endif()

# Select linker script based on BOOTLOADER option
option(BOOTLOADER "Build for DFU bootloader" OFF)
if(BOOTLOADER)
    set(LINKER_SCRIPT "${PLATFORM_DIR}/dep/gcc/MDR32F9Q2I_dfu.ld")
    set(FLASH_SIZE "0x1E000")
else()
    set(LINKER_SCRIPT "${PLATFORM_DIR}/dep/gcc/MDR32F9Q2I.ld")
    set(FLASH_SIZE "0x40000")
endif()
set(RAM_SIZE "0x10000")

# Include toolchain and common cmake
include("${CMAKE_DIR}/gcc-milandr.cmake")

# Add milandr SDK
if(NOT TARGET milandr_sdk)
    add_subdirectory(
        "${COMMON_DIR}/Drivers"
        "${CMAKE_BINARY_DIR}/milandr_sdk"
    )
endif()

# Platform includes
set(PLATFORM_INC
    "${COMMON_DIR}/inc"
    "${DMK_ROOT}/lib/freertos/include"
    "${DMK_ROOT}/lib/freertos/portable/GCC/ARM_CM3"
    "${PLATFORM_DIR}/sdk"
    "${TINYUSB_DIR}/src"
    "${TINYUSB_DIR}/hw"
)

# Platform sources
set(PLATFORM_SRC
    "${COMMON_DIR}/src/clk.c"
    ${FREERTOS_COMMON_SRC}
    "${DMK_ROOT}/lib/freertos/portable/GCC/ARM_CM3/port.c"
    "${COMMON_DIR}/Drivers/SPL/src/MDR32FxQI_adc.c"
    "${COMMON_DIR}/Drivers/SPL/src/MDR32FxQI_i2c.c"
    # Unified USB driver & descriptors
    "${DMK_ROOT}/dmk_core/drivers/app_usb_tinyusb.c"
    "${DMK_ROOT}/dmk_core/drivers/usb_descriptors.c"
    # Milandr TinyUSB DCD
    "${PLATFORM_DIR}/dcd_milandr.c"
    # TinyUSB stack
    "${TINYUSB_DIR}/src/tusb.c"
    "${TINYUSB_DIR}/src/common/tusb_fifo.c"
    "${TINYUSB_DIR}/src/device/usbd_control.c"
    "${TINYUSB_DIR}/src/device/usbd.c"
    "${TINYUSB_DIR}/src/class/hid/hid_device.c"
    "${TINYUSB_DIR}/src/class/midi/midi_device.c"
    # Peripherals
    "${PLATFORM_DIR}/ws2812_ssp.c"
    "${PLATFORM_DIR}/split.c"
)

# Platform libraries
set(PLATFORM_LIBS milandr_sdk)

# Platform-specific post-build command
function(platform_post_build TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMENT "Building ${TARGET_NAME}.hex and ${TARGET_NAME}.bin"
    )
endfunction()

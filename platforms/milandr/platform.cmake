# Platform-specific configuration for Milandr
set(DEP_DIR "${PLATFORM_DIR}/dep")
set(COMMON_DIR "${DEP_DIR}/common")
set(CMAKE_DIR "${COMMON_DIR}/cmake")

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
    "${PLATFORM_DIR}/sdk/USB_Library"
)

# Platform sources
set(PLATFORM_SRC
    "${COMMON_DIR}/src/clk.c"
    "${COMMON_DIR}/src/board_usb.c"
    ${FREERTOS_COMMON_SRC}
    "${DMK_ROOT}/lib/freertos/portable/GCC/ARM_CM3/port.c"
    "${COMMON_DIR}/Drivers/SPL/src/MDR32FxQI_adc.c"
    "${COMMON_DIR}/Drivers/SPL/src/MDR32FxQI_i2c.c"
    "${PLATFORM_DIR}/app_usb_hid.c"
    "${PLATFORM_DIR}/ws2812_ssp.c"
    "${PLATFORM_DIR}/split.c"
)

# Platform libraries
set(PLATFORM_LIBS milandr_sdk)

# Platform-specific post-build command
function(platform_post_build TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
        COMMENT "Building ${TARGET_NAME}.hex and ${TARGET_NAME}.bin"
    )
endfunction()

# Platform-specific configuration for nRF52840

# Set up ARM cross-compiler toolchain (must be before project())
include("${CMAKE_CURRENT_LIST_DIR}/cmake/gcc-nrf52840.cmake")

# nRF5 SDK root
set(NRF5_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/sdk")

# TinyUSB root
if(EXISTS "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/platforms/raspberrypi/pico-sdk/lib/tinyusb")
elseif(EXISTS "${DMK_ROOT}/lib/tinyusb/src/tusb.h")
    set(TINYUSB_DIR "${DMK_ROOT}/lib/tinyusb")
else()
    message(FATAL_ERROR "TinyUSB not found")
endif()

# Linker script configuration
set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/nrf52840_uf2.ld")
add_link_options("-T${LINKER_SCRIPT}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L\"${NRF5_SDK_DIR}/modules/nrfx/mdk\"" CACHE INTERNAL "")

# Default memory sizes
if(NOT MEMORY)
    set(MEMORY "512KB")
endif()

# Convert memory to bytes for linker scripts
if(MEMORY STREQUAL "256KB")
    set(FLASH_SIZE "0x40000")
    set(RAM_SIZE "0x10000")
elseif(MEMORY STREQUAL "512KB")
    set(FLASH_SIZE "0x80000")
    set(RAM_SIZE "0x20000")
elseif(MEMORY STREQUAL "1MB")
    set(FLASH_SIZE "0x100000")
    set(RAM_SIZE "0x40000")
endif()


# Platform includes
set(PLATFORM_INC
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/include"
    "${DMK_ROOT}/lib/freertos/include"
    "${DMK_ROOT}/lib/freertos/portable/GCC/ARM_CM4F"
    # nRF5 SDK – CMSIS / MDK (nrf52840.h, nrf.h, __NVIC_PRIO_BITS, etc.)
    "${NRF5_SDK_DIR}/modules/nrfx/mdk"
    # nrfx core and HAL (nrfx.h, nrf_gpio.h, nrfx_clock.h)
    "${NRF5_SDK_DIR}/modules/nrfx"
    "${NRF5_SDK_DIR}/modules/nrfx/hal"
    "${NRF5_SDK_DIR}/modules/nrfx/drivers/include"
    "${NRF5_SDK_DIR}/modules/nrfx/drivers/src"
    # Legacy integration layer (nrf_drv_clock.h, nrf_drv_power.h)
    "${NRF5_SDK_DIR}/integration/nrfx"
    "${NRF5_SDK_DIR}/integration/nrfx/legacy"
    # TinyUSB
    "${TINYUSB_DIR}/src"
    "${TINYUSB_DIR}/hw"
    # Common utilities (app_error.h, app_util.h)
    "${NRF5_SDK_DIR}/components/libraries/util"
    "${NRF5_SDK_DIR}/components/libraries/delay"
    "${NRF5_SDK_DIR}/components/libraries/log"
    "${NRF5_SDK_DIR}/components/libraries/log/src"
    "${NRF5_SDK_DIR}/components/libraries/timer"
    # sdk_config.h for nRF52840
    "${NRF5_SDK_DIR}/config/nrf52840/config"
    # nRF SDK toolchain glue
    "${NRF5_SDK_DIR}/components/toolchain/cmsis/include"
    # nrf_error.h and related (no softdevice variant)
    "${NRF5_SDK_DIR}/components/drivers_nrf/nrf_soc_nosd"
    # nrf_section.h etc.
    "${NRF5_SDK_DIR}/components/libraries/experimental_section_vars"
    # nrf_atomic.h
    "${NRF5_SDK_DIR}/components/libraries/atomic"
    # nrf_strerror.h
    "${NRF5_SDK_DIR}/components/libraries/strerror"
)

# Platform sources
set(PLATFORM_SRC
    # nRF5 MDK startup and system init
    "${NRF5_SDK_DIR}/modules/nrfx/mdk/gcc_startup_nrf52840.S"
    "${NRF5_SDK_DIR}/modules/nrfx/mdk/system_nrf52840.c"
    ${FREERTOS_COMMON_SRC}
    "${DMK_ROOT}/lib/freertos/portable/GCC/ARM_CM4F/port.c"
    # Unified USB driver & descriptors
    "${DMK_ROOT}/dmk_core/drivers/app_usb_tinyusb.c"
    "${DMK_ROOT}/dmk_core/drivers/usb_descriptors.c"
    # TinyUSB stack
    "${TINYUSB_DIR}/src/tusb.c"
    "${TINYUSB_DIR}/src/common/tusb_fifo.c"
    "${TINYUSB_DIR}/src/device/usbd_control.c"
    "${TINYUSB_DIR}/src/device/usbd.c"
    "${TINYUSB_DIR}/src/class/hid/hid_device.c"
    "${TINYUSB_DIR}/src/class/midi/midi_device.c"
    "${TINYUSB_DIR}/src/portable/nordic/nrf5x/dcd_nrf5x.c"
    # nRF5 SDK – clock & power drivers
    "${NRF5_SDK_DIR}/modules/nrfx/drivers/src/nrfx_clock.c"
    "${NRF5_SDK_DIR}/modules/nrfx/drivers/src/nrfx_power.c"
    "${NRF5_SDK_DIR}/integration/nrfx/legacy/nrf_drv_clock.c"
    "${NRF5_SDK_DIR}/integration/nrfx/legacy/nrf_drv_power.c"
    # nRF5 SDK – atomic operations
    "${NRF5_SDK_DIR}/components/libraries/atomic/nrf_atomic.c"
    "${NRF5_SDK_DIR}/modules/nrfx/soc/nrfx_atomic.c"
    # nRF5 SDK – error handling
    "${NRF5_SDK_DIR}/components/libraries/util/app_error.c"
    "${NRF5_SDK_DIR}/components/libraries/util/app_error_weak.c"
    "${NRF5_SDK_DIR}/components/libraries/util/app_util_platform.c"
    "${NRF5_SDK_DIR}/components/libraries/strerror/nrf_strerror.c"
    # nRF5 SDK – SPIM driver
    "${NRF5_SDK_DIR}/modules/nrfx/drivers/src/nrfx_spim.c"
    # WS2812 SPI driver
    "${PLATFORM_DIR}/ws2812_spi.c"
    "${PLATFORM_DIR}/split.c"
)

# nRF52840 chip-specific compile definitions
set(PLATFORM_DEFS
    NRF52840_XXAA
    BOARD_PCA10056
    CONFIG_GPIO_AS_PINRESET
    FLOAT_ABI_HARD
)

# Platform libraries
set(PLATFORM_LIBS
)

# Platform-specific post-build command
function(platform_post_build TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMAND uv run --project ${DMK_ROOT}/tools/uf2 python ${DMK_ROOT}/tools/uf2/uf2conv.py -c -b 0x26000 -f 0xada52840 -o ${CMAKE_BINARY_DIR}/${TARGET_NAME}.uf2 ${CMAKE_BINARY_DIR}/${TARGET_NAME}.bin
        COMMENT "Generating hex, bin and uf2 for nRF52840"
    )
endfunction()

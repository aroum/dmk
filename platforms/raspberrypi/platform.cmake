# Platform-specific configuration for Raspberry Pi (RP2040/RP2350)

# Set Pico SDK path if not defined
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../examples/raspberrypi/pico-sdk" CACHE PATH "Path to the Raspberry Pi Pico SDK")
endif()
set(PICOTOOL_FORCE_FETCH_FROM_GIT ON CACHE BOOL "Build matching picotool from source")

# Check if SDK path exists
if(NOT EXISTS "${PICO_SDK_PATH}")
    message(FATAL_ERROR "Pico SDK not found at ${PICO_SDK_PATH}. Please make sure it is cloned there.")
endif()

# Include Pico SDK init file
include("${PICO_SDK_PATH}/pico_sdk_init.cmake")

# Default memory sizes
if(NOT MEMORY)
    if(MCU STREQUAL "rp2040")
        set(MEMORY "2MB")
    elseif(MCU STREQUAL "rp2350")
        set(MEMORY "4MB")
    endif()
endif()

# Convert memory to bytes for linker scripts
if(MEMORY STREQUAL "2MB")
    set(FLASH_SIZE "0x200000")
    set(RAM_SIZE "0x40000")
elseif(MEMORY STREQUAL "4MB")
    set(FLASH_SIZE "0x400000")
    set(RAM_SIZE "0x80000")
elseif(MEMORY STREQUAL "8MB")
    set(FLASH_SIZE "0x800000")
    set(RAM_SIZE "0x80000")
elseif(MEMORY STREQUAL "16MB")
    set(FLASH_SIZE "0x1000000")
    set(RAM_SIZE "0x80000")
endif()

# Select FreeRTOS port based on MCU
if(MCU STREQUAL "rp2040")
    set(FREERTOS_PORT_DIR "${PLATFORM_DIR}/freertos/portable/GCC/ARM_CM0")
    set(FREERTOS_PORT_SRC "${FREERTOS_PORT_DIR}/port.c")
elseif(MCU STREQUAL "rp2350")
    set(FREERTOS_PORT_DIR "${DMK_ROOT}/platforms/baikal/Middlewares/Third_Party/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ/non_secure")
    set(FREERTOS_PORT_SRC
        "${FREERTOS_PORT_DIR}/port.c"
        "${FREERTOS_PORT_DIR}/portasm.c"
    )
endif()

# Platform includes
set(PLATFORM_INC
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/include"
    "${PLATFORM_DIR}/freertos/include"
    "${FREERTOS_PORT_DIR}"
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
    ${FREERTOS_PORT_SRC}
    "${PLATFORM_DIR}/freertos/portable/MemMang/heap_4.c"
    "${PLATFORM_DIR}/hal_gpio.c"
    "${PLATFORM_DIR}/app_usb_hid.c"
    "${PLATFORM_DIR}/usb_descriptors.c"
    "${PLATFORM_DIR}/pio/WS2812.cpp"
    "${PLATFORM_DIR}/split.c"
    "${PLATFORM_DIR}/midi_jack.c"
)

# Platform libraries
set(PLATFORM_LIBS
    pico_stdlib
    hardware_exception
    hardware_clocks
    pico_multicore
    tinyusb_device
    tinyusb_board
    hardware_pio
    hardware_flash
    hardware_uart
)

# Platform-specific post-build command
function(platform_post_build TARGET_NAME)
    pico_add_extra_outputs(${TARGET_NAME})
    pico_generate_pio_header(${TARGET_NAME} "${PLATFORM_DIR}/pio/WS2812.pio")
    pico_generate_pio_header(${TARGET_NAME} "${PLATFORM_DIR}/pio/split.pio")
endfunction()

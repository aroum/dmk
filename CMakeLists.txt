cmake_minimum_required(VERSION 3.20)

# Default options
set(KEYBOARD "corne" CACHE STRING "Keyboard name")
set(SIDE "" CACHE STRING "Split keyboard side (left/right)")
set(KEYMAP "default" CACHE STRING "Keymap name")
set(LAYOUT "DEFAULT" CACHE STRING "Physical layout name")
set(MEMORY "" CACHE STRING "Memory configuration (e.g., 2MB, 16MB)")
set(DMK_MODULES "" CACHE STRING "Semicolon-separated list of external module directories")
set(KEYBOARD_DIR "" CACHE PATH "Optional path to external keyboard directory containing config.h")
option(CUSTOM_MATRIX "Enable custom matrix scanning driver" OFF)
option(VIAL "Enable Vial support" OFF)

# Resolve actual keyboard directory to detect default MCU from config.h
if(NOT KEYBOARD_DIR)
    set(ACTUAL_KBD_DIR "${CMAKE_CURRENT_LIST_DIR}/keyboards/${KEYBOARD}")
else()
    set(ACTUAL_KBD_DIR "${KEYBOARD_DIR}")
endif()

# Detect default MCU from config.h if not explicitly specified
set(DEFAULT_MCU_VALUE "milandr")
if(EXISTS "${ACTUAL_KBD_DIR}/config.h")
    file(READ "${ACTUAL_KBD_DIR}/config.h" _KBD_CFG)
    if(_KBD_CFG MATCHES "#[ \t]*define[ \t]+(DEFAULT_MCU|MCU_DEFAULT|MCU)[ \t]+[\"']?([a-zA-Z0-9_]+)[\"']?")
        string(TOLOWER "${CMAKE_MATCH_2}" DEFAULT_MCU_VALUE)
    elseif(_KBD_CFG MATCHES "#[ \t]*define[ \t]+MCU_(RP2040|RP2350|NRF52840|MILANDR|BAIKAL)([ \t\r\n]|$)")
        string(TOLOWER "${CMAKE_MATCH_1}" DEFAULT_MCU_VALUE)
    endif()
endif()

set(MCU "${DEFAULT_MCU_VALUE}" CACHE STRING "Target MCU (milandr/rp2040/rp2350/nrf52840/baikal)")

# Add global compile definitions from DEFINE variable
if(DEFINE)
    string(REPLACE "," ";" DEFINE_LIST "${DEFINE}")
    foreach(DEF ${DEFINE_LIST})
        string(STRIP "${DEF}" STRIPPED_DEF)
        if(NOT STRIPPED_DEF STREQUAL "")
            add_compile_definitions("${STRIPPED_DEF}")
        endif()
    endforeach()
endif()
add_compile_definitions("MCU_${MCU}")



# Path setup
get_filename_component(DMK_ROOT "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

# Map MCU to platform directory
if(MCU STREQUAL "rp2040" OR MCU STREQUAL "rp2350")
    set(PLATFORM_DIR "${DMK_ROOT}/platforms/raspberrypi")
else()
    set(PLATFORM_DIR "${DMK_ROOT}/platforms/${MCU}")
endif()

# Validate platform
if(NOT EXISTS "${PLATFORM_DIR}")
    message(FATAL_ERROR "Platform ${MCU} not found. Available platforms: milandr, rp2040, rp2350, nrf52840, baikal")
endif()

set(CMAKE_PROJECT_NAME dmk_firmware)

# Common FreeRTOS kernel source files
set(FREERTOS_COMMON_SRC
    "${DMK_ROOT}/lib/freertos/croutine.c"
    "${DMK_ROOT}/lib/freertos/event_groups.c"
    "${DMK_ROOT}/lib/freertos/list.c"
    "${DMK_ROOT}/lib/freertos/queue.c"
    "${DMK_ROOT}/lib/freertos/stream_buffer.c"
    "${DMK_ROOT}/lib/freertos/tasks.c"
    "${DMK_ROOT}/lib/freertos/timers.c"
    "${DMK_ROOT}/lib/freertos/heap_4.c"
)

# Platform-specific CMake configuration
include("${PLATFORM_DIR}/platform.cmake" OPTIONAL)

project(${CMAKE_PROJECT_NAME} ASM C CXX)

if(COMMAND pico_sdk_init)
    pico_sdk_init()
endif()

# Include core targets compilation
include(dmk_core/CMakeLists.txt)

cmake_minimum_required(VERSION 3.20)

# Default options
set(KEYBOARD "corne" CACHE STRING "Keyboard name")
set(SIDE "" CACHE STRING "Split keyboard side (left/right)")
set(KEYMAP "default" CACHE STRING "Keymap name")
set(LAYOUT "DEFAULT" CACHE STRING "Physical layout name")
set(MCU "milandr" CACHE STRING "Target MCU (milandr/rp2040/rp2350/nrf52840/baikal)")
set(MEMORY "" CACHE STRING "Memory configuration (e.g., 2MB, 16MB)")
option(VIAL "Enable Vial support" OFF)

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

# Platform-specific CMake configuration
include("${PLATFORM_DIR}/platform.cmake" OPTIONAL)

project(${CMAKE_PROJECT_NAME} ASM C CXX)

if(COMMAND pico_sdk_init)
    pico_sdk_init()
endif()

# Include core targets compilation
include(dmk_core/CMakeLists.txt)

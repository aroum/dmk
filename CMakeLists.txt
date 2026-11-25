cmake_minimum_required(VERSION 3.20)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/dmk_paths.cmake")

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
if(NOT KEYBOARD_DIR AND EXISTS "${KEYBOARD}")
    set(ACTUAL_KBD_DIR "${KEYBOARD}")
endif()
get_filename_component(ACTUAL_KBD_DIR "${ACTUAL_KBD_DIR}" ABSOLUTE)
# Resolve repository roots before MCU/platform detection, just as the target does.
if(NOT EXISTS "${ACTUAL_KBD_DIR}/config.h" AND IS_DIRECTORY "${ACTUAL_KBD_DIR}/keyboards")
    dmk_literal_glob_path(_dmk_kbd_glob "${ACTUAL_KBD_DIR}")
    file(GLOB _dmk_keyboard_candidates LIST_DIRECTORIES true "${_dmk_kbd_glob}/keyboards/*")
    foreach(_candidate IN LISTS _dmk_keyboard_candidates)
        if(EXISTS "${_candidate}/config.h")
            set(ACTUAL_KBD_DIR "${_candidate}")
            break()
        endif()
    endforeach()
endif()
set(KEYBOARD_DIR "${ACTUAL_KBD_DIR}")

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

set(MCU "${DEFAULT_MCU_VALUE}" CACHE STRING "Target MCU identifier (built-in or external platform)")

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

# Determine firmware version (CalVer tag or YY.MM.DD-hash)
if(DEFINED ENV{DMK_FW_VERSION})
    set(DMK_FW_VERSION "$ENV{DMK_FW_VERSION}")
elseif(DEFINED ENV{GITHUB_REF_TYPE} AND "$ENV{GITHUB_REF_TYPE}" STREQUAL "tag")
    set(DMK_FW_VERSION "$ENV{GITHUB_REF_NAME}")
else()
    execute_process(
        COMMAND git describe --tags --exact-match
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(_GIT_TAG)
        set(DMK_FW_VERSION "${_GIT_TAG}")
    else()
        execute_process(
            COMMAND git log -1 --format=%cd --date=format:%y.%m.%d
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE _GIT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        execute_process(
            COMMAND git rev-parse --short=7 HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE _GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_GIT_DATE AND _GIT_HASH)
            set(DMK_FW_VERSION "${_GIT_DATE}-${_GIT_HASH}")
        elseif(DEFINED ENV{GITHUB_SHA})
            string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 _CI_HASH)
            string(TIMESTAMP _CURRENT_DATE "%y.%m.%d")
            set(DMK_FW_VERSION "${_CURRENT_DATE}-${_CI_HASH}")
        else()
            string(TIMESTAMP _FALLBACK_DATE "%y.%m.%d")
            set(DMK_FW_VERSION "${_FALLBACK_DATE}-dev")
        endif()
    endif()
endif()

message(STATUS "Firmware version: ${DMK_FW_VERSION}")
add_compile_definitions(DMK_FW_VERSION="${DMK_FW_VERSION}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build" FORCE)
endif()

add_compile_definitions("MCU_${MCU}")
add_compile_options(-Os -ffunction-sections -fdata-sections)



# Path setup
get_filename_component(DMK_ROOT "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

# HHKB model selection uses the same DEFINE list as other build features.
if(KEYBOARD STREQUAL "hhkb")
    string(REPLACE "," ";" HHKB_MODELS "${DEFINE}")
    if("HHKB_TYPE_S" IN_LIST HHKB_MODELS OR HHKB_TYPE_S)
        message(FATAL_ERROR "HHKB_TYPE_S was renamed to HHKB_CLASSIC_TYPE_S")
    endif()
    if("HHKB_STUDIO" IN_LIST HHKB_MODELS)
        message(FATAL_ERROR "HHKB Studio selector is reserved pending board and layout verification")
    endif()
    if("HHKB_CLASSIC" IN_LIST HHKB_MODELS AND
       ("HHKB_CLASSIC_TYPE_S" IN_LIST HHKB_MODELS OR "HHKB_HYBRID" IN_LIST HHKB_MODELS))
        message(FATAL_ERROR "Choose only one HHKB model selector")
    endif()
    if("HHKB_HYBRID" IN_LIST HHKB_MODELS)
        if("HHKB_CLASSIC_TYPE_S" IN_LIST HHKB_MODELS)
            message(FATAL_ERROR "Classic Type-S are not valid for Hybrid")
        endif()
        set(HHKB_BUILD_HYBRID ON)
    endif()
endif()

# Map MCU to platform directory
include("${DMK_ROOT}/cmake/dmk_platform.cmake")

set(CMAKE_PROJECT_NAME dmk_firmware)

# Common FreeRTOS kernel source files
set(FREERTOS_COMMON_SRC
    "${DMK_ROOT}/lib/freertos/list.c"
    "${DMK_ROOT}/lib/freertos/queue.c"
    "${DMK_ROOT}/lib/freertos/tasks.c"
)
if(NOT MCU STREQUAL "stm32l072")
    list(APPEND FREERTOS_COMMON_SRC "${DMK_ROOT}/lib/freertos/heap_4.c")
endif()

# Platform-specific CMake configuration
include("${PLATFORM_DIR}/platform.cmake")
if(DMK_EXTERNAL_PLATFORM AND NOT DEFINED DMK_PLATFORM_DRIVER_DIR AND EXISTS "${PLATFORM_DIR}/drivers")
    set(DMK_PLATFORM_DRIVER_DIR "${PLATFORM_DIR}/drivers")
endif()
if(DMK_PLATFORM_DRIVER_DIR)
    get_filename_component(DMK_PLATFORM_DRIVER_DIR "${DMK_PLATFORM_DRIVER_DIR}" ABSOLUTE BASE_DIR "${PLATFORM_DIR}")
endif()

project(${CMAKE_PROJECT_NAME} ASM C CXX)

if(COMMAND pico_sdk_init)
    pico_sdk_init()
endif()

# Include core targets compilation
include(dmk_core/CMakeLists.txt)

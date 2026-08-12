cmake_minimum_required(VERSION 3.20)

get_filename_component(USB_EXAMPLES_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

enable_language(C ASM)
project(milandr_sdk)
add_library(milandr_sdk INTERFACE)

target_include_directories(milandr_sdk INTERFACE
    "${USB_EXAMPLES_ROOT}"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/CoreSupport"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/DeviceSupport/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/DeviceSupport/startup"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/inc/IRQ_Handler_Template"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/inc/USB_Library"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL"
)

set(_milandr_sdk_sources
    "${USB_EXAMPLES_ROOT}/common/Startup/startup_gcc_MDR32F9Q2I.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/DeviceSupport/startup/system_K1986VE9xI.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_asm_GCC.S"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_rst_clk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_eeprom.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_bkp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_ssp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_usb.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/USB_Library/MDR32FxQI_usb_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/MDR32FxQI_uart.c"
)

if(MILANDR_SDK_USB_CDC)
    list(APPEND _milandr_sdk_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/SPL/src/USB_Library/MDR32FxQI_usb_CDC.c"
    )
endif()

target_sources(milandr_sdk INTERFACE ${_milandr_sdk_sources})

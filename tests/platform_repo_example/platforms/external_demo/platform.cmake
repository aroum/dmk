# Integration fixture: reuse an existing SDK/RTOS port under a new MCU identifier.
# This is not a real new silicon port and must not be flashed to a keyboard.
set(_external_root "${PLATFORM_DIR}")
set(PLATFORM_DIR "${DMK_ROOT}/platforms/stm32f072")
include("${PLATFORM_DIR}/platform.cmake")
set(PLATFORM_DIR "${_external_root}")
list(APPEND PLATFORM_DEFS configCPU_CLOCK_HZ=48000000UL configTOTAL_HEAP_SIZE=1024
    vPortSVCHandler=SVC_Handler xPortPendSVHandler=PendSV_Handler xPortSysTickHandler=SysTick_Handler)
set(DMK_PLATFORM_DRIVER_DIR "${CMAKE_BINARY_DIR}/external-drivers")
file(MAKE_DIRECTORY "${DMK_PLATFORM_DRIVER_DIR}")
configure_file("${PLATFORM_DIR}/drivers/gpio.c.in" "${DMK_PLATFORM_DRIVER_DIR}/gpio.c" @ONLY)
function(platform_configure_target target)
    target_compile_definitions(${target} PRIVATE EXTERNAL_PLATFORM_TARGET=1)
endfunction()

# Shared board config and common sources for keyboard / midi / vcom examples.
get_filename_component(USB_EXAMPLES_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(USB_EXAMPLES_COMMON_INC
    "${USB_EXAMPLES_ROOT}/common/inc"
    "${USB_EXAMPLES_ROOT}"
)

set(USB_EXAMPLES_COMMON_SRC_BASE
    "${USB_EXAMPLES_ROOT}/common/src/clk.c"
    "${USB_EXAMPLES_ROOT}/common/src/board_usb.c"
)

set(USB_EXAMPLES_COMMON_SRC_GPIO
    "${USB_EXAMPLES_ROOT}/common/src/board_gpio.c"
)

function(usb_examples_apply_common target)
    target_include_directories(${target} PRIVATE ${USB_EXAMPLES_COMMON_INC})
    target_sources(${target} PRIVATE ${USB_EXAMPLES_COMMON_SRC_BASE})
endfunction()

function(usb_examples_apply_common_with_gpio target)
    usb_examples_apply_common(${target})
    target_sources(${target} PRIVATE ${USB_EXAMPLES_COMMON_SRC_GPIO})
endfunction()

function(usb_examples_apply_sdk_includes sdk_target)
    target_include_directories(${sdk_target} INTERFACE ${USB_EXAMPLES_COMMON_INC})
endfunction()

# Per-example SPL overlay: MDR32FxQI_config.h and MDR32FxQI_usb_handlers.h in src/sdk/.
function(usb_examples_apply_milandr_sdk target)
    get_filename_component(_sdk_dir "${CMAKE_CURRENT_SOURCE_DIR}/sdk" ABSOLUTE)
    target_include_directories(${target} BEFORE PRIVATE
        "${_sdk_dir}"
        "${_sdk_dir}/USB_Library"
    )
    target_link_libraries(${target} PRIVATE milandr_sdk)
endfunction()

function(usb_examples_add_milandr_sdk)
    if(NOT TARGET milandr_sdk)
        add_subdirectory(
            "${USB_EXAMPLES_ROOT}/common/Drivers"
            "${CMAKE_BINARY_DIR}/milandr_sdk"
        )
    endif()
endfunction()

set(USB_EXAMPLES_FREERTOS_DIR "${USB_EXAMPLES_ROOT}/freertos")

function(usb_examples_apply_freertos target)
    target_include_directories(${target} PRIVATE
        "${USB_EXAMPLES_FREERTOS_DIR}/include"
        "${USB_EXAMPLES_FREERTOS_DIR}/portable/GCC/ARM_CM3"
    )
    target_sources(${target} PRIVATE
        "${USB_EXAMPLES_FREERTOS_DIR}/croutine.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/event_groups.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/list.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/queue.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/stream_buffer.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/tasks.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/timers.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/portable/GCC/ARM_CM3/port.c"
        "${USB_EXAMPLES_FREERTOS_DIR}/portable/MemMang/heap_4.c"
    )
endfunction()

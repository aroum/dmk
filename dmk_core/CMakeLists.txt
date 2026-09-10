# Common includes and sources
set(DMK_COMMON_INC
    "${DMK_ROOT}/dmk_core/include"
    "${DMK_ROOT}/dmk_core/core"
    "${DMK_ROOT}/dmk_core/features"
    "${DMK_ROOT}/dmk_core/drivers"
    "${DMK_ROOT}/dmk_core/config"
    "${PLATFORM_DIR}"
)

# Base core source files (matrix.c will be conditionally included based on CUSTOM_MATRIX)
set(DMK_CORE_BASE_SRC
    # Core orchestration
    "${DMK_ROOT}/dmk_core/core/main.c"
    "${DMK_ROOT}/dmk_core/core/keyboard.c"
    "${DMK_ROOT}/dmk_core/core/hooks.c"
    # Software features
    "${DMK_ROOT}/dmk_core/features/layers.c"
    "${DMK_ROOT}/dmk_core/features/hold_tap.c"
    "${DMK_ROOT}/dmk_core/features/oneshot.c"
    "${DMK_ROOT}/dmk_core/features/combos.c"
    "${DMK_ROOT}/dmk_core/features/macros.c"
    "${DMK_ROOT}/dmk_core/features/midi.c"
    "${DMK_ROOT}/dmk_core/features/mouse.c"
    # Hardware subsystems & drivers
    "${DMK_ROOT}/dmk_core/drivers/split.c"
    "${DMK_ROOT}/dmk_core/drivers/encoder.c"
    "${DMK_ROOT}/dmk_core/drivers/led.c"
    "${DMK_ROOT}/dmk_core/drivers/rgb.c"
    "${DMK_ROOT}/dmk_core/drivers/usb.c"
    "${DMK_ROOT}/dmk_core/drivers/adc.c"
    "${DMK_ROOT}/dmk_core/drivers/gpio.c"
    "${DMK_ROOT}/dmk_core/drivers/i2c.c"
    "${DMK_ROOT}/dmk_core/drivers/spi.c"
)

# Function to build keyboard firmware
function(build_keyboard KEYBOARD_NAME)
    # Resolve keyboard directory (supports in-tree names or custom external directories)
    if(KEYBOARD_DIR AND EXISTS "${KEYBOARD_DIR}")
        set(ACTUAL_KEYBOARD_DIR "${KEYBOARD_DIR}")
        get_filename_component(KBD_BASE "${KEYBOARD_DIR}" NAME)
    elseif(EXISTS "${KEYBOARD_NAME}")
        get_filename_component(ACTUAL_KEYBOARD_DIR "${KEYBOARD_NAME}" ABSOLUTE)
        get_filename_component(KBD_BASE "${ACTUAL_KEYBOARD_DIR}" NAME)
    else()
        set(ACTUAL_KEYBOARD_DIR "${DMK_ROOT}/keyboards/${KEYBOARD_NAME}")
        set(KBD_BASE "${KEYBOARD_NAME}")
    endif()
    
    if(NOT EXISTS "${ACTUAL_KEYBOARD_DIR}/config.h" AND EXISTS "${ACTUAL_KEYBOARD_DIR}/keyboards")
        file(GLOB SUB_KBD_DIRS "${ACTUAL_KEYBOARD_DIR}/keyboards/*")
        foreach(SUB_DIR ${SUB_KBD_DIRS})
            if(IS_DIRECTORY "${SUB_DIR}" AND EXISTS "${SUB_DIR}/config.h")
                set(ACTUAL_KEYBOARD_DIR "${SUB_DIR}")
                get_filename_component(KBD_BASE "${SUB_DIR}" NAME)
                break()
            endif()
        endforeach()
    endif()

    if(NOT EXISTS "${ACTUAL_KEYBOARD_DIR}")
        message(FATAL_ERROR "Keyboard configuration not found in ${ACTUAL_KEYBOARD_DIR}")
    endif()
    
    set(SUFFIX "")
    if(DEFINE)
        string(TOLOWER "${DEFINE}" DEFINE_LOWER)
        if(DEFINE_LOWER MATCHES "left")
            set(SUFFIX "_left")
        elseif(DEFINE_LOWER MATCHES "right")
            set(SUFFIX "_right")
        else()
            string(REGEX REPLACE "[^a-zA-Z0-9_]" "" CLEAN_DEF "${DEFINE_LOWER}")
            if(NOT CLEAN_DEF STREQUAL "")
                set(SUFFIX "_${CLEAN_DEF}")
            endif()
        endif()
    endif()
    set(TARGET_NAME "dmk_${KBD_BASE}_${MCU}${SUFFIX}")
    
    add_executable(${TARGET_NAME})
    
    target_include_directories(${TARGET_NAME} PRIVATE
        "${ACTUAL_KEYBOARD_DIR}"
        ${DMK_COMMON_INC}
        ${PLATFORM_INC}
    )
    
    # Check for VIAL and CUSTOM_MATRIX configurations
    set(VIAL_ENABLED ${VIAL})
    set(CUSTOM_MATRIX_ENABLED ${CUSTOM_MATRIX})
    if(EXISTS "${ACTUAL_KEYBOARD_DIR}/config.h")
        file(READ "${ACTUAL_KEYBOARD_DIR}/config.h" CONFIG_CONTENT)
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+VIAL([ \t\r\n]|$)")
            set(VIAL_ENABLED ON)
        endif()
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+CUSTOM_MATRIX([ \t\r\n]|$)")
            set(CUSTOM_MATRIX_ENABLED ON)
        endif()
    endif()

    set(VIAL_SRC "")
    if(VIAL_ENABLED)
        set(VIAL_SRC "${DMK_ROOT}/dmk_core/features/vial.c")

        # vial.json lives in the keyboard source dir so it's visible alongside config.h
        set(VIAL_JSON_FILE "${ACTUAL_KEYBOARD_DIR}/vial.json")
        set(VIAL_GEN_H_FILE "${ACTUAL_KEYBOARD_DIR}/vial_generated.h")

        # Generate vial.json only if it doesn't already exist (allows custom layouts in vial.json)
        if(NOT EXISTS "${VIAL_JSON_FILE}")
            message(STATUS "Generating vial.json for ${KBD_BASE} in ${ACTUAL_KEYBOARD_DIR}...")
            set(GEN_EXTRA_DEFS "")
            if(DEFINE)
                string(REPLACE "," ";" DEFINE_LIST "${DEFINE}")
                foreach(DEF ${DEFINE_LIST})
                    string(STRIP "${DEF}" STRIPPED_DEF)
                    if(NOT STRIPPED_DEF STREQUAL "")
                        list(APPEND GEN_EXTRA_DEFS "-D${STRIPPED_DEF}")
                    endif()
                endforeach()
            endif()
            execute_process(
                COMMAND uv run python3 "${DMK_ROOT}/tools/vial/generate_json.py" "${ACTUAL_KEYBOARD_DIR}/config.h" "${VIAL_JSON_FILE}" ${GEN_EXTRA_DEFS}
                RESULT_VARIABLE GEN_EXEC_RESULT
            )
            if(NOT GEN_EXEC_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to execute vial.json generator")
            endif()
        else()
            message(STATUS "Using existing vial.json for ${KBD_BASE} in ${ACTUAL_KEYBOARD_DIR}")
        endif()

        if(EXISTS "${VIAL_JSON_FILE}")
            add_custom_command(
                OUTPUT "${VIAL_GEN_H_FILE}"
                COMMAND uv run python3 "${DMK_ROOT}/tools/vial/compress.py" "${VIAL_JSON_FILE}" "${VIAL_GEN_H_FILE}"
                DEPENDS "${VIAL_JSON_FILE}" "${DMK_ROOT}/tools/vial/compress.py"
                COMMENT "Compressing vial.json for ${KBD_BASE}"
            )
            set_source_files_properties("${VIAL_SRC}" PROPERTIES OBJECT_DEPENDS "${VIAL_GEN_H_FILE}")
            target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        endif()
    endif()

    # Core source list
    set(DMK_CORE_SRC ${DMK_CORE_BASE_SRC})
    if(NOT CUSTOM_MATRIX_ENABLED)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/matrix.c")
    endif()

    # Collect any local keyboard-specific sources (e.g. matrix_magneteno.c)
    file(GLOB KEYBOARD_LOCAL_SRC "${ACTUAL_KEYBOARD_DIR}/*.c")

    target_sources(${TARGET_NAME} PRIVATE
        ${DMK_CORE_SRC}
        ${PLATFORM_SRC}
        ${VIAL_SRC}
        ${KEYBOARD_LOCAL_SRC}
    )
    
    # Auto-discover keyboard-specific modules (e.g. keyboards/<kbd>/modules/*)
    set(ALL_ACTIVE_MODULES "")
    if(EXISTS "${ACTUAL_KEYBOARD_DIR}/modules")
        file(GLOB KBD_MODULE_DIRS LIST_DIRECTORIES true "${ACTUAL_KEYBOARD_DIR}/modules/*")
        foreach(MOD_DIR ${KBD_MODULE_DIRS})
            if(IS_DIRECTORY "${MOD_DIR}")
                list(APPEND ALL_ACTIVE_MODULES "${MOD_DIR}")
            endif()
        endforeach()
    endif()
    if(EXISTS "${ACTUAL_KEYBOARD_DIR}/module.cmake")
        list(APPEND ALL_ACTIVE_MODULES "${ACTUAL_KEYBOARD_DIR}")
    endif()

    # Load external and user-specified modules
    if(DMK_MODULES)
        foreach(MODULE_PATH ${DMK_MODULES})
            if(IS_ABSOLUTE "${MODULE_PATH}")
                list(APPEND ALL_ACTIVE_MODULES "${MODULE_PATH}")
            else()
                set(RESOLVED_PATH "${DMK_ROOT}/${MODULE_PATH}")
                if(NOT EXISTS "${RESOLVED_PATH}" AND EXISTS "${ACTUAL_KEYBOARD_DIR}/modules/${MODULE_PATH}")
                    set(RESOLVED_PATH "${ACTUAL_KEYBOARD_DIR}/modules/${MODULE_PATH}")
                elseif(NOT EXISTS "${RESOLVED_PATH}" AND EXISTS "${DMK_ROOT}/tests/modules/${MODULE_PATH}")
                    set(RESOLVED_PATH "${DMK_ROOT}/tests/modules/${MODULE_PATH}")
                endif()
                list(APPEND ALL_ACTIVE_MODULES "${RESOLVED_PATH}")
            endif()
        endforeach()
    endif()

    if(ALL_ACTIVE_MODULES)
        list(REMOVE_DUPLICATES ALL_ACTIVE_MODULES)
        foreach(ABS_MODULE_PATH ${ALL_ACTIVE_MODULES})
            if(EXISTS "${ABS_MODULE_PATH}")
                message(STATUS "Loading module: ${ABS_MODULE_PATH}")
                target_include_directories(${TARGET_NAME} PRIVATE "${ABS_MODULE_PATH}")
                if(EXISTS "${ABS_MODULE_PATH}/include")
                    target_include_directories(${TARGET_NAME} PRIVATE "${ABS_MODULE_PATH}/include")
                endif()
                if(EXISTS "${ABS_MODULE_PATH}/module.cmake")
                    include("${ABS_MODULE_PATH}/module.cmake")
                else()
                    # Auto-add any .c source files if module.cmake is omitted
                    file(GLOB MOD_C_FILES "${ABS_MODULE_PATH}/*.c")
                    if(MOD_C_FILES)
                        target_sources(${TARGET_NAME} PRIVATE ${MOD_C_FILES})
                    endif()
                endif()

                # Automatically compile any .pio assembly files found in the module
                if(COMMAND pico_generate_pio_header)
                    file(GLOB_RECURSE MODULE_PIO_FILES "${ABS_MODULE_PATH}/*.pio")
                    foreach(PIO_FILE ${MODULE_PIO_FILES})
                        message(STATUS "Generating PIO header for module: ${PIO_FILE}")
                        pico_generate_pio_header(${TARGET_NAME} "${PIO_FILE}")
                    endforeach()
                endif()
            else()
                message(FATAL_ERROR "Module directory not found: ${ABS_MODULE_PATH}")
            endif()
        endforeach()
    endif()

    # Automatically compile any .pio assembly files found in keyboard directory
    if(COMMAND pico_generate_pio_header)
        file(GLOB KEYBOARD_PIO_FILES "${ACTUAL_KEYBOARD_DIR}/*.pio")
        foreach(PIO_FILE ${KEYBOARD_PIO_FILES})
            message(STATUS "Generating PIO header for keyboard: ${PIO_FILE}")
            pico_generate_pio_header(${TARGET_NAME} "${PIO_FILE}")
        endforeach()
    endif()
    
    # Note: Keyboard configuration and keymaps are fully defined in config.h
    
    target_link_libraries(${TARGET_NAME} PRIVATE ${PLATFORM_LIBS})
    
    # Add compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE "MCU_${MCU}")

    if(CUSTOM_MATRIX_ENABLED)
        target_compile_definitions(${TARGET_NAME} PRIVATE "CUSTOM_MATRIX")
    endif()

    if(DEFINE)
        string(REPLACE "," ";" DEFINE_LIST "${DEFINE}")
        foreach(DEF ${DEFINE_LIST})
            string(STRIP "${DEF}" STRIPPED_DEF)
            if(NOT STRIPPED_DEF STREQUAL "")
                target_compile_definitions(${TARGET_NAME} PRIVATE "${STRIPPED_DEF}")
            endif()
        endforeach()
    endif()
 
    if(VIAL_ENABLED)
        target_compile_definitions(${TARGET_NAME} PRIVATE "VIAL")
    endif()
 
    if(DEFINED PLATFORM_DEFS)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${PLATFORM_DEFS})
    endif()
    
    # Platform-specific binary generation
    if(COMMAND platform_post_build)
        platform_post_build(${TARGET_NAME})
    endif()
endfunction()

# Build specified keyboard
build_keyboard(${KEYBOARD})

# Common includes and sources
set(DMK_COMMON_INC
    "${DMK_ROOT}/dmk_core/include"
    "${DMK_ROOT}/dmk_core/core"
    "${DMK_ROOT}/dmk_core/features"
    "${DMK_ROOT}/dmk_core/drivers"
    "${DMK_ROOT}/dmk_core/config"
    "${PLATFORM_DIR}"
)

# Base core source files
set(DMK_CORE_BASE_SRC
    # Core orchestration
    "${DMK_ROOT}/dmk_core/core/main.c"
    "${DMK_ROOT}/dmk_core/core/keyboard.c"
    "${DMK_ROOT}/dmk_core/core/hooks.c"
    # Software features
    "${DMK_ROOT}/dmk_core/features/layers.c"
    # Hardware subsystems & drivers
    "${DMK_ROOT}/dmk_core/drivers/usb.c"
    "${DMK_ROOT}/dmk_core/drivers/gpio.c"
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
    if(EXISTS "${ACTUAL_KEYBOARD_DIR}/config.h")
        file(READ "${ACTUAL_KEYBOARD_DIR}/config.h" CONFIG_CONTENT)
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+VIAL([ \t\r\n]|$)" OR
           CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+VIA([ \t\r\n]|$)" OR
           CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+VIA_V3([ \t\r\n]|$)")
            set(VIAL_ENABLED ON)
        endif()
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+CUSTOM_MATRIX([ \t\r\n]|$)")
            set(CUSTOM_MATRIX_ENABLED ON)
        endif()
    endif()

    if(DEFINE)
        if(DEFINE MATCHES "(^|[,;])VIAL([,;]|$)" OR
           DEFINE MATCHES "(^|[,;])VIA([,;]|$)" OR
           DEFINE MATCHES "(^|[,;])VIA_V3([,;]|$)")
            set(VIAL_ENABLED ON)
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
    endif()

    # Collect any local keyboard-specific sources (e.g. matrix_magneteno.c)
    file(GLOB KEYBOARD_LOCAL_SRC "${ACTUAL_KEYBOARD_DIR}/*.c")

    # Modular feature detection from config.h and modules
    set(DMK_CORE_SRC ${DMK_CORE_BASE_SRC})

    # 1. Custom Matrix vs Standard Matrix
    if(NOT CUSTOM_MATRIX_ENABLED)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/matrix.c")
    endif()

    # 1b. Modular Software Features
    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_HOLD_TAP([ \t\r\n]|$)")
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_HOLD_TAP")
    else()
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/hold_tap.c")
    endif()

    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_ONESHOT([ \t\r\n]|$)")
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_ONESHOT")
    else()
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/oneshot.c")
    endif()

    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_COMBOS([ \t\r\n]|$)")
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_COMBOS")
    else()
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/combos.c")
    endif()

    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_MACROS([ \t\r\n]|$)")
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_MACROS")
    else()
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/macros.c")
    endif()

    # 2. Mouse support (enabled by default, disabled if NO_MOUSE defined)
    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_MOUSE([ \t\r\n]|$)")
        set(MOUSE_ENABLED OFF)
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_MOUSE")
    else()
        set(MOUSE_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/mouse.c")
    endif()

    # 3. Gamepad support (enabled by default, disabled if NO_GAMEPAD defined)
    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_GAMEPAD([ \t\r\n]|$)")
        set(GAMEPAD_ENABLED OFF)
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_GAMEPAD")
    else()
        set(GAMEPAD_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/gamepad.c")
    endif()

    # 4. MIDI support (enabled if MIDI defined and not NO_MIDI)
    set(MIDI_ENABLED OFF)
    if((CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+(MIDI_USB|MIDI_ENABLE|MIDI_JACK)([ \t\r\n]|$)" OR MIDI)
       AND NOT CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_MIDI([ \t\r\n]|$)")
        set(MIDI_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/features/midi.c")
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+MIDI_USB([ \t\r\n]|$)")
            target_compile_definitions(${TARGET_NAME} PRIVATE "MIDI_USB")
        endif()
    else()
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_MIDI")
    endif()

    # 5. Split support
    set(SPLIT_ENABLED OFF)
    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+(NUM_ROWS_SPLIT|NUM_COLS_SPLIT|SPLIT_PINS|SERIAL_PIN)([ \t\r\n]|$)"
       OR (DEFINED DEFINE_LOWER AND DEFINE_LOWER MATCHES "(left|right)"))
        set(SPLIT_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/split.c")
        target_compile_definitions(${TARGET_NAME} PRIVATE "SPLIT_ENABLED")
    endif()

    # 6. Encoder support
    set(ENCODER_ENABLED OFF)
    if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+ENCODER_PINS_A([ \t\r\n]|$)")
        set(ENCODER_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/encoder.c")
    endif()

    # 7. LED support
    set(LED_ENABLED OFF)
    if(CONFIG_CONTENT MATCHES "(LED_PINS|LED_DEBUG|LED_ACTIVITY_PIN|LED_PIN)"
       AND NOT CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_LED([ \t\r\n]|$)")
        set(LED_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/led.c")
    else()
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_LED")
    endif()

    # 8. RGB support
    set(RGB_ENABLED OFF)
    if(CONFIG_CONTENT MATCHES "RGB_NUM"
       AND NOT CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+NO_RGB([ \t\r\n]|$)")
        set(RGB_ENABLED ON)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/rgb.c")
    else()
        target_compile_definitions(${TARGET_NAME} PRIVATE "NO_RGB")
    endif()

    # 9. Hardware bus drivers: I2C, SPI, ADC
    set(I2C_ENABLED OFF)
    set(SPI_ENABLED OFF)
    set(ADC_ENABLED OFF)
    if(CONFIG_CONTENT MATCHES "(I2C_|HAL_USE_I2C)")
        set(I2C_ENABLED ON)
    endif()
    if(CONFIG_CONTENT MATCHES "(SPI_|HAL_USE_SPI)")
        set(SPI_ENABLED ON)
    endif()
    if(CONFIG_CONTENT MATCHES "(ADC_|HAL_USE_ADC)")
        set(ADC_ENABLED ON)
    endif()

    # Scan keyboard local sources and modules for HAL bus driver usage
    set(ALL_FILES_TO_SCAN ${KEYBOARD_LOCAL_SRC})
    foreach(MOD_PATH ${ALL_ACTIVE_MODULES})
        file(GLOB_RECURSE MOD_SRCS "${MOD_PATH}/*.[ch]" "${MOD_PATH}/*.cpp")
        list(APPEND ALL_FILES_TO_SCAN ${MOD_SRCS})
    endforeach()
    foreach(F ${ALL_FILES_TO_SCAN})
        file(READ "${F}" F_TXT)
        if(F_TXT MATCHES "(hal_i2c\\.h|hardware/i2c\\.h)")
            set(I2C_ENABLED ON)
        endif()
        if(F_TXT MATCHES "(hal_spi\\.h|hardware/spi\\.h)")
            set(SPI_ENABLED ON)
        endif()
        if(F_TXT MATCHES "(hal_adc\\.h|hardware/adc\\.h)")
            set(ADC_ENABLED ON)
        endif()
    endforeach()

    if(I2C_ENABLED)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/i2c.c")
        target_compile_definitions(${TARGET_NAME} PRIVATE "I2C_ENABLED")
    endif()
    if(SPI_ENABLED)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/spi.c")
        target_compile_definitions(${TARGET_NAME} PRIVATE "SPI_ENABLED")
    endif()
    if(ADC_ENABLED)
        list(APPEND DMK_CORE_SRC "${DMK_ROOT}/dmk_core/drivers/adc.c")
        target_compile_definitions(${TARGET_NAME} PRIVATE "ADC_ENABLED")
    endif()

    # Conditional platform-specific sources for non-RP2040 MCUs
    set(PLATFORM_EXTRA_SRC "")
    if(SPLIT_ENABLED AND EXISTS "${PLATFORM_DIR}/split.c" AND NOT (MCU STREQUAL "rp2040" OR MCU STREQUAL "rp2350"))
        list(APPEND PLATFORM_EXTRA_SRC "${PLATFORM_DIR}/split.c")
    endif()
    if(RGB_ENABLED AND NOT (MCU STREQUAL "rp2040" OR MCU STREQUAL "rp2350"))
        if(EXISTS "${PLATFORM_DIR}/ws2812_spi.c")
            list(APPEND PLATFORM_EXTRA_SRC "${PLATFORM_DIR}/ws2812_spi.c")
        elseif(EXISTS "${PLATFORM_DIR}/ws2812_ssp.c")
            list(APPEND PLATFORM_EXTRA_SRC "${PLATFORM_DIR}/ws2812_ssp.c")
        endif()
    endif()
    if(MIDI_ENABLED AND (MCU STREQUAL "rp2040" OR MCU STREQUAL "rp2350"))
        if(EXISTS "${PICO_SDK_PATH}/lib/tinyusb/src/class/midi/midi_device.c")
            list(APPEND PLATFORM_EXTRA_SRC "${PICO_SDK_PATH}/lib/tinyusb/src/class/midi/midi_device.c")
        endif()
    endif()

    target_sources(${TARGET_NAME} PRIVATE
        ${DMK_CORE_SRC}
        ${PLATFORM_SRC}
        ${PLATFORM_EXTRA_SRC}
        ${VIAL_SRC}
        ${KEYBOARD_LOCAL_SRC}
    )

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

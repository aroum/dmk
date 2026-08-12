# Common includes and sources
set(DMK_COMMON_INC
    "${DMK_ROOT}/dmk_core/include"
    "${DMK_ROOT}/dmk_core/config"
    "${PLATFORM_DIR}"
)

set(DMK_CORE_SRC
    "${DMK_ROOT}/dmk_core/src/main.c"
    "${DMK_ROOT}/dmk_core/src/matrix.c"
    "${DMK_ROOT}/dmk_core/src/keyboard.c"
    "${DMK_ROOT}/dmk_core/src/layers.c"
    "${DMK_ROOT}/dmk_core/src/hold_tap.c"
    "${DMK_ROOT}/dmk_core/src/oneshot.c"
    "${DMK_ROOT}/dmk_core/src/combos.c"
    "${DMK_ROOT}/dmk_core/src/midi.c"
    "${DMK_ROOT}/dmk_core/src/macros.c"
    "${DMK_ROOT}/dmk_core/src/hooks.c"
    "${DMK_ROOT}/dmk_core/src/split.c"
    "${DMK_ROOT}/dmk_core/src/usb.c"
    "${DMK_ROOT}/dmk_core/src/led.c"
    "${DMK_ROOT}/dmk_core/src/rgb.c"
    "${DMK_ROOT}/dmk_core/src/encoder.c"
)

# Function to build keyboard firmware
function(build_keyboard KEYBOARD_NAME)
    set(KEYBOARD_DIR "${DMK_ROOT}/keyboards/${KEYBOARD_NAME}")
    
    if(NOT EXISTS "${KEYBOARD_DIR}")
        message(WARNING "Keyboard ${KEYBOARD_NAME} not found in ${KEYBOARD_DIR}")
        return()
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
    set(TARGET_NAME "dmk_${KEYBOARD_NAME}_${MCU}${SUFFIX}")
    
    add_executable(${TARGET_NAME})
    
    target_include_directories(${TARGET_NAME} PRIVATE
        "${KEYBOARD_DIR}"
        ${DMK_COMMON_INC}
        ${PLATFORM_INC}
    )
    
    # Enable VIAL if it is defined in the CMake cache or in the keyboard's config.h
    set(VIAL_ENABLED ${VIAL})
    set(VIAL_FROM_CONFIG OFF)
    if(EXISTS "${KEYBOARD_DIR}/config.h")
        file(READ "${KEYBOARD_DIR}/config.h" CONFIG_CONTENT)
        if(CONFIG_CONTENT MATCHES "#[ \t]*define[ \t]+VIAL([ \t\r\n]|$)")
            set(VIAL_ENABLED ON)
            set(VIAL_FROM_CONFIG ON)
        endif()
    endif()

    set(VIAL_SRC "")
    if(VIAL_ENABLED)
        set(VIAL_SRC "${DMK_ROOT}/dmk_core/src/vial.c")

        # vial.json lives in the keyboard source dir so it's visible alongside config.h
        set(VIAL_JSON_FILE "${KEYBOARD_DIR}/vial.json")
        set(VIAL_GEN_H_FILE "${KEYBOARD_DIR}/vial_generated.h")

        # Regenerate vial.json if it doesn't exist or if config.h has changed
        message(STATUS "Generating vial.json for ${KEYBOARD_NAME} in ${KEYBOARD_DIR}...")
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
            COMMAND gcc -O2 -DMCU_${MCU} ${GEN_EXTRA_DEFS} -I${KEYBOARD_DIR} -I${DMK_ROOT}/dmk_core/include -I${PLATFORM_DIR} -I${DMK_ROOT}/tools/vial/cJSON
                    ${DMK_ROOT}/tools/vial/generate_json.c ${DMK_ROOT}/tools/vial/cJSON/cJSON.c
                    -o ${CMAKE_BINARY_DIR}/generate_json
            RESULT_VARIABLE GEN_COMPILE_RESULT
        )
        if(NOT GEN_COMPILE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to compile host vial.json generator")
        endif()

        execute_process(
            COMMAND ${CMAKE_BINARY_DIR}/generate_json "${VIAL_JSON_FILE}"
            RESULT_VARIABLE GEN_EXEC_RESULT
        )
        if(NOT GEN_EXEC_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to execute host vial.json generator")
        endif()

        if(EXISTS "${VIAL_JSON_FILE}")
            add_custom_command(
                OUTPUT "${VIAL_GEN_H_FILE}"
                COMMAND uv run python3 "${DMK_ROOT}/tools/vial/compress.py" "${VIAL_JSON_FILE}" "${VIAL_GEN_H_FILE}"
                DEPENDS "${VIAL_JSON_FILE}" "${DMK_ROOT}/tools/vial/compress.py"
                COMMENT "Compressing vial.json for ${KEYBOARD_NAME}"
            )
            set_source_files_properties("${VIAL_SRC}" PROPERTIES OBJECT_DEPENDS "${VIAL_GEN_H_FILE}")
            target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        endif()
    endif()

    target_sources(${TARGET_NAME} PRIVATE
        ${DMK_CORE_SRC}
        ${PLATFORM_SRC}
        ${VIAL_SRC}
    )
    
    # Note: Keyboard configuration and keymaps are fully defined in config.h
    
    target_link_libraries(${TARGET_NAME} PRIVATE ${PLATFORM_LIBS})
    
    # Add compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE "MCU_${MCU}")

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

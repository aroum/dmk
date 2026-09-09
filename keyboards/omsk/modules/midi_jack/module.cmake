target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/midi_jack.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Automatically activate MIDI keycode handling in DMK core
target_compile_definitions(${TARGET_NAME} PRIVATE
    MIDI_ENABLE=1
)

if(MCU STREQUAL "rp2040" OR MCU STREQUAL "rp2350")
    target_link_libraries(${TARGET_NAME} PRIVATE hardware_uart)
endif()

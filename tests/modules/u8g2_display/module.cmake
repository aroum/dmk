target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/u8g2_display_module.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Sharp Memory LCD (LS011B7DH03 160x68) Module CMake manifest
file(GLOB U8G2_CORE_SRC "${DMK_ROOT}/lib/u8g2/csrc/*.c")

target_sources(${TARGET_NAME} PRIVATE
    ${U8G2_CORE_SRC}
    "${CMAKE_CURRENT_LIST_DIR}/sharp_memory_lcd_module.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${DMK_ROOT}/lib/u8g2/csrc"
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

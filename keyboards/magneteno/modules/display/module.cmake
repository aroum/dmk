file(GLOB U8G2_SOURCES "${DMK_ROOT}/lib/u8g2/csrc/*.c")

target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/magneteno_display.c"
    ${U8G2_SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${DMK_ROOT}/lib/u8g2/csrc"
)

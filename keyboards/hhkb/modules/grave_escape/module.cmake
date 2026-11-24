target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/grave_escape.c")
target_compile_definitions(${TARGET_NAME} PRIVATE HHKB_GRAVE_ESCAPE=1)

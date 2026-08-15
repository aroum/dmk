target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/hall_calibration.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

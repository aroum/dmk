target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/magneteno_imu.c"
)
target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)

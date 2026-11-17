if(HHKB_BUILD_HYBRID)
    target_sources(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/hybrid.c"
        "${CMAKE_CURRENT_LIST_DIR}/hybrid_stop.c")
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
endif()

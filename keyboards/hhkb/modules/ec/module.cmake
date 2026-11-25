target_sources(${TARGET_NAME} PRIVATE
    "${DMK_ROOT}/dmk_core/features/ec_curve.c"
    "${CMAKE_CURRENT_LIST_DIR}/classic_matrix.c"
    "${CMAKE_CURRENT_LIST_DIR}/ec_settings.c"
    "${CMAKE_CURRENT_LIST_DIR}/matrix.c"
    "${CMAKE_CURRENT_LIST_DIR}/rapid_trigger.c"
    "${CMAKE_CURRENT_LIST_DIR}/sof_lock.c"
)
# Optimize cold configuration/storage together without changing scan/ISR codegen.
set_property(SOURCE
    "${DMK_ROOT}/dmk_core/features/vial.c"
    "${DMK_ROOT}/dmk_core/drivers/usb_descriptors.c"
    "${CMAKE_CURRENT_LIST_DIR}/ec_settings.c"
    "${CMAKE_CURRENT_LIST_DIR}/../via_settings/hhkb_keymap_storage.c"
    "${CMAKE_CURRENT_LIST_DIR}/../via_settings/hhkb_via_controls.c"
    APPEND PROPERTY COMPILE_OPTIONS -flto)
# Keep VIA dispatch and resumable storage within the Hybrid flash budget.
# Acquisition retains its explicit O2 setting and is compiled separately.
set_property(SOURCE "${DMK_ROOT}/dmk_core/features/vial.c"
    "${CMAKE_CURRENT_LIST_DIR}/../via_settings/hhkb_keymap_storage.c"
    APPEND PROPERTY COMPILE_OPTIONS -Oz)
target_link_options(${TARGET_NAME} PRIVATE -flto)

target_compile_definitions(${TARGET_NAME} PRIVATE HHKB_USB_REPORT_DIAGNOSTICS=1)

# ayatana-appindicator-glib.deps

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-glib.deps" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

# ayatana-appindicator-glib.vapi

find_package (Vala REQUIRED)

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-glib.vapi"
    DEPENDS "src" "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicatorGlib-2.0.typelib"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${VAPI_GEN}
    --library=ayatana-appindicator-glib
    --pkg gio-2.0
    --pkg gobject-2.0
    --girdir="${CMAKE_SOURCE_DIR}/src"
    --metadatadir "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicatorGlib-2.0.gir"
    "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-custom.vala"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-glib.vapi" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

add_custom_target ("bindings-vala" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-glib.vapi")

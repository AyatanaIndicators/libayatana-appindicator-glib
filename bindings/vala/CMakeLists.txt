# ayatana-appindicator-2.0.deps

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-2.0.deps" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

# ayatana-appindicator-2.0.vapi

find_package (Vala REQUIRED)

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-2.0.vapi"
    DEPENDS "src" "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicator-2.0.typelib"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${VAPI_GEN}
    --library=ayatana-appindicator-2.0
    --pkg gio-2.0
    --pkg gobject-2.0
    --girdir="${CMAKE_SOURCE_DIR}/src"
    --metadatadir "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicator-2.0.gir"
    "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-custom.vala"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-2.0.vapi" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

add_custom_target ("bindings-vala" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-2.0.vapi")

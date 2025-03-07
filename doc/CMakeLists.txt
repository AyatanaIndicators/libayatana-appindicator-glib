# libayatana-appindicator-docs.sgml

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/AyatanaAppIndicator-2.0.toml.in" "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator-2.0.toml" @ONLY)

find_program (GI-DOCGEN gi-docgen)

add_custom_command (
    OUTPUT "gi-docgen"
    DEPENDS "src"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GI-DOCGEN}
    generate
    --quiet
    --config="AyatanaAppIndicator-2.0.toml"
    "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicator-2.0.gir"
)

set_source_files_properties ("gi-docgen" PROPERTIES SYMBOLIC True)
install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator-2.0" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/doc")

add_custom_target ("doc" ALL DEPENDS "gi-docgen")




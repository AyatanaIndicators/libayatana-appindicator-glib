# simple-client

add_executable ("simple-client" "${CMAKE_CURRENT_SOURCE_DIR}/simple-client.c")
target_compile_definitions ("simple-client" PUBLIC LOCAL_ICON="${CMAKE_CURRENT_SOURCE_DIR}/simple-client-test-icon.png")
target_include_directories ("simple-client" PUBLIC ${DEPS_INCLUDE_DIRS})
target_include_directories ("simple-client" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries ("simple-client" ${DEPS_LIBRARIES} ayatana-appindicator)
add_dependencies ("simple-client" "ayatana-appindicator")

# simple-client-vala.c

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c"
    DEPENDS "bindings-vala"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${VALA_COMPILER}
    --pkg ayatana-appindicator-2.0
    --pkg gio-2.0
    --vapidir=${CMAKE_BINARY_DIR}/bindings/vala
    -C simple-client-vala.vala
    --directory=${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
    COMMAND
    sed
    -i "s|#include\\s*<\\s*libayatana-appindicator/ayatana-appindicator.h\\s*>||g"
    "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c"
)

# simple-client-vala

set_source_files_properties ("${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c" PROPERTIES COMPILE_FLAGS " -include ${CMAKE_SOURCE_DIR}/src/ayatana-appindicator.h -Wno-unknown-warning-option -Wno-discarded-qualifiers")
add_executable ("simple-client-vala" "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c")
target_include_directories ("simple-client-vala" PUBLIC ${DEPS_INCLUDE_DIRS})
target_link_libraries ("simple-client-vala" ${DEPS_LIBRARIES} ayatana-appindicator)
add_dependencies ("simple-client-vala" "ayatana-appindicator")

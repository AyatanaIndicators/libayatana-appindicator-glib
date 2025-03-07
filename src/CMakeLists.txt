set (
    SOURCES
    ayatana-appindicator.c
    ayatana-appindicator-enum-types.c
    application-service-marshal.c
)

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libayatana-appindicator-glib")

# ayatana-appindicator-glib.pc

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-glib.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-glib.pc" @ONLY)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-glib.pc" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

# ayatana-appindicator-enum-types.h

find_program(GLIB_MKENUMS glib-mkenums)

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template ayatana-appindicator-enum-types.h.in
    ayatana-appindicator.h
    --output="${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.h"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libayatana-appindicator-glib")

# ayatana-appindicator-enum-types.c

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template ayatana-appindicator-enum-types.c.in
    ayatana-appindicator.h
    --output="${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.c"
)

find_program (GLIB_GENMARSHAL glib-genmarshal)

# application-service-marshal.h

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator-enum-types.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_GENMARSHAL}
    --prefix=_application_service_marshal
    --header application-service-marshal.list
    --quiet
    --output="${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
)

# application-service-marshal.c

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_GENMARSHAL}
    --prefix=_application_service_marshal
    --body application-service-marshal.list
    --include-header=application-service-marshal.h
    --quiet
    --output="${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.c"
)

# libayatana-appindicator-glib.so

add_library ("ayatana-appindicator-glib" SHARED ${SOURCES})
set_target_properties ("ayatana-appindicator-glib" PROPERTIES VERSION 2.0.0 SOVERSION 2)
target_compile_definitions ("ayatana-appindicator-glib" PUBLIC G_LOG_DOMAIN="libayatana-appindicator-glib" SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories ("ayatana-appindicator-glib" PUBLIC ${DEPS_INCLUDE_DIRS})
target_include_directories ("ayatana-appindicator-glib" PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories ("ayatana-appindicator-glib" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries ("ayatana-appindicator-glib" ${DEPS_LIBRARIES})
target_link_options ("ayatana-appindicator-glib" PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator.symbols")
install (TARGETS "ayatana-appindicator-glib" LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

# AyatanaAppIndicatorGlib-2.0.gir

find_package (GObjectIntrospection REQUIRED)

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.gir"
    DEPENDS "ayatana-appindicator-glib"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${INTROSPECTION_SCANNER}
    ayatana-appindicator.c ayatana-appindicator.h
    --add-include-path=${CMAKE_CURRENT_BINARY_DIR}
    --c-include=libayatana-appindicator-glib/ayatana-appindicator.h
    --symbol-prefix=app
    --identifier-prefix=App
    --namespace=AyatanaAppIndicatorGlib
    --nsversion=2.0
    --quiet
    --warn-all
    --include=Gio-2.0
    --include=GObject-2.0
    --library-path=${CMAKE_CURRENT_BINARY_DIR}
    --library="ayatana-appindicator-glib"
    --output "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.gir"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.gir" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/gir-1.0")

# AyatanaAppIndicatorGlib-2.0.typelib

add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.typelib"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.gir"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${INTROSPECTION_COMPILER}
    --includedir=${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.gir
    -o "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.typelib"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.typelib" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/girepository-1.0")

add_custom_target ("src" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicatorGlib-2.0.typelib")

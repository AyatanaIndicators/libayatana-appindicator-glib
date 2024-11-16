set(HEADERS
    app-indicator.h
)

set(SOURCES
    app-indicator.c
    app-indicator-enum-types.c
    application-service-marshal.c
    generate-id.c
    gen-notification-item.xml.c
    gen-notification-watcher.xml.c
)

if (FLAVOUR_GTK3)
    set (ayatana_appindicator_gtkver "ayatana-appindicator3")
    set (ayatana_appindicator_girver "AyatanaAppIndicator3")
    set (gtk_girver "Gtk-3.0")
elseif (FLAVOUR_GTK2)
    set (ayatana_appindicator_gtkver "ayatana-appindicator")
    set (ayatana_appindicator_girver "AyatanaAppIndicator")
    set (gtk_girver "Gtk-2.0")
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app-indicator.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/lib${ayatana_appindicator_gtkver}-0.1/libayatana-appindicator")

# ayatana-appindicator{,3}-0.1.pc

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${ayatana_appindicator_gtkver}-0.1.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_gtkver}-0.1.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_gtkver}-0.1.pc" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

# app-indicator-enum-types.h

find_program(GLIB_MKENUMS glib-mkenums)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template app-indicator-enum-types.h.in
    ${HEADERS}
    --output="${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/lib${ayatana_appindicator_gtkver}-0.1/libayatana-appindicator")

# app-indicator-enum-types.c

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template app-indicator-enum-types.c.in
    ${HEADERS}
    --output="${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
)

find_program(GLIB_GENMARSHAL glib-genmarshal)

# application-service-marshal.h

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_GENMARSHAL}
    --prefix=_application_service_marshal
    --header application-service-marshal.list
    --quiet
    --output="${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
)

# application-service-marshal.c

add_custom_command(
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

# gen-notification-item.xml.h

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-item.xml.h" "extern const char * _notification_item;")

# gen-notification-item.xml.c

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/notification-item.xml" GEN_NOTIFICATION_ITEM_XML_C)
string(REPLACE "\"" "\\\"" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(REPLACE "\n" "\\n\"\n\"" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(REGEX REPLACE "\n\"$" "\n" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(PREPEND GEN_NOTIFICATION_ITEM_XML_C "const char * _notification_item = \n\"")
string(APPEND GEN_NOTIFICATION_ITEM_XML_C "\;")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-item.xml.c" ${GEN_NOTIFICATION_ITEM_XML_C})

# gen-notification-watcher.xml.h

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-watcher.xml.h" "extern const char * _notification_watcher;")

# gen-notification-watcher.xml.c

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/notification-watcher.xml" GEN_NOTIFICATION_WATCHER_XML_C)
string(REPLACE "\"" "\\\"" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(REPLACE "\n" "\\n\"\n\"" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(REGEX REPLACE "\n\"$" "\n" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(PREPEND GEN_NOTIFICATION_WATCHER_XML_C "const char * _notification_watcher = \n\"")
string(APPEND GEN_NOTIFICATION_WATCHER_XML_C "\;")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-watcher.xml.c" ${GEN_NOTIFICATION_WATCHER_XML_C})

# libayatana-appindicator{,3}.so

add_library("${ayatana_appindicator_gtkver}" SHARED ${SOURCES})
set_target_properties("${ayatana_appindicator_gtkver}" PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_compile_definitions("${ayatana_appindicator_gtkver}" PUBLIC G_LOG_DOMAIN="libayatana-appindicator")
target_include_directories("${ayatana_appindicator_gtkver}" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("${ayatana_appindicator_gtkver}" PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories("${ayatana_appindicator_gtkver}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries("${ayatana_appindicator_gtkver}" ${PROJECT_DEPS_LIBRARIES})
if(NOT APPLE)
    target_link_options ("${ayatana_appindicator_gtkver}" PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/app-indicator.symbols")
endif()
install(TARGETS "${ayatana_appindicator_gtkver}" LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

# AyatanaAppIndicator{,3}-0.1.gir

find_package(GObjectIntrospection REQUIRED)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.gir"
    DEPENDS "${ayatana_appindicator_gtkver}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${INTROSPECTION_SCANNER}
    app-indicator.c ${HEADERS}
    --add-include-path=${CMAKE_CURRENT_BINARY_DIR}
    --c-include=libayatana-appindicator/app-indicator.h
    --symbol-prefix=app
    --identifier-prefix=App
    --namespace=${ayatana_appindicator_girver}
    --nsversion=0.1
    --quiet
    --warn-all
    --include=GObject-2.0
    --include=${gtk_girver}
    --library-path=${CMAKE_INSTALL_FULL_LIBDIR}
    --library="${ayatana_appindicator_gtkver}"
    --output "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.gir"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.gir" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/gir-1.0")

# AyatanaAppIndicator{,3}-0.1.typelib

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.typelib"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.gir"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${INTROSPECTION_COMPILER}
    --includedir=${CMAKE_INSTALL_FULL_LIBDIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.gir
    -o "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.typelib"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.typelib" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/girepository-1.0")

add_custom_target(src ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${ayatana_appindicator_girver}-0.1.typelib")

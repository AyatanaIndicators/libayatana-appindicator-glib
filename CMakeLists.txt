cmake_minimum_required (VERSION 3.13)
project (libayatana-appindicator-glib VERSION 2.0.1 LANGUAGES C)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Options

option (ENABLE_TESTS "Enable all tests and checks" OFF)
option (ENABLE_WERROR "Treat all build warnings as errors" OFF)
option (ENABLE_COVERAGE "Enable coverage reports (enables all tests and checks)" OFF)

if (ENABLE_COVERAGE)
    set (ENABLE_TESTS ON)
    set (CMAKE_BUILD_TYPE "Coverage")
else()
    set (CMAKE_BUILD_TYPE "Release")
endif()

if (ENABLE_WERROR)
    add_definitions ("-Werror")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions ("-Weverything")
else ()
    add_definitions ("-Wall")
endif ()

# Prerequisites

include (GNUInstallDirs)
find_package (PkgConfig REQUIRED)
pkg_check_modules (DEPS REQUIRED gio-2.0 gobject-2.0)

# Build

add_subdirectory (src)
add_subdirectory (bindings)
add_subdirectory (doc)

if (ENABLE_TESTS)
    include (CTest)
    enable_testing ()
    add_subdirectory (tests)
    add_subdirectory (examples)

    if (ENABLE_COVERAGE)
        find_package (CoverageReport)
        ENABLE_COVERAGE_REPORT (TARGETS "ayatana-appindicator" TESTS "test-libappindicator" "test-libappindicator-dbus-client" "test-libappindicator-dbus-server" "test-libappindicator-status-client" "test-libappindicator-status-server" FILTER /usr/include ${CMAKE_BINARY_DIR}/*)
    endif ()
endif ()

# Info

message (STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message (STATUS "Build with -Werror: ${ENABLE_WERROR}")
message (STATUS "Unit tests: ${ENABLE_TESTS}")


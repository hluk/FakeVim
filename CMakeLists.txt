cmake_minimum_required(VERSION 3.0)
project(fakevim VERSION 0.0.1)

# Library name
set(bin fakevim)

# Public headers
set(${bin}_public_headers
    fakevim/fakevimactions.h
    fakevim/fakevimhandler.h
    )

# Source files common for all platforms
set(${bin}_sources
    fakevim/fakevimactions.cpp
    fakevim/fakevimhandler.cpp
    ${${bin}_public_headers}
    )

# Required pkg-config packages
set(${bin}_pkg_config_requires)

include(cmake/library.cmake)
include(cmake/qt.cmake)
include(cmake/pkg-config.cmake)

# Files with Q_OBJECT macros to pass to moc utility
set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt6_wrap_cpp(${bin}_mocced "fakevim/fakevimhandler.h")
target_sources(${bin} PRIVATE ${${bin}_mocced})

target_compile_definitions(${bin} PRIVATE
    QT_NO_CAST_TO_ASCII
    QT_RESTRICTED_CAST_FROM_ASCII
    UTILS_STATIC_LIBRARY
)

option(BUILD_TESTS "Build tests")
if (BUILD_TESTS)
    message(STATUS "Building tests")

    find_package(Qt6Test REQUIRED)

    add_executable(fakevim_test
        tests/fakevim_test.cpp
        tests/fakevimplugin.h
        example/editor.cpp
    )
    set_property(TARGET fakevim_test PROPERTY AUTOMOC ON)
    target_link_libraries(fakevim_test fakevim Qt6::Widgets Qt6::Test)

    target_include_directories(fakevim_test PRIVATE
        ${CMAKE_SOURCE_DIR}/fakevim
        ${CMAKE_SOURCE_DIR}
        )

    add_test(fakevim_test fakevim_test)
    enable_testing()
endif()

option(BUILD_EXAMPLE "Build example")
if (BUILD_EXAMPLE)
    message(STATUS "Building example")

    add_executable(fakevim_example example/main.cpp example/editor.cpp)
    set_property(TARGET fakevim_example PROPERTY AUTOMOC ON)

    target_link_libraries(fakevim_example fakevim)
    target_link_libraries(fakevim_example Qt6::Widgets)
endif()

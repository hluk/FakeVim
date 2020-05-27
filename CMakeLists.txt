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
    fakevim/utils/qtcassert.cpp
    ${${bin}_public_headers}
    )

# Required pkg-config packages
set(${bin}_pkg_config_requires)

include(cmake/cxx11.cmake)
include(cmake/library.cmake)
include(cmake/qt.cmake)
include(cmake/pkg-config.cmake)

# Files with Q_OBJECT macros to pass to moc utility
set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt5_wrap_cpp(${bin}_mocced "fakevim/fakevimhandler.h")
# Generate moc file with correct file name.
qt5_generate_moc("fakevim/fakevimhandler.cpp" "fakevimhandler.moc")
target_sources(${bin} PRIVATE ${${bin}_mocced} "fakevimhandler.moc")

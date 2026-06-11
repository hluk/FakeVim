cmake_minimum_required(VERSION 2.8.8)
project(fakevim_example)

find_package(fakevim REQUIRED)
find_package(Qt6Widgets REQUIRED)

set(bin fakevim_example)
add_executable(${bin} main.cpp editor.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt6_generate_moc("editor.cpp" "editor.moc")
target_sources(${bin} PRIVATE "editor.moc")

target_link_libraries(${bin} fakevim)
target_link_libraries(${bin} Qt6::Widgets)

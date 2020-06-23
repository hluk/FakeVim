cmake_minimum_required(VERSION 3.0)
project(fakevim_tests)

find_package(fakevim REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Test REQUIRED)

set(bin fakevim_tests)
add_executable(${bin} fakevim_test.cpp ../example/editor.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt5_wrap_cpp(${bin}_mocced "fakevimplugin.h")
qt5_generate_moc("../example/editor.cpp" "editor.moc")
target_sources(${bin} PRIVATE ${${bin}_mocced} "editor.moc")

target_include_directories(${bin} PRIVATE ../fakevim)

target_link_libraries(${bin} fakevim)
target_link_libraries(${bin} Qt5::Widgets Qt5::Test)
add_test(${bin} ${bin})
enable_testing()

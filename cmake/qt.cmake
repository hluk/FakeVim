find_package(Qt6Widgets REQUIRED)

include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_link_libraries(${bin} Qt6::Widgets)

set(${bin}_pkg_config_requires ${${bin}_pkg_config_requires} Qt6Widgets)

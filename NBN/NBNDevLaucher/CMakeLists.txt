## NBNDevLaucher
project(NBNDevLaucher)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Xml)


set(CMAKE_INCLUDE_CURRENT_DIR 1)

qt5_wrap_ui(UIS NBNDevLauncherMainWindow.ui)


add_executable(${PROJECT_NAME} ${SRC_LIST} ${UIS})


qt5_use_modules(${PROJECT_NAME} Widgets Xml)

cmake_minimum_required(VERSION 3.16)

project(VideoReader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
find_package(OpenCV REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# AUTOUIC must stay off: the project has no Qt Designer forms, and it treats the
# hand-written header ui_controller.h as a generated form for a missing
# controller.ui, which breaks the build.
set(CMAKE_AUTOUIC OFF)

add_subdirectory(src)

option(VIDEOREADER_BUILD_TESTS "Build the unit tests" ON)
if(VIDEOREADER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

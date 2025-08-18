#!/bin/bash

echo "Building Simple Image Player with MOC..."

mkdir -p build_simple

MOC_PATH=$(find /usr -name "moc" -type f 2>/dev/null | grep qt6 | head -1)
if [ -z "$MOC_PATH" ]; then
    echo "Error: Qt MOC not found!"
    exit 1
fi

echo "Using MOC: $MOC_PATH"

echo "Running MOC on header files..."
$MOC_PATH headers/ui/mainwindow.h -o build_simple/moc_mainwindow.cpp
$MOC_PATH headers/core/processors/image_processor.h -o build_simple/moc_image_processor.cpp
$MOC_PATH headers/core/controllers/playback_controller.h -o build_simple/moc_playback_controller.cpp
$MOC_PATH headers/ui/ui_controller.h -o build_simple/moc_ui_controller.cpp

if [ $? -ne 0 ]; then
    echo "MOC failed!"
    exit 1
fi

echo "MOC successful, generated MOC files"

echo "Compiling with g++..."
g++ -std=c++17 \
    -I/usr/include/x86_64-linux-gnu/qt6 \
    -I/usr/include/x86_64-linux-gnu/qt6/QtCore \
    -I/usr/include/x86_64-linux-gnu/qt6/QtWidgets \
    -I/usr/include/x86_64-linux-gnu/qt6/QtGui \
    -I/usr/include/opencv4 \
    -Iheaders \
    -Ibuild_simple \
    src/core/main.cpp \
    src/ui/mainwindow.cpp \
    src/core/processors/image_processor.cpp \
    src/core/controllers/playback_controller.cpp \
    src/ui/ui_controller.cpp \
    build_simple/moc_mainwindow.cpp \
    build_simple/moc_image_processor.cpp \
    build_simple/moc_playback_controller.cpp \
    build_simple/moc_ui_controller.cpp \
    -o build_simple/SimpleImagePlayer \
    -lQt6Core -lQt6Widgets -lQt6Gui \
    -lopencv_core -lopencv_imgproc -lopencv_imgcodecs \
    -pthread \
    -fPIC \
    -DQT_WIDGETS_LIB \
    -DQT_CORE_LIB \
    -DQT_GUI_LIB

if [ $? -eq 0 ]; then
    echo "Build successful! Executable created at build_simple/SimpleImagePlayer"
    echo "You can run it with: ./build_simple/SimpleImagePlayer"
else
    echo "Build failed!"
    exit 1
fi

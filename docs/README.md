# Simple Image Player

A minimal Qt-based application for viewing image sequences.

## Features

- **Load Folder**: Select a folder containing images (JPG, PNG, BMP)
- **Play/Pause**: Automatically cycle through images
- **FPS Control**: Adjust playback speed (1-60 FPS)
- **Simple UI**: Clean, minimal interface

## Building

```bash
./build_simple_moc.sh
```

## Running

```bash
./build_simple/SimpleImagePlayer
```

## Requirements

- Qt6 (Core, Widgets, Gui)
- OpenCV4
- C++17 compiler

## How It Works

1. Click "Load Folder" to select a directory with images
2. Images are loaded and displayed automatically
3. Set your desired FPS using the FPS control (1-60 FPS)
4. Use "Play" to start automatic cycling through images
5. Images loop back to the beginning when reaching the end

## Project Structure

```
videoReader/
├── src/
│   ├── main.cpp              # Application entry point
│   └── mainwindow.cpp        # Main window implementation
├── include/
│   └── mainwindow.h          # Main window header
├── build_simple/
│   └── SimpleImagePlayer     # Compiled executable
├── build_simple_moc.sh       # Build script
└── docs/
    └── README.md             # This file
```

## Code Overview

- **MainWindow**: Main application window with image display and controls
- **QTimer**: Handles automatic frame updates during playback
- **FPS Control**: User-adjustable playback speed (1-60 FPS)
- **Image Loading**: Supports JPG, PNG, and BMP formats

## Simplifications Made

- Removed complex speed controls
- Removed frame sliders and spinboxes
- Removed complex UI grouping
- Simplified image loading (no lazy loading)
- Removed manual frame navigation (next button)
- Cleaner, more focused codebase
- **Added**: Simple FPS control (1-60 FPS range)
- **Removed**: All unnecessary build files and CMake complexity

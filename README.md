# VideoReader - Simple Image Player

A Qt-based application for viewing image sequences with low coupling architecture.

## Project Structure

```
videoReader/
├── core/                          # Core application components
│   ├── main.cpp                   # Application entry point
│   ├── controllers/               # Playback control logic
│   │   ├── playback_controller.h
│   │   ├── playback_controller.cpp
│   │   └── CMakeLists.txt
│   ├── processors/                # Image processing logic
│   │   ├── image_processor.h
│   │   ├── image_processor.cpp
│   │   └── CMakeLists.txt
│   └── CMakeLists.txt
├── ui/                            # User interface components
│   ├── mainwindow.h               # Main window class
│   ├── mainwindow.cpp
│   ├── ui_controller.h            # UI management
│   ├── ui_controller.cpp
│   └── CMakeLists.txt
├── build_scripts/                  # Build automation
│   └── build_simple_moc.sh        # Custom build script
├── docs/                          # Documentation
│   └── LOW_COUPLING_ARCHITECTURE.md
├── CMakeLists.txt                 # Main CMake configuration
└── README.md                      # This file
```

## Architecture

The project follows a **low coupling** architecture with clear separation of concerns:

- **Core**: Contains the main application logic and business components
- **UI**: Handles all user interface and presentation logic
- **Controllers**: Manage playback timing and frame progression
- **Processors**: Handle image loading, processing, and management

## Building

### Using CMake (Recommended)

```bash
mkdir build && cd build
cmake ..
make
```

### Using Custom Build Script

```bash
./build_scripts/build_simple_moc.sh
```

## Features

- Load folders containing image sequences
- Play/pause functionality with adjustable FPS (1-1000)
- Support for JPG, JPEG, PNG, and BMP formats
- Automatic looping through images
- Clean, modular architecture

## Requirements

- Qt6 (Core, Widgets, Gui)
- OpenCV4
- C++17 compiler

## Running

```bash
./build_simple/SimpleImagePlayer
```

## Benefits of the New Structure

1. **Logical Organization**: Files are grouped by functionality
2. **Easy Navigation**: Clear separation between core logic and UI
3. **Modular Builds**: Each component can be built independently
4. **Scalability**: Easy to add new features in appropriate directories
5. **Maintainability**: Clear dependencies and responsibilities

# VideoReader - Simple Image Player

A Qt-based application for viewing image sequences with low coupling architecture.

## Project Structure

```
videoReader/
├── headers/                       # Header files (.h)
│   ├── core/                      # Core application headers
│   │   ├── controllers/           # Playback control headers
│   │   │   └── playback_controller.h
│   │   └── processors/            # Image processing headers
│   │       └── image_processor.h
│   └── ui/                        # User interface headers
│       ├── mainwindow.h           # Main window class
│       └── ui_controller.h        # UI management
├── src/                           # Source files (.cpp)
│   ├── core/                      # Core application sources
│   │   ├── main.cpp               # Application entry point
│   │   ├── controllers/           # Playback control sources
│   │   │   └── playback_controller.cpp
│   │   └── processors/            # Image processing sources
│   │       └── image_processor.cpp
│   └── ui/                        # User interface sources
│       ├── mainwindow.cpp         # Main window implementation
│       └── ui_controller.cpp      # UI management
├── build_scripts/                  # Build automation
│   └── build_simple_moc.sh        # Custom build script
├── docs/                          # Documentation
│   └── LOW_COUPLING_ARCHITECTURE.md
├── CMakeLists.txt                 # Main CMake configuration
└── README.md                      # This file
```

## Architecture

The project follows a **low coupling** architecture with clear separation of concerns:

- **Headers**: Contains all interface definitions and class declarations
- **Source**: Contains all implementation files
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

1. **Logical Organization**: Headers and sources are clearly separated
2. **Easy Navigation**: Clear distinction between interface and implementation
3. **Modular Builds**: Each component can be built independently
4. **Scalability**: Easy to add new features in appropriate directories
5. **Maintainability**: Clear dependencies and responsibilities
6. **Standard Layout**: Follows common C++ project organization patterns

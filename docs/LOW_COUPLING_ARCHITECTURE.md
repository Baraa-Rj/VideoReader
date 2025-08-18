# Low Coupling Architecture

## Overview

This document describes the refactored architecture that achieves **low coupling** by separating concerns into distinct, focused components.

## Before vs After

### Before (High Coupling)
- **Single Responsibility Violation**: `MainWindow` handled UI, image processing, playback control, and file management
- **Tight Dependencies**: Direct OpenCV usage, timer management, and file operations embedded in UI class
- **Hard to Test**: Business logic mixed with presentation logic
- **Hard to Modify**: Changes to one aspect could affect multiple areas

### After (Low Coupling)
- **Single Responsibility**: Each class has one clear purpose
- **Loose Dependencies**: Components communicate through well-defined interfaces
- **Easy to Test**: Business logic separated from UI logic
- **Easy to Modify**: Changes isolated to specific components

## Architecture Components

### 1. ImageProcessor
**Responsibility**: Image loading, processing, and management
- Loads images from folders
- Handles OpenCV operations
- Manages image paths and validation
- Emits signals for state changes

**Dependencies**: OpenCV, Qt Core
**Coupling Level**: Low (only depends on external libraries)

### 2. PlaybackController
**Responsibility**: Playback timing and frame progression
- Manages playback state (play/pause/stop)
- Controls frame rate and timing
- Handles frame progression logic
- Emits signals for playback events

**Dependencies**: Qt Core, Qt Timer
**Coupling Level**: Low (only depends on Qt timer)

### 3. UIController
**Responsibility**: UI creation and management
- Creates and manages UI widgets
- Handles layout management
- Updates UI state based on signals
- Emits signals for user interactions

**Dependencies**: Qt Widgets
**Coupling Level**: Low (only depends on Qt widgets)

### 4. MainWindow
**Responsibility**: Component coordination and high-level flow
- Creates and manages controllers
- Connects signals and slots between components
- Handles high-level application logic
- Manages window-level operations

**Dependencies**: All controllers (but through interfaces)
**Coupling Level**: Medium (coordinates components but doesn't implement their logic)

## Communication Pattern

```
User Action → UIController → Signal → MainWindow → Controller → Signal → UI Update
```

- **Signals and Slots**: Components communicate through Qt's signal-slot mechanism
- **Interface Contracts**: Each component exposes well-defined public interfaces
- **Event-Driven**: Components react to events rather than calling each other directly

## Benefits of Low Coupling

### 1. **Maintainability**
- Changes to image processing don't affect UI
- Playback logic can be modified without touching image handling
- UI changes don't impact business logic

### 2. **Testability**
- Each component can be tested in isolation
- Mock objects can easily replace dependencies
- Unit tests focus on single responsibilities

### 3. **Reusability**
- `ImageProcessor` can be used in other applications
- `PlaybackController` can work with different image sources
- `UIController` can be adapted for different UI layouts

### 4. **Extensibility**
- Easy to add new image formats to `ImageProcessor`
- Simple to add new playback modes to `PlaybackController`
- UI can be enhanced without affecting other components

## Design Principles Applied

### 1. **Single Responsibility Principle (SRP)**
Each class has one reason to change:
- `ImageProcessor`: Image operations only
- `PlaybackController`: Playback logic only
- `UIController`: UI management only
- `MainWindow`: Component coordination only

### 2. **Dependency Inversion Principle (DIP)**
High-level modules don't depend on low-level modules:
- `MainWindow` depends on controller interfaces, not implementations
- Components depend on abstractions, not concrete classes

### 3. **Open/Closed Principle (OCP)**
Open for extension, closed for modification:
- New image formats can be added without changing existing code
- New UI controls can be added without affecting business logic

### 4. **Interface Segregation Principle (ISP)**
Clients don't depend on interfaces they don't use:
- Each controller exposes only the methods needed by its clients
- Clean, focused interfaces for each component

## Future Enhancements

With this architecture, it's easy to add:

1. **New Image Formats**: Extend `ImageProcessor` with new format handlers
2. **Advanced Playback**: Add effects, transitions, or speed controls to `PlaybackController`
3. **Different UI Themes**: Create new `UIController` implementations
4. **Plugin System**: Allow external components to extend functionality
5. **Network Support**: Add remote image loading without affecting UI

## Conclusion

The refactored architecture achieves **low coupling** through:
- **Clear separation of concerns**
- **Interface-based communication**
- **Event-driven architecture**
- **Single responsibility design**

This makes the codebase more maintainable, testable, and extensible while preserving all original functionality.

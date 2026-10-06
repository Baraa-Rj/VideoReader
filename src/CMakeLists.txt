# Headers declaring Q_OBJECT must be listed as target sources: they live in a
# different directory tree from their .cpp, so AUTOMOC cannot discover them on
# its own and no meta-object code would be generated at all.
add_library(controllers
    core/controllers/playback_controller.cpp
    ${CMAKE_SOURCE_DIR}/headers/core/controllers/playback_controller.h
)

target_include_directories(controllers PUBLIC
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(controllers
    Qt6::Core
)

add_library(processors
    core/processors/image_processor.cpp
    ${CMAKE_SOURCE_DIR}/headers/core/processors/image_processor.h
)

target_include_directories(processors PUBLIC
    ${CMAKE_SOURCE_DIR}/headers
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(processors
    Qt6::Core
    Qt6::Gui
    ${OpenCV_LIBS}
)

add_library(ui
    ui/ui_controller.cpp
    ui/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/headers/ui/ui_controller.h
    ${CMAKE_SOURCE_DIR}/headers/ui/mainwindow.h
)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(ui
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    controllers
    processors
)

add_executable(SimpleImagePlayer
    core/main.cpp
)

target_include_directories(SimpleImagePlayer PRIVATE
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(SimpleImagePlayer
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    ${OpenCV_LIBS}
    controllers
    processors
    ui
)

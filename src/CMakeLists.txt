add_library(controllers
    core/controllers/playback_controller.cpp
)

target_include_directories(controllers PUBLIC
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(controllers
    Qt6::Core
)

add_library(processors
    core/processors/image_processor.cpp
)

target_include_directories(processors PUBLIC
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(processors
    Qt6::Core
    ${OpenCV_LIBS}
)

add_library(ui
    ui/ui_controller.cpp
    ui/mainwindow.cpp
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

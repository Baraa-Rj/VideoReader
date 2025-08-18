
add_executable(SimpleImagePlayer
    main.cpp
    mainwindow.cpp
    image_processor.cpp
    playback_controller.cpp
    ui_controller.cpp
)

target_link_libraries(SimpleImagePlayer
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    ${OpenCV_LIBS}
)

target_include_directories(SimpleImagePlayer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(SimpleImagePlayer PROPERTIES
    OUTPUT_NAME "SimpleImagePlayer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

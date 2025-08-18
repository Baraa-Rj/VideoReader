add_executable(SimpleImagePlayer
    main.cpp
)

target_include_directories(SimpleImagePlayer PRIVATE
    ${CMAKE_SOURCE_DIR}/core/controllers
    ${CMAKE_SOURCE_DIR}/core/processors
    ${CMAKE_SOURCE_DIR}/ui
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

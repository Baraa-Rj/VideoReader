add_library(ui
    ui_controller.cpp
    mainwindow.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core/controllers
    ${CMAKE_SOURCE_DIR}/core/processors
)

target_link_libraries(ui
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    controllers
    processors
)

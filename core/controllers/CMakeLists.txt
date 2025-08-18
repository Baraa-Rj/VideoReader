add_library(controllers
    playback_controller.cpp
)

target_include_directories(controllers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(controllers
    Qt6::Core
)

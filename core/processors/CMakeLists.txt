add_library(processors
    image_processor.cpp
)

target_include_directories(processors PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(processors
    Qt6::Core
    ${OpenCV_LIBS}
)

# Include directory CMakeLists.txt

# This directory contains header files
# No targets to build here, just configuration

message(STATUS "Include directory configuration loaded")

# Set include directory properties
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
)

# List header files for reference
file(GLOB HEADERS "*.h")
message(STATUS "Header files found: ${HEADERS}")

# Set up include directories for the project
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.10)
project(interlaced_core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)

# Create interface library for header-only library
add_library(interlaced_core INTERFACE)

# Specify include directories for the interface library
target_include_directories(interlaced_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Tests removed: CTest disabled and tests/ directory removed.

# Install rules (for distribution)
install(TARGETS interlaced_core
        EXPORT interlaced_core-targets
        INCLUDES DESTINATION include
)
install(DIRECTORY include/interlaced_core/
        DESTINATION include
)

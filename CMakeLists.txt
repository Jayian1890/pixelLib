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

# Enable testing
enable_testing()

# Fetch GTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        03597a01ee50ed33e9dfd640b249b4be3799d395
)
# For Windows: Prevent overriding the parent project's compiler/linker on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add tests subdirectory so test executables are built
add_subdirectory(tests)

# Install rules (for distribution)
install(TARGETS interlaced_core
        EXPORT interlaced_core-targets
        INCLUDES DESTINATION include
)
install(DIRECTORY include/interlaced_core/
        DESTINATION include
)

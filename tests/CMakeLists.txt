# Prefer target-based include directories (set per-target below)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)
# Prefer the provided GTest targets (GTest::gtest_main) from the downloaded project

# Create test executable for logging
add_executable(logging_test logging_test.cc)

target_include_directories(logging_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(logging_test
    PRIVATE
    interlaced_core
    GTest::gtest_main
)

# Add test
add_test(NAME logging_test COMMAND logging_test)

# Create test executable for json
add_executable(json_test json_test.cc)

target_include_directories(json_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(json_test
    PRIVATE
    interlaced_core
    GTest::gtest_main
)

# Add test
add_test(NAME json_test COMMAND json_test)
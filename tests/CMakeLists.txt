# Prefer target-based include directories (set per-target below)

# Create test executable for logging
add_executable(logging_test logging_test.cc)

target_include_directories(logging_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link only gtest_main (it already pulls in gtest) to avoid duplicate-library warnings
target_link_libraries(logging_test
    PRIVATE
    gtest_main
    interlaced_core
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
    gtest_main
    interlaced_core
)

# Add test
add_test(NAME json_test COMMAND json_test)
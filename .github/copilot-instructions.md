# Interlaced Core Copilot Instructions

## Architecture Overview
Interlaced Core is a header-only C++ library providing modular utilities for filesystem operations, JSON handling, logging, and networking. The library is structured as an interface library in CMake, with all headers in the `include/` directory. Key components include:
- `filesystem.hpp`: File system utilities
- `json.hpp`: JSON parsing and manipulation
- `logging.hpp`: Logging mechanisms
- `network.hpp`: Network-related functions

The "why" behind the header-only design is to simplify integration without linking, allowing users to include only necessary headers.

## Build System
- Use CMake 3.10+ with C++17 standard.
- Root `CMakeLists.txt` sets up the interface library `interlaced_core` and includes `include/` directory.
- Build command: `cmake -S . -B build && cmake --build build`
- Compile commands are exported for IDE integration.

## Testing
This repository does not include unit tests in this branch.

## Developer Workflows
- CI/CD via GitHub Actions in `.github/workflows/build-test.yml` (assumes standard build and test steps).
- Debug builds: Add `-DCMAKE_BUILD_TYPE=Debug` to cmake command.
- Dependencies: no third-party testing framework is required; everything builds with CMake + the standard library.

## Conventions and Patterns
- Header files use `.hpp` extension, placed directly in `include/`.
- Test files use `.cc` extension in `tests/`.
- CMake structure: Root CMakeLists for main library, separate for tests.
- Example pattern: Interface library with `target_include_directories` for public headers, e.g., `$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>`.

## Integration Points
- Tests are implemented using GoogleTest under `tests/` and link to `GTest::gtest_main`.
- No cross-component communication beyond header includes; each module is self-contained.
- Installation: `cmake --install build` installs headers to `include/`.

## Rules & Restrictions
- No third-party dependencies are required; the testing helpers are part of this repository.
- Maintain header-only design; no compiled binaries.
- Follow C++17 standard; no newer features.
---
trigger: always_on
---

# pixelLib Project Rules

## Project Overview
pixelLib is a modern C++23 library providing core utilities for filesystem operations, JSON parsing, logging, and networking functionality. The project uses a modular architecture with comprehensive testing.

## Quick Start Commands
- **Build all**: `make` (uses C++23, builds tests with coverage)
- **Debug build**: `make DEBUG=1`
- **Run tests**: `make run-tests`
- **Coverage report**: `make coverage`
- **Clean build**: `make clean`

## Architecture & Structure
- **Headers**: All public headers in `include/pixellib/core/`
- **Source**: Implementation in corresponding directories
- **Tests**: All tests in `tests/` using doctest framework
- **Third-party**: doctest header in `third-party/doctest/`

## Key Development Rules

### Language & Standards
- **C++23 required** - All code must compile with `-std=c++23`
- **Compiler flags**: `-Wall -Wextra -Wpedantic` enforced
- **Headers**: Use `#include "third-party/doctest/doctest.h"` for tests

### Testing Conventions
- **Framework**: doctest only (no custom test framework)
- **Test main**: Single test main in `tests/doctest_main.cpp`
- **Test macros**: Use `TEST_SUITE()`, `TEST_CASE()`, `CHECK()`, `REQUIRE()`, `CHECK_FALSE()`, `CHECK_NOTHROW()`, `CHECK_THROWS_AS()`
- **No TEST_SUITE**: Remove wrapper blocks, use individual test cases
- **No Approx**: Use direct comparisons instead of floating-point approximations

### Module Organization
- **Filesystem**: `pixellib::core::filesystem` namespace
- **JSON**: `pixellib::core::json` namespace  
- **Logging**: `pixellib::core::logging` namespace
- **Network**: `pixellib::core::network` namespace

### Build System
- **Makefile-based**: Uses GNU make with compiler detection
- **Cross-platform**: Supports macOS (clang), Linux (gcc/clang), Windows (mingw)
- **Coverage**: Supports both LLVM and GCC coverage tools
- **Dependencies**: Minimal external deps, only doctest (vendored)

### Code Style
- **Namespace**: All code in `pixellib::core::*` namespaces
- **Error handling**: Use appropriate exception types and return codes
- **RAII**: Follow RAII principles for resource management
- **Const correctness**: Use const where appropriate

## Testing Workflow
1. Write tests using doctest macros in `tests/test_*.cc`
2. Include `#include "third-party/doctest/doctest.h"` at top
3. Build with `make` or `make DEBUG=1`
4. Run tests with `make run-tests`
5. Check coverage with `make coverage`

## Common Patterns
- **File operations**: Use `pixellib::core::filesystem::FileSystem`
- **JSON parsing**: Use `pixellib::core::json::JSON` class
- **Logging**: Use `pixellib::core::logging::Logger` static methods
- **Network**: Use `pixellib::core::network::Network` static methods

## File Locations
- Main headers: `include/pixellib/core/{module}.hpp`
- Test files: `tests/test_{module}.cc`
- Build artifacts: `build/` directory
- Coverage reports: `build/coverage/`

## Important Notes
- No external dependencies beyond doctest
- Single test main entry point
- C++23 features available (concepts, ranges, etc.)
- Cross-platform compatibility required
- Comprehensive test coverage expected

## Important Rules
- ALWAYS read `.github/*.instructions` before making changes
- ALWAYS fix lint warnings before submitting a code change
- ALWAYS fix doctest errors before submitting a code change
- ALWAYS fix warnings before submitting a code change
- ALWAYS fix errors before submitting a code change
- ALWAYS push changes to github after completing a task
- NEVER use mock code for any reason, always use real code
- ALWAYS convert mock code to real code
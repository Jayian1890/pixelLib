# AI Development Guide for pixelLib

## Project Overview
pixelLib is a **header-only C++23 library** providing cross-platform utilities for filesystem operations, JSON parsing, logging, and networking. All functionality is implemented in headers under `include/` with no separate `.cpp` files.

## Project Structure
```
pixelLib/
├── include/              # All implementations (header-only)
│   ├── filesystem.hpp    # Cross-platform file/directory operations
│   ├── json.hpp          # DOM parser with strict validation
│   ├── logging.hpp       # Thread-safe logging with rotation
│   └── network.hpp       # HTTP/DNS/networking utilities
├── tests/                # doctest-based unit tests
│   ├── doctest_main.cpp  # Test runner entry point
│   ├── test_*.cc         # One test file per module
│   └── test_main.cc      # Basic sanity checks
├── third-party/          # Header-only dependencies
│   └── doctest/          # doctest.h testing framework
└── .github/              # Instructions and CI workflows
```

## Development Guidelines

### Code Style
- **C++ Standard**: C++23
- **Architecture**: Pure header-only library (no `.cpp` implementation files)
- **Namespace**: `pixellib::core::<module>` (e.g., `pixellib::core::filesystem`)
- **Header Guards**: `#ifndef PIXELLIB_CORE_<MODULE>_HPP` with `#define`
- **Naming**: `snake_case` for functions/variables, `PascalCase` for types
- **Platform Code**: Use preprocessor guards (`#ifdef _WIN32`, `#ifdef __APPLE__`)
- **Documentation**: Block comments with copyright header; inline comments for complex logic

### Testing Requirements
- **Framework**: doctest (header-only in `third-party/doctest/`)
- **Coverage Target**: 95%+ code coverage
- **Test Organization**:
  - One test file per module: `tests/test_<module>.cc`
  - Entry point: `tests/doctest_main.cpp` with `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`
  - Group related tests with `TEST_SUITE("Module Name")`
  - Use `TEST_CASE("functionality description")` for individual tests
  - Prefer `CHECK()` over `REQUIRE()` for non-fatal assertions
- **Test Mode**: Set `PIXELLIB_TEST_MODE=1` environment variable for deterministic offline tests

### Build System
- **Build Tool**: Makefile with cross-platform support (macOS, Linux, Windows/MinGW)
- **Compiler**: Auto-detects clang++ or g++ (set `CXX` to override)
- **Key Make Targets**:
  - `make build_test` - Build test binary with coverage instrumentation
  - `make run_test` - Run all tests
  - `make coverage` - Generate HTML and LCOV coverage reports in `build/coverage/`
  - `make compile-commands` - Generate `compile_commands.json` for clangd
  - `make clean` - Remove all build artifacts
  - `make clang-tidy` - Run static analysis
- **Output**: All artifacts in `build/` directory (excluded from version control)

### Commit Guidelines
- **Format**: Conventional Commits (feat:, fix:, docs:, etc.)
- **Scope**: Keep commits focused and atomic
- **Messages**: Clear, concise, and reference issues when applicable

## AI Assistant Guidelines

### Before Making Changes
1. Check `git status` for uncommitted changes
2. Pull latest changes from main branch
3. Run tests to ensure a clean baseline

### Development Workflow
1. **For New Features**:
   - Add tests in `tests/test_<module>.cc` using `TEST_CASE`
   - Implement in header file under `include/` (no `.cpp` files)
   - Run `make run_test` to verify
   - Check coverage with `make coverage` (target: 95%+)
   - Run `make clang-tidy` to catch issues

2. **For Bug Fixes**:
   - Write failing test that reproduces the bug
   - Fix in the appropriate header file
   - Verify with `make run_test`
   - Ensure coverage remains above 95%

3. **For Refactoring**:
   - Verify tests pass before starting: `make run_test`
   - Make incremental changes to headers
   - Re-run tests after each change
   - Regenerate `compile_commands.json`: `make compile-commands`

### Important Rules
- **NEVER** create `.cpp` implementation files (this is a header-only library)
- **ALWAYS** implement all functions inline in header files
- **ALWAYS** use `pixellib::core::<module>` namespace pattern
- **ALWAYS** run `make run_test` before committing
- **ALWAYS** maintain 95%+ test coverage (check with `make coverage`)
- **ALWAYS** fix clang-tidy warnings: `make clang-tidy`
- **ALWAYS** use platform guards for OS-specific code (`#ifdef _WIN32`, etc.)
- **ALWAYS** push changes to GitHub after completing a task

### Documentation in Code
- **File Headers**: Include MIT license and copyright block
- **Comments**: Use inline `//` comments for complex logic
- **Block Comments**: Use `/* */` for multi-line explanations
- **README.md**: Keep feature list and usage examples up-to-date
- **Public API**: Document expected behavior, parameters, and return values in comments

## Common Tasks

### Running Tests
```bash
# Build and run all tests
make run_test

# Just build tests (with coverage instrumentation)
make build_test

# Run tests with offline mode
PIXELLIB_TEST_MODE=1 ./build/unit_tests

# Generate coverage report (HTML + LCOV)
make coverage
# View: open build/coverage/index.html
```

### Development Commands
```bash
# Generate compile_commands.json for clangd/IDE
make compile-commands

# Run static analysis
make clang-tidy

# Auto-fix clang-tidy issues
make clang-tidy-fix

# Clean all build artifacts
make clean
```

## Key Architecture Patterns

### Header-Only Implementation
All functionality in `include/*.hpp` files:
```cpp
namespace pixellib::core::<module> {
  class MyClass {
  public:
    static ReturnType function(const std::string& param) {
      // Full implementation here, not just declaration
      return result;
    }
  };
}
```

### Cross-Platform Code
Use preprocessor guards for platform-specific implementations:
```cpp
#ifdef _WIN32
  // Windows implementation
#elif defined(__APPLE__)
  // macOS implementation  
#else
  // Linux/Unix implementation
#endif
```

### Error Handling
- Return booleans for success/failure
- Use empty string/vector for failure cases
- Provide clear error context in comments

## Troubleshooting

### Build Issues
- **Coverage tools missing**: Install `gcovr` (coverage) and `lcov` (LCOV format)
- **Compiler not found**: Set `CXX=clang++` or `CXX=g++`
- **Test failures**: Check if `PIXELLIB_TEST_MODE=1` is needed for network tests

### IDE Setup
- Run `make compile-commands` to generate `compile_commands.json`
- Restart language server if needed (clangd task available)

## Additional Instructions

For detailed guidelines on specific components:

- [Header Files](instructions/header-files.instructions.md)
- [Makefile](instructions/makefile.instructions.md)
- [Test Files](instructions/test-files.instructions.md)

## License
MIT License

Copyright (c) 2025 Interlaced Pixel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---
*Last updated: February 2, 2026 - This document is maintained by the project maintainers.*

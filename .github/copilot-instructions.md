# AI Development Guide for C++ Projects

## Project Overview
This document provides AI assistants with the necessary context and guidelines for contributing to this C++ project. It covers project structure, coding standards, and development workflows.

## Project Structure
```
project/
├── include/              # Public headers (single include style)
│   └── project/          # Project namespace
│       └── core/         # Core functionality modules
├── src/                  # Implementation files
│   └── core/             # Core implementation files
├── tests/               # Unit tests
│   ├── test_*.cc        # Test files (one per module)
│   └── test_main.cc     # Test runner
├── third-party/         # Dependencies (git submodules or headers)
└── .github/             # CI/CD and project documentation
```

## Development Guidelines

### Code Style
- **C++ Standard**: C++23
- **Formatting**: clang-format (config in `.clang-format`)
- **Naming**: `snake_case` for variables/functions, `PascalCase` for types
- **Headers**: Header-only libraries preferred
- **Documentation**: All code must be documented using Doxygen-style comments. This includes:
  - File headers with `@file`, `@brief`, `@author`, and `@date`
  - Namespace documentation with `@brief`
  - Class/struct documentation with `@brief`, `@details`, `@tparam` for templates
  - Function documentation with `@brief`, `@param`, `@return`, and `@throw`
  - Variable documentation with `@brief`
  - Group related functionality with `@defgroup` and `@{\n...\n@}`

### Testing Requirements
- **Framework**: doctest (header-only)
- **Coverage**: 75%+ test coverage target
- **Test Organization**:
  - One test file per module
  - Group related tests in `TEST_SUITE`
  - Use `TEST_CASE` for test groups
  - Prefer `CHECK` over `REQUIRE` for multiple assertions
- **No Mocking**: Mock code is strictly forbidden. Write tests against real implementations only.

### Build System
- **Build Tool**: Make (GNU Make required)
- **Build Modes**:
  - Debug: `make DEBUG=1` (defaults to `-O0 -g`)
  - Release: `make` (defaults to `-O3 -DNDEBUG`)
- **Dependencies**: Document all third-party deps in `third-party/`
- **Output**: Build artifacts go to `build/` directory

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
   - Add tests first (TDD approach)
   - Implement the feature
   - Ensure all tests pass
   - Update documentation

2. **For Bug Fixes**:
   - Add a failing test that reproduces the bug
   - Fix the bug
   - Verify the test passes
   - Add regression tests

3. **For Refactoring**:
   - Ensure comprehensive test coverage
   - Make changes incrementally
   - Verify all tests pass after each change

### Important Rules
- **ALWAYS** fix lint warnings before submitting a code change
- **ALWAYS** fix doctest errors before submitting a code change
- **ALWAYS** fix warnings before submitting a code change
- **ALWAYS** fix errors before submitting a code change
- **ALWAYS** push changes to GitHub after completing a task
- **NEVER** use mock code for any reason, always use real code
- **ALWAYS** convert mock code to real code

### Documentation Requirements
- **Doxygen Comments Required For**:
  - All header files
  - Public and protected class members
  - Function declarations and definitions
  - Global and class-scope variables
  - Namespaces and their contents
- **Format**:
  ```cpp
  /**
   * @brief Brief description (one line)
   * 
   * Detailed description (multiple lines if needed)
   * @param param1 Description of first parameter
   * @param param2 Description of second parameter
   * @return Description of return value
   * @throw ExceptionType Description of when this exception is thrown
   * @note Any important notes about the function
   * @warning Any warnings about using the function
   * @see Related functions or classes
   */
  ```
- **Examples**: Include `@code` blocks for complex functionality
- **Maintainability**: Update documentation when code changes
- **Verification**: Run Doxygen to verify documentation builds without warnings

## Common Tasks

### Running Tests
```bash
# Run all tests
make test

# Run specific test
./build/tests/test_module

# Generate coverage report
make coverage
```

### Building the Project
```bash
# Debug build
mkdir -p build/debug && cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make -j$(nproc)

# Release build
mkdir -p build/release && cd build/release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make -j$(nproc)
```

### Code Formatting
```bash
# Format all source files
find include tests -name '*.hpp' -o -name '*.cpp' | xargs clang-format -i
```

## Troubleshooting

### Common Issues
- **Missing Dependencies**: Check `third-party/` and README.md
- **Build Failures**: Clean build directory and rebuild
- **Test Failures**: Run with `--verbose` for detailed output
- **Memory Issues**: Use AddressSanitizer/Valgrind

### Performance Optimization
- Profile before optimizing
- Focus on algorithmic complexity first
- Use appropriate data structures
- Consider cache locality

## Security Guidelines
- Validate all inputs
- Use secure defaults
- Handle errors gracefully
- Follow principle of least privilege
- Keep dependencies updated

## Additional Instructions

For detailed guidelines on specific components:

- [Header Files](./instructions/header-files.instructions.md)
- [Makefile](./instructions/makefile.instructions.md)
- [Test Files](./instructions/test-files.instructions.md)

## License
MIT License

Copyright (c) 2026 Interlaced Pixel

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
*This document is maintained by the project maintainers. Please update it when making significant changes to the project structure or development workflow.*

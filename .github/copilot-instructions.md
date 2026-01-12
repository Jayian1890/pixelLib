# pixelLib — Copilot Instructions

This is a header-only C++23 utility library providing essential modules for filesystem operations, JSON parsing, logging, and network operations. The library emphasizes modern C++ practices, cross-platform compatibility, and comprehensive testing.

## Required Before Each Commit
- Run `make run-tests` to ensure all tests pass
- Run `make clang-tidy` to verify code quality and modern C++ compliance
- Ensure test coverage remains above 95% by running `make coverage`
- Format code with clang-format (integrated with VS Code or run manually)

## Development Flow
- **Build**: `make test` or simply `make` (builds test binary with coverage enabled)
- **Debug builds**: `make CXXFLAGS="-O0 -g"` for debugging symbols
- **Test**: `make run-tests` (executes all test suites, ~73 tests)
- **Coverage**: `make coverage` → generates HTML report in `build/coverage/html/index.html`
- **Code quality**: `make clang-tidy` (static analysis), `make clang-tidy-fix` (auto-fix)
- **Full CI check**: `make clean && make test && make run-tests && make coverage && make clang-tidy`

## Repository Structure
```
pixelLib/
├── include/           # Public headers (header-only library)
│   ├── filesystem.hpp # Cross-platform file/directory operations
│   ├── json.hpp       # DOM parser with Unicode support
│   ├── logging.hpp    # Multi-level logging with rotation
│   └── network.hpp    # HTTP/HTTPS, hostname resolution, downloads
├── tests/            # Unit tests (one per module + main)
│   ├── test_filesystem.cc
│   ├── test_json.cc
│   ├── test_logging.cc
│   ├── test_network.cc
│   ├── test_main.cc
│   └── doctest_main.cpp
├── tools/            # Utility scripts
│   ├── get_doctest.sh        # Fetches doctest header
│   ├── run-clang-tidy.sh     # Runs clang-tidy checks
│   └── setup-clang-tidy.sh   # Installs clang-tidy
└── build/           # Build artifacts (gitignored)
    ├── tests/       # Compiled test binary
    └── coverage/    # Coverage reports (HTML & LCOV)
```

## Architecture & Design
- **Header-only design**: All implementations in headers (`include/`), no linking required
- **C++23 target**: Modern C++ features enforced by Makefile (`STD ?= c++23`)
- **Namespace structure**: `pixellib::core::<module>::<class>` (e.g., `pixellib::core::json::JSON`)
- **Testing**: Single test binary using doctest framework, 95%+ code coverage target
- **Cross-platform**: Windows (MinGW/MSVC), macOS, Linux with platform-specific handling

## Code Standards & Conventions

### File Organization
- **Headers**: `.hpp` extension only, placed in `include/` directory (no subdirectories)
- **Tests**: `.cc` extension, placed in `tests/` directory, one test file per module
- **Naming**: Follow existing patterns - `test_<module>.cc` for tests, lowercase with underscores

### C++ Style & Best Practices
- **Modern C++23**: Use std::filesystem, std::ranges, concepts, and other C++23 features
- **clang-tidy enforced**: Runs modernize-*, performance-*, and safety checks automatically
- **clang-format**: Automatic formatting configured in `.clang-format` file
- **Namespace structure**: Always use `pixellib::core::<module>::<class>` pattern
- **Header-only constraint**: All implementations must be in headers (no `.cpp` files)
- **Minimal dependencies**: Only doctest for testing; avoid adding new dependencies
- **Cross-platform code**: Use preprocessor directives for platform-specific code when needed

### Testing Requirements
- **Coverage target**: Maintain 95%+ code coverage for all new code
- **Test isolation**: Use `PIXELLIB_TEST_MODE=1` environment variable for deterministic tests
- **Single file per module**: Keep all tests for a module in one `tests/test_<module>.cc` file
- **Doctest macros**: Use `TEST_CASE`, `CHECK`, `REQUIRE`, `SUBCASE` from doctest framework
- **No external dependencies**: Tests must be self-contained and offline-safe

## Build System & Testing Details
- **Makefile-driven**: Single Makefile handles all build, test, and coverage tasks
- **Compiler detection**: Auto-detects clang/gcc/Windows and adjusts flags accordingly
- **Coverage support**: 
  - LLVM/clang coverage on macOS (llvm-profdata, llvm-cov)
  - GCC/gcovr on Linux
  - HTML reports in `build/coverage/html/` with LCOV files
- **Test mode**: Set `PIXELLIB_TEST_MODE=1` for deterministic offline-safe tests
- **Compilation database**: `make compile-commands` generates `build/compile_commands.json` for clangd

## Code Quality & Tooling
- **clang-tidy**: Comprehensive configuration in `.clang-tidy` with modern C++ enforcement
  - Checks: modernize-*, performance-*, readability-*, bugprone-*, cppcoreguidelines-*
  - Run: `make clang-tidy` (check), `make clang-tidy-fix` (auto-fix), `make clang-tidy-report` (detailed report)
- **clang-format**: Code formatting via `.clang-format` file (auto-applied in VS Code)
- **VS Code integration**: Optimized settings in `.vscode/` for clangd and clang-tidy
- **CI/CD**: GitHub Actions in `.github/workflows/build-test.yml` runs all checks on push/PR

## Development Workflow Examples

### Adding a New Feature
1. Identify the module to modify (filesystem, json, logging, or network)
2. Edit the header file in `include/<module>.hpp`
3. Add comprehensive tests in `tests/test_<module>.cc`
4. Run `make run-tests` to verify tests pass
5. Run `make coverage` to ensure >95% coverage
6. Run `make clang-tidy` to verify code quality
7. Commit changes with descriptive message

### Fixing a Bug
1. Add a failing test case that reproduces the bug in `tests/test_<module>.cc`
2. Run `make run-tests` to confirm the test fails
3. Fix the bug in `include/<module>.hpp`
4. Run `make run-tests` to confirm the test passes
5. Run `make clang-tidy` to check for issues
6. Commit with reference to the bug

### Debugging Build Issues
- Use debug builds: `make CXXFLAGS="-O0 -g"` for debug symbols
- Check compiler errors carefully - header-only means all errors at compile time
- For coverage issues on macOS, ensure xcrun tools are available
- For Windows, ensure MinGW or MSVC is properly configured

## Module-Specific Guidelines

### Filesystem Module (`filesystem.hpp`)
- Cross-platform operations with Windows/Unix path handling
- Uses `std::filesystem` internally with fallback mechanisms
- Test with both absolute and relative paths
- Handle edge cases: permissions, non-existent paths, special characters

### JSON Module (`json.hpp`)
- DOM parser with strict validation and Unicode support (including surrogate pairs)
- Preserves original numeric representation for round-trip accuracy
- Configurable serialization: compact vs. pretty-print, solidus escaping
- Test with malformed JSON, edge cases (empty objects/arrays), and Unicode

### Logging Module (`logging.hpp`)
- Multi-level logging: TRACE, DEBUG, INFO, WARNING, ERROR, FATAL
- File rotation: size-based and time-based (pre-emptive rotation before exceeding limits)
- Thread-safe operations, structured logging with key/value pairs
- Compile-time filtering via `PIXELLIB_COMPILED_LOG_LEVEL` macro
- Test with concurrent logging, rotation triggers, and custom formatters

### Network Module (`network.hpp`)
- HTTP/HTTPS operations, hostname resolution, downloads with progress
- Use `PIXELLIB_TEST_MODE=1` for deterministic offline testing
- Handle timeouts, redirects, and connection failures gracefully
- Test with both IPv4 and IPv6 addresses

## Testing Guidelines
- **High coverage target**: Aim for 95%+ code coverage on all new code
- **Deterministic tests**: Use `PIXELLIB_TEST_MODE=1` for offline-safe network operations
- **Test organization**: All tests for a module in one `tests/test_<module>.cc` file
- **Doctest usage**: Use `TEST_CASE`, `CHECK`, `REQUIRE`, `SUBCASE` macros appropriately
- **Test isolation**: Each test should be independent with no external service dependencies
- **Edge cases**: Test boundary conditions, error paths, and platform-specific behavior

## Common Tasks & Examples

### Adding a New Function to a Module
```cpp
// In include/<module>.hpp
namespace pixellib::core::<module> {
  class ModuleName {
  public:
    // Add new function with clear documentation
    static Result new_function(const std::string& param) {
      // Implementation here
    }
  };
}

// In tests/test_<module>.cc
TEST_CASE("ModuleName::new_function - basic usage") {
  auto result = ModuleName::new_function("test");
  CHECK(result.success);
  CHECK(result.value == "expected");
}

TEST_CASE("ModuleName::new_function - error handling") {
  auto result = ModuleName::new_function("");
  CHECK(!result.success);
}
```

### Updating Documentation
- Update README.md for user-facing changes
- Update inline code comments for complex logic
- Keep this copilot-instructions.md updated for development workflow changes
- Update docstring-style comments for public APIs

## Key Constraints & Requirements
- **Header-only**: No compiled library artifacts, all code in headers
- **C++23 compliance**: Use modern C++ features, enforced by clang-tidy
- **Minimal dependencies**: Only add dependencies if absolutely necessary
- **Consistent style**: Follow existing naming conventions and clang-format rules
- **Test coverage**: Add comprehensive tests for any new functionality
- **Performance**: Consider performance implications; clang-tidy flags inefficiencies
- **Cross-platform**: Ensure Windows/macOS/Linux compatibility with platform-specific code when needed

## Integration & Tooling
- **VS Code**: Optimized configuration with clangd, clang-tidy, and clang-format
- **clangd**: Language server for code navigation, auto-completion, and diagnostics
- **Coverage visualization**: Use "Coverage Gutters" extension with `build/coverage/lcov.relative.info`
- **CI/CD**: GitHub Actions automatically runs build, test, coverage, and clang-tidy on PR

## Guidelines for AI Agents
- **Root cause analysis**: Use clang-tidy to identify fundamental issues, not just symptoms
- **Modern C++ first**: Leverage C++23 features and follow clang-tidy recommendations
- **Header-only constraint**: All implementations must be in headers, never create `.cpp` files
- **Test-driven**: Always add/modify tests when implementing changes
- **Performance aware**: Watch for clang-tidy performance warnings
- **Cross-platform**: Consider Windows/macOS/Linux differences in implementations
- **Minimal changes**: Make surgical, targeted changes rather than broad refactoring
- **Validate thoroughly**: Run tests, coverage, and clang-tidy before considering work complete
- **Commit messages**: Keep commit messages short and concise - maximum 50 characters for the first line, no long explanations
- **No mock code**: NEVER use mock code for any reason, always use real code
- **Convert mock to real**: ALWAYS convert mock code to real code

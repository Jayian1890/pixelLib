---
applyTo: "**/Makefile"
---

## Makefile Modification Guidelines

The Makefile is the central build system for pixelLib. When modifying it, follow these guidelines:

### Build System Principles
1. **Maintain simplicity** - Keep the Makefile straightforward and easy to understand
2. **Cross-platform support** - Ensure changes work on macOS, Linux, and Windows (MinGW)
3. **Compiler flexibility** - Support both clang++ and g++ with auto-detection
4. **Default to modern C++** - Keep `STD ?= c++23` as the standard

### Key Targets (Do Not Break)
- `all` - Main target: clean, build tests, run coverage
- `test` - Build the test binary with coverage flags
- `run-tests` - Execute the test binary
- `coverage` - Generate coverage reports (HTML and LCOV)
- `clean` - Remove build artifacts
- `clang-tidy` - Run static analysis
- `clang-tidy-fix` - Auto-fix issues
- `compile-commands` - Generate compilation database for clangd

### Coverage System
```makefile
# LLVM/Clang coverage (macOS, Linux with clang)
ifeq ($(IS_CLANG),1)
  COV_CFLAGS := -fprofile-instr-generate -fcoverage-mapping
  # Use xcrun on macOS, direct tools elsewhere
endif

# GCC coverage (Linux with gcc)
ifeq ($(IS_GCC),1)
  COV_CFLAGS := --coverage
endif
```

### Platform Detection
The Makefile auto-detects:
- **Compiler type**: `IS_CLANG` or `IS_GCC` based on `$(CXX) --version`
- **Operating system**: `IS_WINDOWS` based on `uname -s` or `OS` environment variable
- **Tool paths**: On macOS, uses `xcrun` to find LLVM tools; elsewhere uses direct paths

### Common Modifications

#### Adding a New Compiler Flag
```makefile
# Add to appropriate section
WARN := -Wall -Wextra -Wpedantic -Wnew-flag
```

#### Adding a Platform-Specific Library
```makefile
ifeq ($(IS_WINDOWS),1)
  LIBS += -lnewlib  # Windows only
endif
```

#### Adding a New Target
```makefile
.PHONY: new-target
new-target: dependencies
	@echo "Running new target..."
	@command-to-execute
```

### Testing Makefile Changes
1. Test on clang: `make clean && CXX=clang++ make test`
2. Test on gcc: `make clean && CXX=g++ make test`
3. Test coverage: `make clean && make coverage`
4. Test all targets: `make all`
5. Verify clean: `make clean` should remove all generated files

### Important Variables
- `CXX` - C++ compiler (default: clang++)
- `STD` - C++ standard (default: c++23)
- `WARN` - Warning flags
- `DBG` - Debug flags
- `INCLUDES` - Include directories
- `LIBS` - Libraries to link
- `COV_CFLAGS` - Coverage compilation flags
- `TEST_BIN` - Path to test binary

### Validation Checklist
- [ ] Changes work with both clang++ and g++
- [ ] Coverage generation still works (LLVM and GCC)
- [ ] All existing targets still function
- [ ] Platform detection logic is correct
- [ ] No hardcoded paths (use variables)
- [ ] Clean target removes all new artifacts
- [ ] Changes documented in comments

### Avoid
- Breaking existing targets that CI/CD depends on
- Hardcoding absolute paths (use relative paths and variables)
- Removing cross-platform support
- Making compiler-specific changes without detection logic
- Adding complex logic that reduces maintainability

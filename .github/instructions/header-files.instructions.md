---
applyTo: "**/include/*.hpp"
---

## Header File Requirements

When modifying or creating header files in pixelLib, follow these strict guidelines:

### Header-Only Library Constraints
1. **All code in headers** - This is a header-only library; never create `.cpp` implementation files
2. **Complete implementations** - All function implementations must be in the header files
3. **Include guards** - Use `#pragma once` at the top of every header file
4. **Self-contained** - Each header should be independently includable without requiring specific include order

### Code Organization
1. **Namespace structure** - Always use `namespace pixellib::core::<module>` pattern
2. **Module isolation** - Keep each module's code in its respective header file
3. **Minimal includes** - Only include headers that are strictly necessary
4. **Forward declarations** - Use forward declarations when possible to reduce dependencies

### C++23 Best Practices
1. **Modern features** - Use C++23 features: concepts, ranges, std::format (when available), etc.
2. **Type safety** - Prefer strong typing over raw types (e.g., use enums over integers)
3. **Const correctness** - Mark functions and variables `const` where appropriate
4. **noexcept** - Use `noexcept` for functions that don't throw exceptions

### Platform Compatibility
```cpp
// Use preprocessor directives for platform-specific code
#ifdef _WIN32
    // Windows-specific implementation
#elif defined(__APPLE__) || defined(__linux__)
    // Unix-like implementation
#endif
```

### Code Style
1. **clang-format** - Code will be automatically formatted; follow existing style
2. **clang-tidy** - Run `make clang-tidy` to catch issues before committing
3. **Documentation** - Add clear comments for public APIs and complex algorithms
4. **Error handling** - Use Result/Optional types or exceptions with clear error messages

### Common Patterns
```cpp
// Result pattern for operations that can fail
struct Result {
    bool success;
    std::string message;
    // ... additional fields
};

// Static class methods for utility functions
class ModuleName {
public:
    static Result operation(const std::string& input) {
        // Validate input
        if (input.empty()) {
            return {false, "Input cannot be empty"};
        }
        
        // Perform operation
        // ...
        
        return {true, "Success"};
    }
};
```

### Validation Checklist
- [ ] Header uses `#pragma once`
- [ ] Code is in `namespace pixellib::core::<module>`
- [ ] All functions have implementations (no forward declarations without definitions)
- [ ] Platform-specific code is properly guarded with `#ifdef`
- [ ] Public APIs have documentation comments
- [ ] Code compiles with C++23 standard (`-std=c++23`)
- [ ] `make clang-tidy` passes without errors
- [ ] Changes maintain header-only architecture

### Avoid
- Creating `.cpp` files (this is header-only)
- Using deprecated C++ features (clang-tidy will flag these)
- Platform-specific code without preprocessor guards
- Heavy templates that significantly increase compile time
- Exposing internal implementation details in public APIs

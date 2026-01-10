# Clang-Tidy Integration

This project includes a comprehensive clang-tidy configuration optimized for modern C++23 development.

## Quick Start

```bash
# Run all clang-tidy checks
make clang-tidy

# Run checks and apply automatic fixes
make clang-tidy-fix

# Generate a markdown report
make clang-tidy-report

# Run with custom options
./tools/run-clang-tidy.sh --jobs 8 --checks "bugprone-*,performance-*"
```

## Configuration Features

The `.clang-tidy` configuration is optimized for:

- **Modern C++23 Practices**: Encourages use of modern language features
- **Performance**: Identifies performance bottlenecks and inefficiencies
- **Safety**: Catches potential bugs and security issues
- **Code Quality**: Improves readability and maintainability
- **Portability**: Ensures cross-platform compatibility

## Key Check Categories

### Modern C++ (`modernize-*`)
- Use `auto` where appropriate
- Prefer range-based for loops
- Use `nullptr` instead of `NULL`
- Modernize string literals and using declarations

### Performance (`performance-*`)
- Detect unnecessary copies
- Identify inefficient algorithms
- Move semantics optimization
- Exception safety performance

### Bug Prevention (`bugprone-*`)
- Memory safety issues
- Incorrect arithmetic operations
- Resource leaks
- Exception handling problems

### Security (`cert-*`)
- CERT security guidelines
- Input validation
- Buffer overflow prevention
- Secure coding practices

### Code Style (`readability-*`, `google-*`)
- Naming conventions
- Code organization
- Documentation standards
- Consistent formatting

## Usage Examples

### Basic Usage
```bash
# Check all files
./tools/run-clang-tidy.sh

# Apply fixes automatically
./tools/run-clang-tidy.sh --fix

# Verbose output
./tools/run-clang-tidy.sh --verbose
```

### Advanced Usage
```bash
# Run specific checks
./tools/run-clang-tidy.sh --checks "bugprone-*,performance-*"

# Limit to specific files
./tools/run-clang-tidy.sh --checks "modernize-*" include/network.hpp

# Generate fixes file for review
./tools/run-clang-tidy.sh --export-fixes fixes.yaml

# Parallel execution
./tools/run-clang-tidy.sh --jobs 12
```

### Integration with Build
```bash
# Run as part of your build process
make clean && make clang-tidy && make test

# Generate reports
make clang-tidy-report
```

## Configuration Details

### Enabled Checks
The configuration enables comprehensive checks while excluding noisy ones:

**Always Enabled:**
- All `bugprone-*` checks (critical safety)
- All `cert-*` checks (security)
- Most `performance-*` checks (optimization)
- Selected `modernize-*` checks (C++23 best practices)
- Selected `readability-*` checks (code quality)

**Explicitly Disabled:**
- `readability-magic-numbers` (too noisy for constants)
- `modernize-use-trailing-return-type` (style preference)
- `google-readability-todo` (TODO comments are useful)
- Various overly pedantic checks

### Warning Levels
- **Errors**: Critical safety and security issues
- **Warnings**: Performance and style issues
- **Notes**: Suggestions for improvement

### Naming Conventions
- Classes: `CamelCase`
- Functions: `camelBack`
- Variables: `snake_case`
- Constants: `UPPER_CASE` with `k` prefix
- Member variables: `snake_case` with `_` prefix

## IDE Integration

### VS Code
Add to `.vscode/settings.json`:
```json
{
    "clang-tidy.checks": "*",
    "clang-tidy.compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "clang-tidy.run": "onSave"
}
```

### CLion
1. Open Settings → Editor → Inspections → C/C++ → Clang-Tidy
2. Enable "Clang-Tidy" and configure checks
3. Set configuration file to project root

### Vim/Neovim
```vim
" Install clang-tidy plugin
Plug 'rhysd/vim-clang-format'

" Run on save
autocmd BufWritePost *.cpp,*.hpp !clang-tidy % --fix
```

## Continuous Integration

### GitHub Actions
```yaml
- name: Run clang-tidy
  run: |
    make clang-tidy-report
    # Upload report as artifact
```

### Pre-commit Hook
```bash
#!/bin/sh
# .git/hooks/pre-commit
./tools/run-clang-tidy.sh --fix
git add -A
```

## Troubleshooting

### Common Issues

1. **"clang-tidy not found"**
   ```bash
   # macOS
   brew install llvm
   
   # Ubuntu/Debian
   sudo apt install clang-tidy
   
   # Add to PATH
   export PATH="/usr/local/opt/llvm/bin:$PATH"
   ```

2. **"No compilation database found"**
   ```bash
   # Generate compile_commands.json
   make clean
   bear -- make
   ```

3. **Too many warnings**
   ```bash
   # Run specific check categories
   ./tools/run-clang-tidy.sh --checks "bugprone-*,cert-*"
   
   # Or disable specific checks in .clang-tidy
   ```

4. **Performance issues**
   ```bash
   # Limit parallel jobs
   ./tools/run-clang-tidy.sh --jobs 4
   
   # Check specific files only
   ./tools/run-clang-tidy.sh include/network.hpp
   ```

### Customization

To customize the configuration:

1. Edit `.clang-tidy` directly
2. Use command-line options to override
3. Create project-specific configurations
4. Exclude specific files or directories

## Best Practices

1. **Run regularly**: Integrate into your development workflow
2. **Fix incrementally**: Don't try to fix everything at once
3. **Review fixes**: Automatic fixes may need manual review
4. **Team consistency**: Ensure all team members use the same configuration
5. **CI integration**: Run in CI to maintain code quality

## Performance Tips

- Use parallel execution (`--jobs` flag)
- Limit checks to specific categories
- Exclude third-party code
- Use incremental analysis
- Cache compilation database

## Resources

- [Clang-Tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [Clang-Tidy Checks Reference](https://clang.llvm.org/extra/clang-tidy/checks/)
- [Modern C++ Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/display/cplusplus/2+Rules)

---
applyTo: "**/tests/test_*.cc"
---

## Unit Test Requirements

When writing or modifying unit tests for pixelLib, follow these guidelines:

### Test Organization
1. **One test file per module** - Keep all tests for a module in a single `tests/test_<module>.cc` file
2. **Logical grouping** - Use `TEST_CASE` for major functionality, `SUBCASE` for variations
3. **Descriptive names** - Use clear, descriptive test names: `TEST_CASE("ModuleName::function - edge case description")`

### Test Writing Standards
1. **Use doctest macros** - Prefer `CHECK` over `REQUIRE` unless test cannot continue on failure
2. **Test independence** - Each test should be self-contained and not rely on other tests' state
3. **Coverage focus** - Aim for 95%+ code coverage, including error paths and edge cases
4. **Deterministic tests** - Use `PIXELLIB_TEST_MODE=1` for network tests to avoid external dependencies

### Test Structure
```cpp
TEST_CASE("ModuleName::function - descriptive name") {
    // Setup
    auto input = create_test_input();
    
    // Execute
    auto result = ModuleName::function(input);
    
    // Verify
    CHECK(result.success);
    CHECK(result.value == expected_value);
    
    SUBCASE("edge case 1") {
        // Test specific edge case
    }
    
    SUBCASE("error handling") {
        // Test error conditions
    }
}
```

### Common Patterns
1. **Test both success and failure paths** - Every function should have tests for valid and invalid inputs
2. **Boundary testing** - Test edge cases: empty strings, null values, maximum values, etc.
3. **Platform-specific tests** - Use `#ifdef` when testing platform-specific behavior
4. **Resource cleanup** - Ensure tests clean up any files, connections, or resources they create

### Test Verification
- Run `make run-tests` to execute all tests (should complete in seconds)
- Run `make coverage` to verify coverage remains above 95%
- Check that tests pass with `PIXELLIB_TEST_MODE=1` for deterministic network behavior
- Ensure tests are platform-agnostic or properly guarded with preprocessor directives

### Avoid
- External dependencies (network calls, file system dependencies outside test directory)
- Hardcoded paths (use relative paths or construct paths programmatically)
- Long-running tests (each test should complete in milliseconds)
- Flaky tests (tests that sometimes pass and sometimes fail)

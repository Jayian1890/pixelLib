# pixelLib

A comprehensive C++ core library providing essential utilities for filesystem operations, JSON parsing, logging, and network operations.

## Features

### Filesystem
- Cross-platform file and directory operations
- File existence checking, copying, moving, and deletion
- Directory creation and listing
- Path manipulation utilities

### JSON
- Header-only DOM parser and serializer (objects, arrays, numbers, booleans, strings, null)
- Unicode escape handling (\uXXXX with surrogate pairs) and correct string escaping
- Strict validation with detailed error reporting (`parse_or_throw`) plus a boolean `validate` helper
- Compact and pretty-print serialization with configurable indentation and solidus escaping
- Deterministic numeric formatting (stores original representation for round-trips)

### Logging
- Configurable logging with multiple levels (TRACE, DEBUG, INFO, WARNING, ERROR, FATAL)
- Customizable output formatting
- File rotation (size-based and time-based). Size-based rotation now considers the next write and will rotate before a write that would exceed the limit.
- Thread-safe logging operations
- Structured logging support (key/value pairs)
- Fluent configuration using `Logger::LoggerConfig` and `Logger::LoggerConfigBuilder`
- Named category loggers via `Logger::get("name")` and `Logger::LoggerRegistry`
- Compile-time log-level filtering via setting preprocessor macro `PIXELLIB_COMPILED_LOG_LEVEL` to one of:
  - `PIXELLIB_LOG_LEVEL_TRACE` (0) — enable all logs
  - `PIXELLIB_LOG_LEVEL_DEBUG` (1)
  - `PIXELLIB_LOG_LEVEL_INFO` (2)
  - `PIXELLIB_LOG_LEVEL_WARNING` (3)
  - `PIXELLIB_LOG_LEVEL_ERROR` (4)
  - `PIXELLIB_LOG_LEVEL_FATAL` (5)

  Define `PIXELLIB_COMPILED_LOG_LEVEL` before including `logging.hpp` to remove lower-level logging APIs at compile time (e.g., `#define PIXELLIB_COMPILED_LOG_LEVEL PIXELLIB_LOG_LEVEL_INFO`).

- Disable file logging by calling `Logger::set_file_logging(nullptr)` to safely remove any active file logger
- Simple "{}" placeholder formatting is supported (e.g. `Logger::error("Failed: {}", error_code)`) and structured logging is available as `Logger::info("User login", "user_id", 12345, "ip", "1.2.3.4")`

### Network
- Hostname resolution to IP addresses
- Host reachability testing
- HTTP/HTTPS GET and POST requests
- File downloading with progress tracking
- URL encoding/decoding utilities
- Network interface information retrieval
- IP address validation (IPv4 and IPv6)
- Bandwidth and latency measurement tools

## Building

This project uses a Makefile for building. Run the following commands:

```bash
# Build the test binary
make test

# Run the tests
make run-tests

# Generate coverage report
make coverage
```

### Testing

Unit tests are included and can be run using the Makefile. The tests use doctest for assertions and aim for at least 95% code coverage.

```bash
# Run tests
make run-tests

# Generate coverage report (HTML and LCOV)
make coverage
```

Coverage reports are generated in the `build/coverage` directory.

Test mode for per-test isolation: set the environment variable `PIXELLIB_TEST_MODE=1` to make network operations deterministic and offline-safe (e.g., DNS resolves to 127.0.0.1, reachability returns success, and downloads write a small placeholder file).

**Note:** Test source files under `tests/` are excluded from the coverage report to avoid counting test code in coverage numbers.

Note for VS Code: some coverage viewers (e.g., the "Coverage Gutters" extension) prefer workspace-relative paths in LCOV files. The Makefile now emits a workspace-relative LCOV at `build/coverage/lcov.relative.info` in addition to `build/coverage/lcov.info`. If your coverage extension cannot map lines to source files, run:

```bash
make coverage    # generates lcov.info
make lcov-rel    # creates build/coverage/lcov.relative.info with workspace-relative paths
```

In VS Code use the Coverage Gutters command palette ("Coverage Gutters: Display Coverage") and select `build/coverage/lcov.relative.info` if needed.

## Usage

Include the header files in your project:

```cpp
#include "pixellib/filesystem.hpp"
#include "pixellib/json.hpp"
#include "pixellib/logging.hpp"
#include "pixellib/network.hpp"
```

### Example Usage

#### Note: The project namespace and include path are now `pixellib` (e.g., `#include "pixellib/filesystem.hpp"`).

#### Filesystem Operations
```cpp
#include "pixellib/filesystem.hpp"

bool exists = pixellib::core::filesystem::FileSystem::exists("file.txt");
pixellib::core::filesystem::FileSystem::create_directories("path/to/dir");
```

#### JSON Operations
```cpp
#include "json.hpp"
using pixellib::core::json::JSON;
using pixellib::core::json::StringifyOptions;

JSON doc = JSON::parse_or_throw(R"({"name":"Ada","scores":[1,2,3],"active":true})");

const JSON *name = doc.find("name");
if (name && name->is_string()) {
    std::cout << "Name: " << name->as_string() << "\n";
}

JSON &user = doc["user"];
user = JSON::object({{"id", JSON::number("42")}});
std::string compact = doc.stringify();

StringifyOptions pretty{true, 2, false};
std::string pretty_text = doc.stringify(pretty);
```

#### Logging
```cpp
#include "logging.hpp"

using namespace pixellib::core::logging;

Logger::set_level(LOG_DEBUG);
Logger::info("Application started");
Logger::error("An error occurred: {}", error_code);
```

#### Network Operations
```cpp
#include "network.hpp"

// Resolve hostname to IP
auto resolve_result = pixellib::core::network::Network::resolve_hostname("google.com");
if (resolve_result.success) {
    std::cout << "IP Address: " << resolve_result.message << std::endl;
}

// Check if host is reachable
auto reach_result = pixellib::core::network::Network::is_host_reachable("google.com");
if (reach_result.success) {
    std::cout << "Host is reachable" << std::endl;
}

// Download a file
auto download_result = pixellib::core::network::Network::download_file(
    "http://example.com/file.txt", "local_file.txt");
```

## Requirements

- C++17 or later
- Clang++ (recommended) or GCC
- Make
- LLVM tools (for coverage on macOS: xcrun llvm-profdata, xcrun llvm-cov)
- For Windows: MinGW-w64 or Visual Studio with Windows SDK
- For Unix-like systems: GCC/Clang with standard development tools

## Cross-Platform Support

The library is designed to work on:
- Windows (MinGW, Visual Studio)
- macOS
- Linux
- Other Unix-like systems

Platform-specific implementations are included where necessary to ensure consistent behavior across all supported platforms.

## Testing

Unit tests are included and can be run using the Makefile. The tests use doctest for assertions and aim for at least 95% code coverage.

```bash
# Run tests
make run-tests

# Generate coverage report (HTML and LCOV)
make coverage
```

Coverage reports are generated in the `build/coverage` directory.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a pull request

## License

MIT License

Copyright (c) 2025 Your Name or Organization

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
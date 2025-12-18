# Interlaced Core Library

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
- Configurable logging with multiple levels (DEBUG, INFO, WARNING, ERROR)
- Customizable output formatting
- File rotation (size-based and time-based). Size-based rotation now considers the next write and will rotate before a write that would exceed the limit.
- Thread-safe logging operations
- Structured logging support (key/value pairs)
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

This project uses CMake for building. Recommended commands:

```bash
# Create an out-of-source build directory and configure
mkdir -p build
cmake -S . -B build

# Build (optionally specify a config on multi-config generators)
cmake --build build --config Debug
```

### Testing

This repository does not include unit tests in this branch. Tests were removed; re-add them as needed.

## Usage

Include the header files in your project:

```cpp
#include "interlaced_core/filesystem.hpp"
#include "interlaced_core/json.hpp"
#include "interlaced_core/logging.hpp"
#include "interlaced_core/network.hpp"
```

### Example Usage

#### Filesystem Operations
```cpp
#include "interlaced_core/filesystem.hpp"

bool exists = interlaced::core::filesystem::FileSystem::exists("file.txt");
interlaced::core::filesystem::FileSystem::create_directories("path/to/dir");
```

#### JSON Operations
```cpp
#include "interlaced_core/json.hpp"
using interlaced::core::json::JSON;
using interlaced::core::json::StringifyOptions;

// Parse (throws on error with position info)
JSON doc = JSON::parse_or_throw(R"({"name":"Ada","scores":[1,2,3],"active":true})");

// Access object members
const JSON *name = doc.find("name");
if (name && name->is_string()) {
    std::cout << "Name: " << name->as_string() << "\n";
}

// Mutate and serialize (compact)
JSON &user = doc["user"];
user = JSON::object({{"id", JSON::number("42")}});
std::string compact = doc.stringify();

// Pretty-print
StringifyOptions pretty{true /*pretty*/, 2 /*indent*/, false /*escape_solidus*/};
std::string pretty_text = doc.stringify(pretty);
```

#### Logging
```cpp
#include "interlaced_core/logging.hpp"

using namespace interlaced::core::logging;

Logger::set_level(LOG_DEBUG);
Logger::info("Application started");
Logger::error("An error occurred: {}", error_code);
```

#### Network Operations
```cpp
#include "interlaced_core/network.hpp"

// Resolve hostname to IP
auto resolve_result = interlaced::core::network::Network::resolve_hostname("google.com");
if (resolve_result.success) {
    std::cout << "IP Address: " << resolve_result.message << std::endl;
}

// Check if host is reachable
auto reach_result = interlaced::core::network::Network::is_host_reachable("google.com");
if (reach_result.success) {
    std::cout << "Host is reachable" << std::endl;
}

// Download a file
auto download_result = interlaced::core::network::Network::download_file(
    "http://example.com/file.txt", "local_file.txt");
```

## Requirements

- C++17 or later
- CMake 3.10 or later
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

This repository does not include unit tests in this branch.

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
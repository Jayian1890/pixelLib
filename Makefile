# Makefile for pixelLib project
# Cross-platform build system with compiler detection

# Variables
	CXX ?= clang++
STD ?= c++23
WARN := -Wall -Wextra -Wpedantic
DBG := -g
INCLUDES := -Iinclude -Ithird-party
LIBS :=
CLANG_TIDY ?= /opt/homebrew/Cellar/llvm/21.1.8/bin/clang-tidy

# Platform detection
UNAME := $(shell uname -s)
IS_MACOS := $(if $(findstring Darwin, $(UNAME)),1,0)
IS_WINDOWS := $(if $(findstring MINGW, $(UNAME)),1,0)

# Compiler detection
CXX_VERSION := $(shell $(CXX) --version 2>/dev/null)
IS_CLANG := $(if $(findstring clang, $(CXX_VERSION)),1,0)
IS_GCC := $(if $(findstring g++, $(CXX_VERSION)),1,0)

# Coverage flags
COV_CFLAGS :=
ifeq ($(IS_CLANG),1)
  COV_CFLAGS := -fprofile-instr-generate -fcoverage-mapping
  ifeq ($(IS_MACOS),1)
    LLVM_PROFDATA := xcrun llvm-profdata
    LLVM_COV := xcrun llvm-cov
  else
    LLVM_PROFDATA := llvm-profdata
    LLVM_COV := llvm-cov
  endif
endif
ifeq ($(IS_GCC),1)
  COV_CFLAGS := --coverage
endif

# Targets
.PHONY: all build run build_test run_test coverage clean compile-commands clang-tidy clang-tidy-fix

all: clean build_test compile-commands coverage

build_test:
	mkdir -p build
	$(CXX) -std=$(STD) $(WARN) $(DBG) $(COV_CFLAGS) $(INCLUDES) -o build/unit_tests tests/doctest_main.cpp tests/test_filesystem.cc tests/test_json.cc tests/test_logging.cc tests/test_network.cc $(LIBS)

run_test: build_test
	./build/unit_tests

coverage: run_test
	mkdir -p build/coverage
ifeq ($(IS_CLANG),1)
	$(LLVM_PROFDATA) merge -sparse default.profraw -o default.profdata
	$(LLVM_COV) show ./build/unit_tests -instr-profile=default.profdata > build/coverage/coverage.txt
	$(LLVM_COV) report ./build/unit_tests -instr-profile=default.profdata
	@command -v gcovr >/dev/null 2>&1 \
		&& gcovr -r . --exclude 'third-party/*' --exclude 'tests/*' --lcov -o build/coverage/coverage.lcov \
		&& echo "LCOV report generated at build/coverage/coverage.lcov" \
		|| echo "gcovr not found; to generate LCOV install it (e.g. 'pip install gcovr' or 'apt-get install gcovr')."
else ifeq ($(IS_GCC),1)
	@command -v gcovr >/dev/null 2>&1 \
		&& gcovr -r . --html --html-details -o build/coverage/index.html . \
		|| echo "gcovr not found; to generate HTML coverage install it (e.g. 'pip install gcovr' or 'apt-get install gcovr')."
	@command -v lcov >/dev/null 2>&1 \
		&& lcov --capture --directory . --output-file build/coverage/coverage.lcov --exclude "*/third-party/*" --exclude "*/tests/*" 2>/dev/null \
		&& echo "LCOV report generated at build/coverage/coverage.lcov" \
		|| echo "Note: lcov not installed, install with: brew install lcov (macOS) or apt-get install lcov (Linux)"
endif

clean:
	rm -rf build/
	rm -f *.profraw *.profdata
	rm -f coverage.lcov
	find . -name "*.gcov" -delete 2>/dev/null || true
	find . -name "*.gcda" -delete 2>/dev/null || true
	find . -name "*.gcno" -delete 2>/dev/null || true

compile-commands:
	mkdir -p build
	@echo '[' > build/compile_commands.json
	@echo '  {"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) -c tests/doctest_main.cpp","file":"tests/doctest_main.cpp"},' >> build/compile_commands.json
	@echo '  {"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) -c tests/test_filesystem.cc","file":"tests/test_filesystem.cc"},' >> build/compile_commands.json
	@echo '  {"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) -c tests/test_json.cc","file":"tests/test_json.cc"},' >> build/compile_commands.json
	@echo '  {"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) -c tests/test_logging.cc","file":"tests/test_logging.cc"},' >> build/compile_commands.json
	@echo '  {"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) -c tests/test_network.cc","file":"tests/test_network.cc"}' >> build/compile_commands.json
	@echo ']' >> build/compile_commands.json

clang-tidy: compile-commands
	$(CLANG_TIDY) -p build $(shell find . -name "*.cpp" -o -name "*.cc" -o -name "*.hpp" -o -name "*.h" | grep -v "./build/" | grep -v "./third-party/")

clang-tidy-fix: compile-commands
	$(CLANG_TIDY) -p build --fix-errors $(shell find . -name "*.cpp" -o -name "*.cc" -o -name "*.hpp" -o -name "*.h" | grep -v "./build/" | grep -v "./third-party/")

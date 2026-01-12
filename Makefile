# Makefile for pixelLib project
# Cross-platform build system with compiler detection

# Variables
CXX ?= clang++
STD ?= c++23
WARN := -Wall -Wextra -Wpedantic
DBG := -g
INCLUDES := -Iinclude -Ithird-party
LIBS :=

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
.PHONY: all build run build_test run_test coverage clean compile-commands

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
	else ifeq ($(IS_GCC),1)
		gcovr -r . --html --html-details -o build/coverage/index.html .
	endif

clean:
	rm -rf build/

compile-commands: build_test
	mkdir -p build
	echo '[{"directory":"'$(shell pwd)'","command":"$(CXX) -std=$(STD) $(WARN) $(DBG) $(COV_CFLAGS) $(INCLUDES) -o build/unit_tests tests/doctest_main.cpp tests/test_filesystem.cc tests/test_json.cc tests/test_logging.cc tests/test_network.cc $(LIBS)","file":"tests/doctest_main.cpp"}]' > build/compile_commands.json

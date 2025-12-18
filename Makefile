CXX ?= clang++
STD ?= c++17
WARN := -Wall -Wextra -Wpedantic
DBG := -O0 -g
INCLUDES := -Iinclude -Ithird_party/doctest

# Detect compiler type
CXX_VERSION := $(shell $(CXX) --version 2>/dev/null)
IS_CLANG := $(if $(findstring clang,$(CXX_VERSION)),1,0)
IS_GCC := $(if $(findstring g++,$(CXX_VERSION)),1,0)
IS_GCC := $(if $(findstring gcc,$(CXX_VERSION)),1,$(IS_GCC))

# Detect OS
UNAME_S := $(shell uname -s 2>/dev/null)

# Set coverage flags based on compiler
ifeq ($(IS_CLANG),1)
  # LLVM/Clang coverage
  COV_CFLAGS := -fprofile-instr-generate -fcoverage-mapping
  COV_ENV := LLVM_PROFILE_FILE
  # Use appropriate llvm tools based on OS
  ifeq ($(UNAME_S),Darwin)
    # macOS: use xcrun to find tools
    LLVM_PROFDATA := xcrun llvm-profdata
    LLVM_COV := xcrun llvm-cov
  else
    # Linux/other: use tools directly
    LLVM_PROFDATA := llvm-profdata
    LLVM_COV := llvm-cov
  endif
else ifeq ($(IS_GCC),1)
  # GCC coverage
  COV_CFLAGS := --coverage
  COV_ENV := 
else
  # No coverage for unknown compiler
  COV_CFLAGS := 
  COV_ENV := 
endif

TEST_DIR := tests
BIN_DIR := build/tests
COVERAGE_DIR := coverage

SOURCES := $(wildcard $(TEST_DIR)/*.cc)
TEST_BIN := $(BIN_DIR)/interlaced_core_tests

.PHONY: all test run-tests coverage clean doctest

all: test

$(TEST_BIN): $(SOURCES) third_party/doctest/doctest.h | $(BIN_DIR)
	$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) $(COV_CFLAGS) $(SOURCES) -o $(TEST_BIN)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

third_party/doctest/doctest.h:
	@mkdir -p third_party/doctest
	@./tools/get_doctest.sh

test: $(TEST_BIN)
	@echo "Built test binary: $(TEST_BIN)"

run-tests: test
ifeq ($(IS_CLANG),1)
	@$(COV_ENV)="$(TEST_BIN).profraw" "$(TEST_BIN)"
else
	@"$(TEST_BIN)"
endif

coverage: run-tests
ifeq ($(IS_CLANG),1)
	@mkdir -p $(COVERAGE_DIR)/html
	@echo "Merging raw profiles -> $(COVERAGE_DIR)/coverage.profdata";
	@$(LLVM_PROFDATA) merge -sparse $(BIN_DIR)/*.profraw -o $(COVERAGE_DIR)/coverage.profdata
	@echo "Exporting lcov -> $(COVERAGE_DIR)/lcov.info";
	@$(LLVM_COV) export --format=lcov --ignore-filename-regex="third_party/.*" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata > $(COVERAGE_DIR)/lcov.info
	@echo "Generating HTML -> $(COVERAGE_DIR)/html";
	@$(LLVM_COV) show --ignore-filename-regex="third_party/.*" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata -format=html -output-dir=$(COVERAGE_DIR)/html -show-expansions -show-line-counts-or-regions
	@echo "Open: $(COVERAGE_DIR)/html/index.html"
else ifeq ($(IS_GCC),1)
	@mkdir -p $(COVERAGE_DIR)/html
	@echo "GCC coverage: generating HTML with gcovr (if available)"
	@which gcovr >/dev/null 2>&1 && gcovr --html-details $(COVERAGE_DIR)/html/index.html --exclude 'third_party/.*' || echo "gcovr not found; install it to generate GCC coverage reports"
else
	@echo "Coverage not supported for current compiler"
endif

clean:
	rm -rf $(BIN_DIR) $(COVERAGE_DIR)

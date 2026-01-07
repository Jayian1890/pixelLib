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
# Default: assume non-Windows
IS_WINDOWS := 0
# Prefer uname-based detection when available (MSYS2/MinGW/Git Bash, etc.)
ifneq (,$(findstring NT,$(UNAME_S)))
  IS_WINDOWS := 1
else ifneq (,$(findstring MINGW,$(UNAME_S)))
  IS_WINDOWS := 1
else ifeq ($(OS),Windows_NT)
  # Fallback for environments that set OS=Windows_NT
  IS_WINDOWS := 1
endif

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

# Detect if we need to link libstdc++fs for filesystem support
# GCC < 9 requires explicit linking of stdc++fs
LIBS :=
ifeq ($(IS_GCC),1)
  GCC_VERSION := $(shell $(CXX) -dumpversion 2>/dev/null | cut -d. -f1)
  ifeq ($(shell test $(GCC_VERSION) -lt 9 2>/dev/null && echo 1),1)
    LIBS += -lstdc++fs
  endif
endif

# Add Windows socket library if on Windows
ifeq ($(IS_WINDOWS),1)
  LIBS += -lws2_32
endif

TEST_DIR := tests
BIN_DIR := build/tests
COVERAGE_DIR := build/coverage

ALL_SOURCES := $(wildcard $(TEST_DIR)/*.cc)
SOURCES := $(ALL_SOURCES)
TEST_BIN := $(BIN_DIR)/pixellib_tests

.PHONY: all test run-tests coverage clean doctest

all: clean test coverage doctest

$(TEST_BIN): $(SOURCES) third_party/doctest/doctest.h | $(BIN_DIR)
	$(CXX) -std=$(STD) $(WARN) $(DBG) $(INCLUDES) $(COV_CFLAGS) $(SOURCES) -o $(TEST_BIN) $(LIBS)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

third_party/doctest/doctest.h:
	@mkdir -p third_party/doctest
	@./tools/get_doctest.sh

# Explicit target to fetch doctest header
doctest: third_party/doctest/doctest.h
	@echo "doctest header present at third_party/doctest/doctest.h"

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
	@$(LLVM_COV) export --format=lcov --ignore-filename-regex="(third_party/.*|tests/.*)" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata > $(COVERAGE_DIR)/lcov.info
	@sed "s|SF:$(CURDIR)/|SF:|g" $(COVERAGE_DIR)/lcov.info > $(COVERAGE_DIR)/lcov.relative.info
	@echo "Also created: $(COVERAGE_DIR)/lcov.relative.info (workspace-relative paths)";
	@echo "Generating HTML -> $(COVERAGE_DIR)/html";
	@$(LLVM_COV) show --ignore-filename-regex="(third_party/.*|tests/.*)" $(TEST_BIN) -instr-profile=$(COVERAGE_DIR)/coverage.profdata -format=html -output-dir=$(COVERAGE_DIR)/html -show-expansions -show-line-counts-or-regions -Xdemangler c++filt
	@echo "Open: $(COVERAGE_DIR)/html/index.html"
else ifeq ($(IS_GCC),1)
	@mkdir -p $(COVERAGE_DIR)/html
	@echo "GCC coverage: generating HTML with gcovr (if available)"
	@which gcovr >/dev/null 2>&1 && gcovr --html-details $(COVERAGE_DIR)/html/index.html --exclude 'third_party/.*' --exclude 'tests/.*' || echo "gcovr not found; install it to generate GCC coverage HTML."
else
	@echo "Coverage not supported for current compiler"
endif

.PHONY: lcov-rel
lcov-rel: coverage
	@sed "s|SF:$(CURDIR)/|SF:|g" $(COVERAGE_DIR)/lcov.info > $(COVERAGE_DIR)/lcov.relative.info
	@echo "Generated $(COVERAGE_DIR)/lcov.relative.info"

clean:
	rm -rf $(BIN_DIR) $(COVERAGE_DIR)
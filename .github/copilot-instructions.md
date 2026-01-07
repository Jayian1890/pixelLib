# pixelLib — Copilot Instructions

## Purpose
Concise contributor guidance for this header-only C++ utility library (filesystem, JSON, logging, network).

## Key Principles
- Header-only: all public headers live in `include/` and should not require linking.
- C++ standard: target C++17 only.
- Minimal dependencies: only Doctest is allowed for tests; it is fetched via `tools/get_doctest.sh`.

## Build & Test
- Build and test with the repository Makefile: `make test`, `make run-tests`, `make coverage`.
- Tests live in `tests/` and use Doctest; coverage uses `gcov`/`lcov` and outputs under `build/coverage/`.
- For local debug builds add `-O0 -g` to `CXXFLAGS` in the Makefile.

## Conventions
- Headers: `.hpp` in `include/`.
- Tests: `.cc` in `tests/`.
- Keep modules self-contained; prefer small, well-documented header APIs.

## CI / Workflow
- CI uses GitHub Actions (see `.github/workflows/build-test.yml`) to run the Makefile targets.

## Rules
- Preserve header-only design — no compiled library artifacts in source.
- Aim for high test coverage; tests must be deterministic and self-contained (no external services).
 - Tests for a given module must live in a single file under `tests/` (e.g., `tests/test_network.cc`). Do not split a module's tests across multiple files.

## Notes for Copilot / Assistants
- Focus on minimal, precise edits: fix root causes, not cosmetic workarounds.
- When editing files use the repository Makefile to run tests after changes.
- Keep changes consistent with existing style and naming.

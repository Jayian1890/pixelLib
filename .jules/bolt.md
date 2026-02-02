# Bolt Performance Journal

## YYYY-MM-DD - Optimized logging message formatting
**Learning:** std::format is significantly faster than std::ostringstream for string formatting, especially with chrono types.
**Action:** Use std::format instead of std::ostringstream for log message formatting in future optimizations.

## 2026-02-02 - IPv4 parser pointer optimization
**Learning:** Replacing the index-based IPv4 parser with a pointer-based, allocation-free parser yielded a measurable improvement in hot loops. Local microbench (100k iterations) showed valid input: ~16ms → ~6ms, invalid: ~5ms → ~3ms (~2–3x faster).
**Action:** Favor allocation-free parsing for other hot string-processing functions (e.g., URL encoding/decoding) and add microbenchmarks to tests to catch regressions early.
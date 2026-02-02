# Bolt Performance Journal

## YYYY-MM-DD - Optimized logging message formatting
**Learning:** std::format is significantly faster than std::ostringstream for string formatting, especially with chrono types.
**Action:** Use std::format instead of std::ostringstream for log message formatting in future optimizations.

## 2026-02-02 - IPv4 parser pointer optimization
**Learning:** Replacing the index-based IPv4 parser with a pointer-based, allocation-free parser yielded a measurable improvement in hot loops. Local microbench (100k iterations) showed valid input: ~16ms → ~6ms, invalid: ~5ms → ~3ms (~2–3x faster).
**Action:** Favor allocation-free parsing for other hot string-processing functions (e.g., URL encoding/decoding) and add microbenchmarks to tests to catch regressions early.

## 2026-02-02 - Log level check lock-free early exit
**Learning:** Replacing mutex-protected level checks with a lock-free atomic read removes frequent mutex acquisitions on filtered log paths. Microbenchmark (`tests::LogFilterPerformance`, 100k iterations) measured ~10ms for filtered debug calls on Linux x86_64.
**Action:** Prefer atomic operations on small shared flags that are read frequently and updated rarely (e.g., log level). Add similar microbenchmarks when optimizing hot logging paths.
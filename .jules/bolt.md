# Bolt Performance Journal

## YYYY-MM-DD - Optimized logging message formatting
**Learning:** std::format is significantly faster than std::ostringstream for string formatting, especially with chrono types.
<<<<<<< Updated upstream
=======
<<<<<<< Updated upstream
**Action:** Use std::format instead of std::ostringstream for log message formatting in future optimizations.
=======
>>>>>>> Stashed changes
**Action:** Use std::format instead of std::ostringstream for log message formatting in future optimizations.

## 2026-02-02 - IPv4 parser pointer optimization
**Learning:** Replacing the index-based IPv4 parser with a pointer-based, allocation-free parser yielded a measurable improvement in hot loops. Local microbench (100k iterations) showed valid input: ~16ms → ~6ms, invalid: ~5ms → ~3ms (~2–3x faster).
**Action:** Favor allocation-free parsing for other hot string-processing functions (e.g., URL encoding/decoding) and add microbenchmarks to tests to catch regressions early.

## 2026-02-02 - Log level check lock-free early exit
**Learning:** Replacing mutex-protected level checks with a lock-free atomic read removes frequent mutex acquisitions on filtered log paths. Microbenchmark (`tests::LogFilterPerformance`, 100k iterations) measured ~10ms for filtered debug calls on Linux x86_64.
<<<<<<< Updated upstream
**Action:** Prefer atomic operations on small shared flags that are read frequently and updated rarely (e.g., log level). Add similar microbenchmarks when optimizing hot logging paths.
=======
**Action:** Prefer atomic operations on small shared flags that are read frequently and updated rarely (e.g., log level). Add similar microbenchmarks when optimizing hot logging paths.

## 2026-02-03 - Avoid temporary std::string allocations on filtered logs
**Learning:** Adding `const char*` overloads for log methods and checking the level before constructing a `std::string` avoids temporary allocations when logging string literals that are filtered by log level. This reduces allocation overhead on hot, filtered paths without changing the API behavior for callers using `std::string`.
**Action:** Add `const char*` overloads for common logging entry-points in other modules if needed and add microbenchmarks to `tests/` to track regression."
>>>>>>> Stashed changes
>>>>>>> Stashed changes

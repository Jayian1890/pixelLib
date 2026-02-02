# Bolt Performance Journal

## YYYY-MM-DD - Optimized logging message formatting
**Learning:** std::format is significantly faster than std::ostringstream for string formatting, especially with chrono types.
**Action:** Use std::format instead of std::ostringstream for log message formatting in future optimizations.
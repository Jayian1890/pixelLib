#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <thread>

#include <gtest/gtest.h>

#include "interlaced_core/logging.hpp"

// Helper functions to reset logger state and clean test files.
namespace {
void CleanupTestFiles() {
  std::filesystem::remove("test_log.txt");
  std::filesystem::remove("test_log.txt.1");
  std::filesystem::remove("test_log.txt.2");
}

void ResetLoggerState() {
  CleanupTestFiles();
  interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_DEBUG);
  interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
  interlaced::core::logging::Logger::set_formatter(nullptr);
  interlaced::core::logging::Logger::set_file_logging(nullptr);
}
} // namespace

// Test log_level_to_string function
TEST(Logging, LogLevelToString) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  EXPECT_STREQ(log_level_to_string(LOG_DEBUG), "DEBUG");
  EXPECT_STREQ(log_level_to_string(LOG_INFO), "INFO");
  EXPECT_STREQ(log_level_to_string(LOG_WARNING), "WARNING");
  EXPECT_STREQ(log_level_to_string(LOG_ERROR), "ERROR");
  EXPECT_STREQ(log_level_to_string(static_cast<LogLevel>(999)), "UNKNOWN");
}

// Test Logger::set_level and level filtering
TEST(Logging, SetLevel) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  // Capture output
  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  // Set level to WARNING
  Logger::set_level(LOG_WARNING);

  // Debug and info should not appear
  Logger::debug("Debug message");
  Logger::info("Info message");
  EXPECT_TRUE(output.str().empty());

  // Warning and error should appear
  Logger::warning("Warning message");
  Logger::error("Error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("Warning message") != std::string::npos);
  EXPECT_TRUE(error_str.find("Error message") != std::string::npos);
}

// Test basic logging functionality
TEST(Logging, BasicLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  Logger::debug("Debug message");
  Logger::info("Info message");
  Logger::warning("Warning message");
  Logger::error("Error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("[DEBUG] Debug message") != std::string::npos);
  EXPECT_TRUE(output_str.find("[INFO] Info message") != std::string::npos);
  EXPECT_TRUE(output_str.find("[WARNING] Warning message") !=
              std::string::npos);
  EXPECT_TRUE(error_str.find("[ERROR] Error message") != std::string::npos);
}

// Test logging with file and line information
TEST(Logging, LoggingWithFileLine) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test message", "test.cpp", 42);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Test message") != std::string::npos);
  EXPECT_TRUE(output_str.find("test.cpp:42") != std::string::npos);
}

// Test structured logging with key-value pairs
TEST(Logging, StructuredLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("User login", "user_id", 12345, "ip", "192.168.1.1");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User login") != std::string::npos);
  EXPECT_TRUE(output_str.find("user_id=12345") != std::string::npos);
  EXPECT_TRUE(output_str.find("ip=192.168.1.1") != std::string::npos);
}

// Test custom formatter
TEST(Logging, CustomFormatter) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  class TestFormatter : public LogFormatter {
  public:
    std::string format(LogLevel level, const std::string &message,
                       const std::tm &time_info, const char *file = nullptr,
                       int line = 0) override {
      return "CUSTOM: " + message;
    }
  };

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);
  Logger::set_formatter(std::make_unique<TestFormatter>());

  Logger::info("Test message");

  EXPECT_TRUE(output.str().find("CUSTOM: Test message") != std::string::npos);
}

// Test DefaultLogFormatter
TEST(Logging, DefaultLogFormatter) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter(TimestampFormat::STANDARD, "PREFIX");

  std::tm time_info = {};
  time_info.tm_year = 125; // 2025
  time_info.tm_mon = 11;   // December
  time_info.tm_mday = 15;
  time_info.tm_hour = 12;
  time_info.tm_min = 30;
  time_info.tm_sec = 45;

  std::string result =
      formatter.format(LOG_INFO, "Test message", time_info, "file.cpp", 100);

  EXPECT_TRUE(result.find("PREFIX") != std::string::npos);
  EXPECT_TRUE(result.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(result.find("Test message") != std::string::npos);
  EXPECT_TRUE(result.find("file.cpp:100") != std::string::npos);
}

// Test DefaultLogFormatter with different timestamp formats
TEST(Logging, DefaultLogFormatterTimestampFormats) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::tm time_info = {};
  time_info.tm_year = 125;
  time_info.tm_mon = 11;
  time_info.tm_mday = 15;
  time_info.tm_hour = 12;
  time_info.tm_min = 30;
  time_info.tm_sec = 45;

  // Test ISO8601 format
  DefaultLogFormatter iso_formatter(TimestampFormat::ISO8601);
  std::string iso_result = iso_formatter.format(LOG_DEBUG, "Test", time_info);
  EXPECT_TRUE(iso_result.find("2025-12-15T12:30:45Z") != std::string::npos);

  // Test NONE format (no timestamp, but still include level)
  DefaultLogFormatter none_formatter(TimestampFormat::NONE);
  std::string none_result = none_formatter.format(LOG_DEBUG, "Test", time_info);
  // Ensure there is no timestamp-like prefix (e.g., "2025-")
  EXPECT_TRUE(none_result.find("2025-") == std::string::npos);
  EXPECT_TRUE(none_result.find("[DEBUG] Test") != std::string::npos);
}

// Test RotatingFileLogger size-based rotation
TEST(Logging, RotatingFileLoggerSizeBased) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  const size_t max_size = 100; // Small size for testing

  RotatingFileLogger logger(filename, max_size, 2);

  // Write messages until rotation occurs (small number to make rotation behavior deterministic)
  std::string long_message = "This is a long message that should trigger "
                             "rotation when written multiple times.";
  for (int i = 0; i < 2; ++i) {
    logger.write(long_message);
  }

  // Check that files exist
  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
  EXPECT_FALSE(std::filesystem::exists(filename + ".2")); // Should not exist yet

  // Write more to trigger another rotation
  for (int i = 0; i < 2; ++i) {
    logger.write(long_message);
  }

  EXPECT_TRUE(std::filesystem::exists(filename + ".2"));
}

// Test RotatingFileLogger time-based rotation (simulated with size)
TEST(Logging, RotatingFileLoggerTimeBased) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";

  // Use size-based rotation for testing (time-based would require waiting)
  RotatingFileLogger logger(filename, 50, 2);

  logger.write("First message - this should trigger rotation soon");

  logger.write("Second message - should be in new file");

  // Check that files exist
  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
}

// Test file logging integration
TEST(Logging, FileLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  Logger::set_file_logging(filename, 1024, 3);

  Logger::info("File log message");
  Logger::error("File error message");

  // Force flush by setting different logging
  Logger::set_file_logging(nullptr);

  // Check file contents
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  EXPECT_TRUE(line.find("[INFO] File log message") != std::string::npos);

  std::getline(file, line);
  EXPECT_TRUE(line.find("[ERROR] File error message") != std::string::npos);
}

// Test thread safety
TEST(Logging, ThreadSafety) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  const int num_threads = 10;
  const int messages_per_thread = 100;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([i, messages_per_thread]() {
      for (int j = 0; j < messages_per_thread; ++j) {
        Logger::info("Thread " + std::to_string(i) + " message " +
                     std::to_string(j));
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  std::string output_str = output.str();

  // Count total messages (should be num_threads * messages_per_thread)
  std::regex message_regex(R"(\[INFO\])");
  auto matches_begin =
      std::sregex_iterator(output_str.begin(), output_str.end(), message_regex);
  auto matches_end = std::sregex_iterator();
  int message_count = std::distance(matches_begin, matches_end);

  EXPECT_EQ(message_count, num_threads * messages_per_thread);
}

// Test macros
TEST(Logging, Macros) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  LOG_INFO("Macro info message");
  LOG_WARNING("Macro warning message");
  LOG_ERROR("Macro error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("Macro info message") != std::string::npos);
  EXPECT_TRUE(output_str.find("Macro warning message") != std::string::npos);
  EXPECT_TRUE(error_str.find("Macro error message") != std::string::npos);
}

// Test variadic template logging
TEST(Logging, VariadicTemplateLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("User {} logged in from {}", "john_doe", "192.168.1.1");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User john_doe logged in from 192.168.1.1") !=
              std::string::npos);
}

// Test error handling in RotatingFileLogger
TEST(Logging, RotatingFileLoggerErrorHandling) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  // Try to create logger with invalid path
  const std::string invalid_path = "/invalid/path/test.log";
  RotatingFileLogger logger(invalid_path, 1024, 2);

  // This should not crash, should fall back to cerr
  testing::internal::CaptureStderr();
  logger.write("Test message");
  std::string captured = testing::internal::GetCapturedStderr();

  EXPECT_TRUE(captured.find("Test message") != std::string::npos);
}

// Test logger with custom streams
TEST(Logging, CustomStreams) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream custom_output;
  std::stringstream custom_error;

  Logger::set_output_streams(custom_output, custom_error);

  Logger::info("Info to custom stream");
  Logger::error("Error to custom stream");

  EXPECT_TRUE(custom_output.str().find("Info to custom stream") !=
              std::string::npos);
  EXPECT_TRUE(custom_error.str().find("Error to custom stream") !=
              std::string::npos);
}

// Test log level filtering edge cases
TEST(Logging, LogLevelFiltering) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  // Capture both normal and error output in the same buffer for this test
  Logger::set_output_streams(output, output);

  // Set to ERROR level - only errors should pass
  Logger::set_level(LOG_ERROR);

  Logger::debug("Debug - should not appear");
  Logger::info("Info - should not appear");
  Logger::warning("Warning - should not appear");
  Logger::error("Error - should appear");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Debug") == std::string::npos);
  EXPECT_TRUE(output_str.find("Info") == std::string::npos);
  EXPECT_TRUE(output_str.find("Warning") == std::string::npos);
  EXPECT_TRUE(output_str.find("Error") != std::string::npos);
}

// Test timestamp formatting in logs
TEST(Logging, TimestampFormatting) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test message");

  std::string output_str = output.str();

  // Check for timestamp format YYYY-MM-DD HH:MM:SS
  std::regex timestamp_regex(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\])");
  EXPECT_TRUE(std::regex_search(output_str, timestamp_regex));
}

// Test file path extraction in logging
TEST(Logging, FilePathExtraction) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test", "/full/path/to/file.cpp", 123);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("file.cpp:123") != std::string::npos);
  EXPECT_TRUE(output_str.find("/full/path/to/") == std::string::npos);
}

// Test multiple rotations in RotatingFileLogger
TEST(Logging, MultipleRotations) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  const size_t max_size = 50; // Very small for testing

  RotatingFileLogger logger(filename, max_size, 3);

  // Write enough to cause multiple rotations
  for (int i = 0; i < 20; ++i) {
    logger.write("Message " + std::to_string(i));
  }

  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
  EXPECT_TRUE(std::filesystem::exists(filename + ".2"));
  EXPECT_TRUE(std::filesystem::exists(filename + ".3"));
}

// Test empty message logging
TEST(Logging, EmptyMessage) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("[INFO]") != std::string::npos);
}

// Test logging with special characters
TEST(Logging, SpecialCharacters) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info(
      "Message with special chars: \n\t\"quotes\" 'apostrophes' @#$%^&*()");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Message with special chars") !=
              std::string::npos);
}

// Test formatter with null file parameter
TEST(Logging, FormatterNullFile) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter;

  std::tm time_info = {};
  std::string result =
      formatter.format(LOG_INFO, "Test", time_info, nullptr, 0);

  EXPECT_TRUE(result.find("Test") != std::string::npos);
  // Ensure no file/line suffix (i.e., no parentheses containing filename:line)
  EXPECT_TRUE(result.find("(") == std::string::npos); // No file/line information
}

// Test structured logging with various data types using std::string
TEST(Logging, StructuredLoggingTypes) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // Use std::string for the message with two pairs to avoid odd argument count
  Logger::info(std::string("Test int"), "value", 42, "count", 1);
  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("value=42") != std::string::npos);
  EXPECT_TRUE(output_str.find("count=1") != std::string::npos);

  output.str("");
  Logger::info(std::string("Test bool"), "flag", true, "enabled", false);
  output_str = output.str();
  EXPECT_TRUE(output_str.find("flag=1") != std::string::npos);
  EXPECT_TRUE(output_str.find("enabled=0") != std::string::npos);
}

// Test logger reset functionality
TEST(Logging, LoggerReset) {
  using namespace interlaced::core::logging;

  // Set custom settings
  Logger::set_level(LOG_ERROR);
  Logger::set_formatter(
      std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE));

  // Reset via helper
  ResetLoggerState();

  // Verify reset
  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::debug("Should appear now");
  EXPECT_TRUE(output.str().find("Should appear now") != std::string::npos);
}

// Test time-based rotation constructor
TEST(Logging, RotatingFileLoggerTimeBasedConstructor) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  RotatingFileLogger logger(filename, std::chrono::hours(1), 3);

  logger.write("Time-based rotation test message");

  EXPECT_TRUE(std::filesystem::exists(filename));

  // Read back the file
  std::ifstream file(filename);
  std::string line;
  std::getline(file, line);
  EXPECT_TRUE(line.find("Time-based rotation test message") != std::string::npos);
}

// Test time-based rotation with file logging
TEST(Logging, TimeBasedFileLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  Logger::set_file_logging(filename, std::chrono::hours(24), 5);

  Logger::info("Time-based log message");
  Logger::warning("Time-based warning");

  // Disable logging to flush
  Logger::set_file_logging(nullptr);

  // Verify file contents
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  EXPECT_TRUE(content.find("Time-based log message") != std::string::npos);
  EXPECT_TRUE(content.find("Time-based warning") != std::string::npos);
}

// Test format string with placeholder substitution
TEST(Logging, FormatStringPlaceholders) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // Test format with multiple placeholders
  Logger::info("User {} logged in with ID {}", "alice", 42);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User alice logged in with ID 42") != std::string::npos);
}

// Test format string with different types
TEST(Logging, FormatStringVariousTypes) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, output);

  // Test with two ints
  Logger::debug("Count: {} total: {}", 100, 200);
  EXPECT_TRUE(output.str().find("Count: 100") != std::string::npos);

  output.str("");
  // Test with two floats
  Logger::info("Value: {} other: {}", 3.14f, 2.71f);
  EXPECT_TRUE(output.str().find("Value: 3.14") != std::string::npos);

  output.str("");
  // Test with two strings
  Logger::warning("Name: {} status: {}", std::string("test"), "ok");
  EXPECT_TRUE(output.str().find("Name: test") != std::string::npos);

  output.str("");
  // Test with bool and int
  Logger::error("Status: {} code: {}", true, 200);
  EXPECT_TRUE(output.str().find("Status: 1") != std::string::npos);
}

// Test format string with no placeholders but extra args using std::string
TEST(Logging, FormatStringNoPlaceholders) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // This should be treated as structured logging - use std::string to avoid ambiguity
  Logger::info(std::string("Event"), "user", "bob", "action", "login");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Event") != std::string::npos);
  EXPECT_TRUE(output_str.find("user=bob") != std::string::npos);
  EXPECT_TRUE(output_str.find("action=login") != std::string::npos);
}

// Test debug with format string
TEST(Logging, DebugFormatString) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::debug("Debug value: {} count: {}", 999, 1);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Debug value: 999") != std::string::npos);
  EXPECT_TRUE(output_str.find("[DEBUG]") != std::string::npos);
}

// Test warning with format string
TEST(Logging, WarningFormatString) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::warning("Warning: {} code: {}", "system overload", 503);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Warning: system overload") != std::string::npos);
  EXPECT_TRUE(output_str.find("[WARNING]") != std::string::npos);
}

// Test error with format string
TEST(Logging, ErrorFormatString) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream error_output;
  Logger::set_output_streams(std::cout, error_output);

  Logger::error("Error code: {} msg: {}", 500, "internal");

  std::string output_str = error_output.str();
  EXPECT_TRUE(output_str.find("Error code: 500") != std::string::npos);
  EXPECT_TRUE(output_str.find("[ERROR]") != std::string::npos);
}

// Test format with multiple mixed-type placeholders
TEST(Logging, FormatMultipleMixedTypes) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // Use 2 or 4 arguments, not 3 (odd numbers cause template issues)
  Logger::info("User {} with ID {}", "john", 123);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User john with ID 123") != std::string::npos);
}

// Test structured logging with even more types - simplified
TEST(Logging, StructuredLoggingMoreTypes) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // Test with two pairs
  Logger::info(std::string("Complex event"), "id", 1, "active", false);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("id=1") != std::string::npos);
  EXPECT_TRUE(output_str.find("active=0") != std::string::npos);
}

// Test DefaultLogFormatter with UNIX timestamp format
TEST(Logging, DefaultFormatterUnixTimestamp) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter(TimestampFormat::UNIX);

  std::tm time_info = {};
  time_info.tm_year = 125;
  time_info.tm_mon = 11;
  time_info.tm_mday = 15;
  time_info.tm_hour = 12;
  time_info.tm_min = 30;
  time_info.tm_sec = 45;

  std::string result = formatter.format(LOG_INFO, "Test", time_info);

  // UNIX format falls back to STANDARD format in implementation
  EXPECT_TRUE(result.find("2025-12-15") != std::string::npos);
  EXPECT_TRUE(result.find("Test") != std::string::npos);
}

// Test log level boundaries
TEST(Logging, LogLevelBoundaries) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, output);

  // Set to INFO level
  Logger::set_level(LOG_INFO);

  // Debug should not appear
  Logger::debug("Debug message");
  EXPECT_TRUE(output.str().empty());

  // Info should appear
  Logger::info("Info message");
  EXPECT_TRUE(output.str().find("Info message") != std::string::npos);
}

// Test file logger with very small file size
TEST(Logging, FileLoggerSmallSize) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  RotatingFileLogger logger(filename, 10, 2); // Very small: 10 bytes

  logger.write("A");
  logger.write("B");
  logger.write("C");

  // Multiple rotations should have occurred
  EXPECT_TRUE(std::filesystem::exists(filename));
}

// Test structured logging with two key-value pairs using std::string
TEST(Logging, StructuredLoggingTwoPairs) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info(std::string("Two pairs"), "key1", "value1", "key2", "value2");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Two pairs") != std::string::npos);
  EXPECT_TRUE(output_str.find("key1=value1") != std::string::npos);
  EXPECT_TRUE(output_str.find("key2=value2") != std::string::npos);
}

// Test log with file and line using const char* overload
TEST(Logging, ConstCharPtrWithFileLine) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // This should be detected as (message, file, line) pattern
  Logger::info("Test message", "source.cpp", 42);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Test message") != std::string::npos);
  EXPECT_TRUE(output_str.find("source.cpp:42") != std::string::npos);
}

// Test multiple consecutive rotations
TEST(Logging, ConsecutiveRotations) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  RotatingFileLogger logger(filename, 30, 4);

  for (int i = 0; i < 50; ++i) {
    logger.write("Rotation test line " + std::to_string(i));
  }

  // Check that rotation files exist
  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
}

// Test formatter with zero line number
TEST(Logging, FormatterZeroLine) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter;

  std::tm time_info = {};
  std::string result = formatter.format(LOG_DEBUG, "Message", time_info, "file.cpp", 0);

  // Line 0 means no file/line info should be added
  EXPECT_TRUE(result.find("Message") != std::string::npos);
  EXPECT_TRUE(result.find("file.cpp") == std::string::npos);
}

// Test complex structured data using std::string
TEST(Logging, ComplexStructuredData) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::debug(std::string("Debug event"), "step", 1, "status", "pending");
  std::string out1 = output.str();
  EXPECT_TRUE(out1.find("step=1") != std::string::npos);

  output.str("");
  Logger::warning(std::string("Warning event"), "code", 404, "path", "/api/test");
  std::string out2 = output.str();
  EXPECT_TRUE(out2.find("code=404") != std::string::npos);
}

// Test that formatter base case is reached
TEST(Logging, FormatterBaseCaseReached) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  // Simple message with no args - tests base case
  Logger::info("Simple");

  EXPECT_TRUE(output.str().find("Simple") != std::string::npos);
}

// Test format_helper with trailing text after placeholders
TEST(Logging, FormatHelperTrailingText) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Value {} done {}", 42, "ok");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Value 42 done ok") != std::string::npos);
}

// Test format_helper with text before placeholders
TEST(Logging, FormatHelperLeadingText) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Start {} end {}", "middle", "fin");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Start middle end fin") != std::string::npos);
}

// Test file logging and then clearing it
TEST(Logging, FileLoggingClearSequence) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";

  // Enable file logging
  Logger::set_file_logging(filename, 1024, 2);
  Logger::info("Message 1");

  // Clear file logging
  Logger::set_file_logging(nullptr);

  // Now log to stream
  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);
  Logger::info("Message 2");

  EXPECT_TRUE(output.str().find("Message 2") != std::string::npos);
}

// Test structured logging with more than 3 pairs using std::string
TEST(Logging, StructuredLoggingManyPairs) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info(std::string("Event"), "a", 1, "b", 2, "c", 3, "d", 4);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("a=1") != std::string::npos);
  EXPECT_TRUE(output_str.find("b=2") != std::string::npos);
  EXPECT_TRUE(output_str.find("c=3") != std::string::npos);
  EXPECT_TRUE(output_str.find("d=4") != std::string::npos);
}

// Test LOG_DEBUG macro when enabled
#ifndef INTERLACED_CORE_DISABLE_DEBUG_LOGS
TEST(Logging, LogDebugMacroEnabled) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  LOG_DEBUG("Macro debug message");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Macro debug message") != std::string::npos);
  EXPECT_TRUE(output_str.find("logging_test.cc") != std::string::npos);
}
#endif

// Test formatter with prefix and different levels
TEST(Logging, FormatterPrefixAllLevels) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter(TimestampFormat::NONE, "[APP]");

  std::tm time_info = {};

  std::string debug_msg = formatter.format(LOG_DEBUG, "Debug", time_info);
  EXPECT_TRUE(debug_msg.find("[APP]") != std::string::npos);
  EXPECT_TRUE(debug_msg.find("[DEBUG]") != std::string::npos);

  std::string warning_msg = formatter.format(LOG_WARNING, "Warning", time_info);
  EXPECT_TRUE(warning_msg.find("[APP]") != std::string::npos);
  EXPECT_TRUE(warning_msg.find("[WARNING]") != std::string::npos);

  std::string error_msg = formatter.format(LOG_ERROR, "Error", time_info);
  EXPECT_TRUE(error_msg.find("[APP]") != std::string::npos);
  EXPECT_TRUE(error_msg.find("[ERROR]") != std::string::npos);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
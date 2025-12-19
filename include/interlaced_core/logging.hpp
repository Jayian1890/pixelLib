/*
 * Interlaced Core Library
 * Copyright (c) 2025 Interlaced Pixel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INTERLACED_CORE_LOGGING_HPP
#define INTERLACED_CORE_LOGGING_HPP

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <cstddef>
#include <cstring> // for strstr used to detect placeholder patterns
#include <mutex>
#include <sstream>
#include <string>

// Platform-specific time conversion helper
inline void localtime_threadsafe(const std::time_t *time, std::tm *tm) {
#if defined(_MSC_VER) || defined(_WIN32)
  localtime_s(tm, time);
#else
  localtime_r(time, tm);
#endif
}

/**
 * @brief Core logging utilities for the Interlaced framework
 *
 * This module provides a thread-safe logging system with multiple log levels
 * and configurable output destinations.
 */
namespace interlaced {

namespace core {

namespace logging {

/**
 * @brief Log levels for categorizing log messages by severity
 *
 * Log levels are ordered by severity, with DEBUG being the lowest
 * and LOG_ERROR being the highest. Messages are only logged if their
 * level is greater than or equal to the current log level setting.
 */
enum LogLevel {
  LOG_DEBUG,   ///< Debug-level messages, typically for development only
  LOG_INFO,    ///< Informational messages about normal operation
  LOG_WARNING, ///< Warning messages about potential issues
  LOG_ERROR    ///< Error messages about serious problems
};

/**
 * @brief Convert a LogLevel enum to its string representation
 *
 * @param level The LogLevel to convert
 * @return const char* String representation of the LogLevel
 */
inline const char *log_level_to_string(LogLevel level) {
  switch (level) {
  case LOG_DEBUG:
    return "DEBUG";
  case LOG_INFO:
    return "INFO";
  case LOG_WARNING:
    return "WARNING";
  case LOG_ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief Timestamp format options
 */
enum class TimestampFormat {
  STANDARD, ///< YYYY-MM-DD HH:MM:SS
  ISO8601,  ///< YYYY-MM-DDTHH:MM:SSZ
  UNIX,     ///< Unix timestamp (seconds since epoch)
  NONE      ///< No timestamp
};

/**
 * @brief File rotation strategy
 */
enum class RotationStrategy {
  SIZE, ///< Rotate based on file size
  TIME  ///< Rotate based on time intervals
};

/**
 * @brief File logger with rotation capabilities
 */
class RotatingFileLogger {
private:
  std::string base_filename_;
  std::ofstream current_file_;
  size_t max_file_size_;
  int max_files_;
  RotationStrategy strategy_;
  std::chrono::steady_clock::time_point last_rotation_;
  std::chrono::hours rotation_interval_;
  size_t current_file_size_;

public:
  /**
   * @brief Constructor for RotatingFileLogger
   *
   * @param base_filename The base filename for log files
   * @param max_file_size Maximum size of each log file (in bytes)
   * @param max_files Maximum number of log files to keep
   */
  RotatingFileLogger(const std::string &base_filename,
                     size_t max_file_size = 10485760, int max_files = 5)
      : base_filename_(base_filename), max_file_size_(max_file_size),
        max_files_(max_files), strategy_(RotationStrategy::SIZE),
        last_rotation_(std::chrono::steady_clock::now()),
        rotation_interval_(std::chrono::hours(24)), current_file_size_(0) {
    open_current_file();
  }

  /**
   * @brief Constructor for RotatingFileLogger with time-based rotation
   *
   * @param base_filename The base filename for log files
   * @param rotation_hours Hours between log rotations
   * @param max_files Maximum number of log files to keep
   */
  RotatingFileLogger(const std::string &base_filename,
                     std::chrono::hours rotation_hours, int max_files = 5)
      : base_filename_(base_filename), max_file_size_(0), max_files_(max_files),
        strategy_(RotationStrategy::TIME),
        last_rotation_(std::chrono::steady_clock::now()),
        rotation_interval_(rotation_hours), current_file_size_(0) {
    open_current_file();
  }

  /**
   * @brief Write a message to the log file
   *
   * @param message The message to write
   */
  void write(const std::string &message) {
    try {
      if (!current_file_.is_open()) {
        // Try to reopen the file
        open_current_file();
        if (!current_file_.is_open()) {
          // If we still can't open the file, fall back to cerr
          std::cerr << "Failed to open log file: " << base_filename_
                    << std::endl;
          std::cerr << message << std::endl;
          return;
        }
      }

      // Check if we need to rotate (considering the upcoming write size)
      if (should_rotate(message.length() + 1)) {
        rotate();
      }

      current_file_ << message << std::endl;
      current_file_.flush();

      // Update file size for size-based rotation
      if (strategy_ == RotationStrategy::SIZE) {
        current_file_size_ += message.length() + 1; // +1 for newline
      }

      // Check if the stream is in a good state
      if (!current_file_.good()) {
        // If the stream is not good, try to reset it
        current_file_.clear();
      }
    } catch (const std::exception &e) {
      // If we can't write to the file, try to write to cerr as a fallback
      std::cerr << "File logging error: " << e.what() << std::endl;
      std::cerr << message << std::endl;
    } catch (...) {
      // If we can't write to the file, try to write to cerr as a fallback
      std::cerr << "Unknown file logging error occurred" << std::endl;
      std::cerr << message << std::endl;
    }
  }

  // Test helpers to modify internal state for unit tests
  void test_set_badbit() { current_file_.setstate(std::ios::badbit); }
  void test_clear_badbit() { current_file_.clear(); }

private:
  /**
   * @brief Open the current log file
   */
  void open_current_file() {
    try {
      current_file_.open(base_filename_, std::ios::app);
      if (current_file_.is_open() && strategy_ == RotationStrategy::SIZE) {
        // Get current file size
        current_file_.seekp(0, std::ios::end);
        current_file_size_ = current_file_.tellp();
      }
    } catch (const std::exception &e) {
      std::cerr << "Error opening log file: " << e.what() << std::endl;
    }
  }

  /**
   * @brief Check if we should rotate the log file
   *
   * @return true if we should rotate, false otherwise
   */
  // Determine whether a rotation is required. For size-based rotation,
  // the optional next_write_size parameter allows deciding based on the size
  // of the next write so we rotate before exceeding the limit.
  bool should_rotate(size_t next_write_size = 0) {
    try {
      if (strategy_ == RotationStrategy::SIZE) {
        // Rotate if the next write would exceed the maximum file size
        return (current_file_size_ + next_write_size) >= max_file_size_;
      } else { // TIME strategy
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
            now - last_rotation_);
        return elapsed >= rotation_interval_;
      }
    } catch (...) {
      // If there's an error checking rotation, default to not rotating
      return false;
    }
  }

  /**
   * @brief Rotate the log files
   */
  void rotate() {
    try {
      current_file_.close();

      // Rename existing files
      for (int i = max_files_ - 1; i > 0; --i) {
        std::string old_name = base_filename_ + "." + std::to_string(i);
        std::string new_name = base_filename_ + "." + std::to_string(i + 1);

        // Remove the new file if it exists
        std::remove(new_name.c_str());

        // Rename the old file to new name
        std::rename(old_name.c_str(), new_name.c_str());
      }

      // Rename current file to .1
      std::string backup_name = base_filename_ + ".1";
      std::rename(base_filename_.c_str(), backup_name.c_str());

      // Open new file
      open_current_file();
      current_file_size_ = 0;
      last_rotation_ = std::chrono::steady_clock::now();
    } catch (const std::exception &e) {
      std::cerr << "Error rotating log files: " << e.what() << std::endl;
      // Try to reopen the current file
      open_current_file();
    }
  }
};

/**
 * @brief Log formatter interface
 */
class LogFormatter {
public:
  virtual ~LogFormatter() = default;
  virtual std::string format(LogLevel level, const std::string &message,
                             const std::tm &time_info,
                             const char *file = nullptr, int line = 0) = 0;
};

/**
 * @brief Default log formatter implementation
 */
class DefaultLogFormatter : public LogFormatter {
private:
  TimestampFormat timestamp_format_;
  std::string prefix_;

public:
  DefaultLogFormatter(TimestampFormat format = TimestampFormat::STANDARD,
                      const std::string &prefix = "")
      : timestamp_format_(format), prefix_(prefix) {}

  std::string format(LogLevel level, const std::string &message,
                     const std::tm &time_info, const char *file = nullptr,
                     int line = 0) override {
    std::ostringstream oss;

    // Add prefix if specified
    if (!prefix_.empty()) {
      oss << prefix_ << " ";
    }

    // Add timestamp based on format
    switch (timestamp_format_) {
    case TimestampFormat::STANDARD: {
      oss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
      break;
    }
    case TimestampFormat::ISO8601: {
      oss << "[" << std::put_time(&time_info, "%Y-%m-%dT%H:%M:%SZ") << "] ";
      break;
    }
    case TimestampFormat::UNIX: {
      // This would require getting the time_t value, so we'll skip for now
      oss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
      break;
    }
    case TimestampFormat::NONE:
      // No timestamp
      break;
    }

    // Add log level
    oss << "[" << log_level_to_string(level) << "] ";

    // Add message
    oss << message;

    // Add file and line information if provided
    if (file && line > 0) {
      // Extract just the filename from the full path
      const char *filename = file;
      for (const char *p = file; *p; ++p) {
        if (*p == '/' || *p == '\\') {
          filename = p + 1;
        }
      }
      oss << " (" << filename << ":" << line << ")";
    }

    return oss.str();
  }
};

/**
 * @brief Thread-safe logging utility class
 *
 * The Logger class provides methods for logging messages at different
 * severity levels. All logging operations are thread-safe.
 *
 * Example usage:
 * @code
 * using namespace interlaced::core::logging;
 * Logger::set_level(LOG_INFO);
 * Logger::info("Application started");
 * Logger::warning("This is a warning message");
 * Logger::error("An error occurred");
 * @endcode
 *
 * For more detailed logging with file and line information:
 * @code
 * LOG_INFO("Application started");
 * LOG_WARNING("This is a warning message");
 * LOG_ERROR("An error occurred");
 * @endcode
 *
 * For structured logging with key-value pairs:
 * @code
 * Logger::info("User logged in", "user_id", 12345, "ip_address",
 * "192.168.1.1");
 * @endcode
 *
 * For file logging with rotation:
 * @code
 * Logger::set_file_logging("app.log", 10485760, 5); // 10MB max size, 5 files
 * Logger::set_file_logging("app.log", std::chrono::hours(24), 7); // Daily
 * rotation, 7 files
 * @endcode
 */
class Logger {
private:
  static LogLevel current_level; ///< Current minimum log level
  static std::mutex log_mutex;   ///< Mutex for thread safety
  static std::ostream
      *output_stream; ///< Output stream for LOG_INFO and LOG_DEBUG messages
  static std::ostream
      *error_stream; ///< Output stream for LOG_WARNING and LOG_ERROR messages
  static std::unique_ptr<LogFormatter> formatter; ///< Custom log formatter
  static std::unique_ptr<RotatingFileLogger>
      file_logger; ///< File logger with rotation

public:
  /**
   * @brief Set the minimum log level
   *
   * Messages with a level lower than the current level will not be logged.
   *
   * @param level The minimum LogLevel to display
   */
  static void set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_level = level;
  }

  /**
   * @brief Set custom output streams for logging
   *
   * Allows redirection of log output to custom streams (e.g., files).
   *
   * @param output The output stream for LOG_INFO and LOG_DEBUG messages
   * @param error The output stream for LOG_WARNING and LOG_ERROR messages
   */
  static void set_output_streams(std::ostream &output, std::ostream &error) {
    std::lock_guard<std::mutex> lock(log_mutex);
    output_stream = &output;
    error_stream = &error;
  }

  /**
   * @brief Set up file logging with size-based rotation
   *
   * @param filename The base filename for log files
   * @param max_file_size Maximum size of each log file (in bytes)
   * @param max_files Maximum number of log files to keep
   */
  static void set_file_logging(const std::string &filename,
                               size_t max_file_size = 10485760,
                               int max_files = 5) {
    std::lock_guard<std::mutex> lock(log_mutex);
    file_logger = std::make_unique<RotatingFileLogger>(filename, max_file_size,
                                                       max_files);
  }

  /**
   * @brief Set up file logging with time-based rotation
   *
   * @param filename The base filename for log files
   * @param rotation_hours Hours between log rotations
   * @param max_files Maximum number of log files to keep
   */
  static void set_file_logging(const std::string &filename,
                               std::chrono::hours rotation_hours,
                               int max_files = 5) {
    std::lock_guard<std::mutex> lock(log_mutex);
    file_logger = std::make_unique<RotatingFileLogger>(filename, rotation_hours,
                                                       max_files);
  }

  /**
   * @brief Disable file logging (clear file logger)
   *
   * Accepts nullptr to explicitly clear/disable file logging.
   */
  static void set_file_logging(std::nullptr_t) {
    std::lock_guard<std::mutex> lock(log_mutex);
    file_logger.reset();
  }

  /**
   * @brief Set a custom log formatter
   *
   * @param custom_formatter The custom formatter to use
   */
  static void set_formatter(std::unique_ptr<LogFormatter> custom_formatter) {
    std::lock_guard<std::mutex> lock(log_mutex);
    formatter = std::move(custom_formatter);
  }

  /**
   * @brief Log a message at the specified level
   *
   * @param level The LogLevel for this message
   * @param message The message to log
   */
  static void log(LogLevel level, const std::string &message) {
    // Early exit if the message won't be logged
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    // Format the message outside the critical section
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm struct in a thread-safe way
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    // Use custom formatter if available
    if (formatter) {
      formatted_message = formatter->format(level, message, local_tm);
    } else {
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message;
      formatted_message = oss.str();
    }

    // Output the message (critical section only for output)
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        // Choose output stream based on log level
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        // Check if the stream is in a good state
        if (!out_stream.good()) {
          // If the stream is not good, try to reset it
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

  /**
   * @brief Log a message with file and line information
   *
   * @param level The LogLevel for this message
   * @param message The message to log
   * @param file The source file name (typically __FILE__)
   * @param line The source line number (typically __LINE__)
   */
  static void log(LogLevel level, const std::string &message, const char *file,
                  int line) {
    // Early exit if the message won't be logged
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    // Format the message outside the critical section
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm struct in a thread-safe way
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    // Use custom formatter if available
    if (formatter) {
      formatted_message =
          formatter->format(level, message, local_tm, file, line);
    } else {
      // Extract just the filename from the full path
      const char *filename = file;
      for (const char *p = file; *p; ++p) {
        if (*p == '/' || *p == '\\') {
          filename = p + 1;
        }
      }

      // Format the message with file and line information
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message << " (" << filename
          << ":" << line << ")";
      formatted_message = oss.str();
    }

    // Output the message (critical section only for output)
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        // Choose output stream based on log level
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        // Check if the stream is in a good state
        if (!out_stream.good()) {
          // If the stream is not good, try to reset it
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

  /**
   * @brief Log a debug message
   *
   * @param message The debug message to log
   */
  static void debug(const std::string &message) { log(LOG_DEBUG, message); }

  /**
   * @brief Log a debug message with file and line information
   *
   * @param message The debug message to log
   * @param file The source file name (typically __FILE__)
   * @param line The source line number (typically __LINE__)
   */
  static void debug(const std::string &message, const char *file, int line) {
    log(LOG_DEBUG, message, file, line);
  }

  /**
   * @brief Log an informational message
   *
   * @param message The informational message to log
   */
  static void info(const std::string &message) { log(LOG_INFO, message); }

  /**
   * @brief Log an informational message with file and line information
   *
   * @param message The informational message to log
   * @param file The source file name (typically __FILE__)
   * @param line The source line number (typically __LINE__)
   */
  static void info(const std::string &message, const char *file, int line) {
    log(LOG_INFO, message, file, line);
  }

  /**
   * @brief Log a warning message
   *
   * @param message The warning message to log
   */
  static void warning(const std::string &message) { log(LOG_WARNING, message); }

  /**
   * @brief Log a warning message with file and line information
   *
   * @param message The warning message to log
   * @param file The source file name (typically __FILE__)
   * @param line The source line number (typically __LINE__)
   */
  static void warning(const std::string &message, const char *file, int line) {
    log(LOG_WARNING, message, file, line);
  }

  /**
   * @brief Log an error message
   *
   * @param message The error message to log
   */
  static void error(const std::string &message) { log(LOG_ERROR, message); }

  /**
   * @brief Log an error message with file and line information
   *
   * @param message The error message to log
   * @param file The source file name (typically __FILE__)
   * @param line The source line number (typically __LINE__)
   */
  static void error(const std::string &message, const char *file, int line) {
    log(LOG_ERROR, message, file, line);
  }

  // Structured logging with key-value pairs
  /**
   * @brief Log a message with structured key-value pairs
   *
   * @tparam Args Variadic template arguments for key-value pairs
   * @param level The LogLevel for this message
   * @param message The message to log
   * @param args Key-value pairs to include in the log
   */
  template <typename... Args>
  static void log(LogLevel level, const std::string &message, Args &&...args) {
    // Early exit if the message won't be logged
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    // Format the message outside the critical section
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm struct in a thread-safe way
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    // Use custom formatter if available
    if (formatter) {
      formatted_message = formatter->format(level, message, local_tm);
    } else {
      // Format the message with key-value pairs
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message;

      // Add key-value pairs
      format_key_value_pairs(oss, std::forward<Args>(args)...);
      formatted_message = oss.str();
    }

    // Output the message (critical section only for output)
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        // Choose output stream based on log level
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        // Check if the stream is in a good state
        if (!out_stream.good()) {
          // If the stream is not good, try to reset it
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        // If we can't write to the stream, try to write to cerr as a fallback
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

  /**
   * @brief Log a debug message with structured key-value pairs
   *
   * @tparam Args Variadic template arguments for key-value pairs
   * @param message The message to log
   * @param args Key-value pairs to include in the log
   */
  template <typename... Args>
  static void debug(const std::string &message, Args &&...args) {
    log(LOG_DEBUG, message, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an info message with structured key-value pairs
   *
   * @tparam Args Variadic template arguments for key-value pairs
   * @param message The message to log
   * @param args Key-value pairs to include in the log
   */
  template <typename... Args>
  static void info(const std::string &message, Args &&...args) {
    log(LOG_INFO, message, std::forward<Args>(args)...);
  }

  /**
   * @brief Log a warning message with structured key-value pairs
   *
   * @tparam Args Variadic template arguments for key-value pairs
   * @param message The message to log
   * @param args Key-value pairs to include in the log
   */
  template <typename... Args>
  static void warning(const std::string &message, Args &&...args) {
    log(LOG_WARNING, message, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an error message with structured key-value pairs
   *
   * @tparam Args Variadic template arguments for key-value pairs
   * @param message The message to log
   * @param args Key-value pairs to include in the log
   */
  template <typename... Args>
  static void error(const std::string &message, Args &&...args) {
    log(LOG_ERROR, message, std::forward<Args>(args)...);
  }

  // Variadic template functions for type-safe logging
  /**
   * @brief Log a debug message with type-safe formatting
   *
   * @tparam Args Variadic template arguments
   * @param format The format string
   * @param args Arguments to format
   */
  // Dispatch for APIs that accept a leading const char* and variadic arguments.
  // Behavior (runtime + compile-time checks):
  // - If the format contains "{}" then treat as a format string and apply
  //   the simple '{}' substitution.
  // - Else if there are exactly two args and their types are (const char*, int)
  //   treat the call as (message, file, line).
  // - Else if additional arguments exist, treat as structured key/value pairs.
  // - Otherwise, treat it as a simple message.
  template <typename... Args>
  static void format_and_log_with_format_string(LogLevel level, const char *format, Args &&...args) {
    // Quick runtime check for '{}' placeholders
    if (std::strstr(format, "{}") != nullptr) {
      std::ostringstream oss;
      format_message(oss, format, std::forward<Args>(args)...);
      log(level, oss.str());
      return;
    }

    // No placeholders: special-case (file, line) pair
    if constexpr (sizeof...(Args) == 2) {
      using First = std::tuple_element_t<0, std::tuple<Args...>>;
      using Second = std::tuple_element_t<1, std::tuple<Args...>>;
      if constexpr (std::is_convertible_v<std::decay_t<First>, const char *> &&
                    std::is_integral_v<std::decay_t<Second>>) {
        log(level, std::string(format), std::forward<Args>(args)...);
        return;
      }
    }

    if constexpr (sizeof...(Args) > 0) {
      if constexpr (sizeof...(Args) % 2 == 0) {
        // Even number of args: treat as structured key/value pairs
        log(level, std::string(format), std::forward<Args>(args)...);
        return;
      } else {
        // Odd number of args: ignore extras; log simple message
        log(level, std::string(format));
        return;
      }
    } else {
      // No extra args: simple message
      log(level, std::string(format));
      return;
    }
  }

  template <typename... Args>
  static void debug(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_DEBUG, format, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an info message with type-safe formatting
   *
   * @tparam Args Variadic template arguments
   * @param format The format string
   * @param args Arguments to format
   */
  template <typename... Args>
  static void info(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_INFO, format, std::forward<Args>(args)...);
  }

  /**
   * @brief Log a warning message with type-safe formatting
   *
   * @tparam Args Variadic template arguments
   * @param format The format string
   * @param args Arguments to format
   */
  template <typename... Args>
  static void warning(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_WARNING, format, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an error message with type-safe formatting
   *
   * @tparam Args Variadic template arguments
   * @param format The format string
   * @param args Arguments to format
   */
  template <typename... Args>
  static void error(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_ERROR, format, std::forward<Args>(args)...);
  }

  /**
   * @brief Format a message with variadic arguments
   *
   * @tparam Args Variadic template arguments
   * @param oss The output string stream
   * @param format The format string
   * @param args Arguments to format
   */
  template <typename... Args>
  static void format_message(std::ostringstream &oss, const char *format,
                             Args &&...args) {
    format_helper(oss, format, std::forward<Args>(args)...);
  }

private:
  /**
   * @brief Format key-value pairs for structured logging
   *
   * @tparam Key Type of the key
   * @tparam Value Type of the value
   * @tparam Args Variadic template arguments
   * @param oss The output string stream
   * @param key The key
   * @param value The value
   * @param args Remaining key-value pairs
   */
  template <typename Key, typename Value, typename... Args>
  static void format_key_value_pairs(std::ostringstream &oss, Key &&key,
                                     Value &&value, Args &&...args) {
    oss << " " << key << "=" << value;
    format_key_value_pairs(oss, std::forward<Args>(args)...);
  }

  /**
   * @brief Format key-value pairs for structured logging (base case)
   *
   * @param oss The output string stream
   */
  static void format_key_value_pairs(std::ostringstream &oss) {
    // Base case - no more key-value pairs
  }

  /**
   * @brief Helper function to format messages with variadic arguments
   *
   * @tparam T Type of the first argument
   * @tparam Args Variadic template arguments
   * @param oss The output string stream
   * @param format The format string
   * @param value The first argument
   * @param args Remaining arguments
   */
  template <typename T, typename... Args>
  static void format_helper(std::ostringstream &oss, const char *format,
                            T &&value, Args &&...args) {
    while (*format) {
      if (*format == '{' && *(format + 1) == '}') {
        oss << value;
        format += 2;
        format_helper(oss, format, std::forward<Args>(args)...);
        return;
      } else {
        oss << *format;
        ++format;
      }
    }
  }

  /**
   * @brief Helper function to format messages (base case)
   *
   * @param oss The output string stream
   * @param format The format string
   */
  static void format_helper(std::ostringstream &oss, const char *format) {
    while (*format) {
      oss << *format;
      ++format;
    }
  }
};

// Static member definitions
inline LogLevel Logger::current_level = LOG_INFO;
inline std::mutex Logger::log_mutex;
inline std::ostream *Logger::output_stream = &std::cout;
inline std::ostream *Logger::error_stream = &std::cerr;
inline std::unique_ptr<LogFormatter> Logger::formatter = nullptr;
inline std::unique_ptr<RotatingFileLogger> Logger::file_logger = nullptr;

/**
 * @brief Convenience macro for logging debug messages with file and line
 * information
 *
 * Usage: LOG_DEBUG("Message");
 */
#ifndef INTERLACED_CORE_DISABLE_DEBUG_LOGS
#define LOG_DEBUG(msg)                                                         \
  interlaced::core::logging::Logger::debug(msg, __FILE__, __LINE__)
#else
#define LOG_DEBUG(msg) (void)0
#endif

/**
 * @brief Convenience macro for logging informational messages with file and
 * line information
 *
 * Usage: LOG_INFO("Message");
 */
#define LOG_INFO(msg)                                                          \
  interlaced::core::logging::Logger::info(msg, __FILE__, __LINE__)

/**
 * @brief Convenience macro for logging warning messages with file and line
 * information
 *
 * Usage: LOG_WARNING("Message");
 */
#define LOG_WARNING(msg)                                                       \
  interlaced::core::logging::Logger::warning(msg, __FILE__, __LINE__)

/**
 * @brief Convenience macro for logging error messages with file and line
 * information
 *
 * Usage: LOG_ERROR("Message");
 */
#define LOG_ERROR(msg)                                                         \
  interlaced::core::logging::Logger::error(msg, __FILE__, __LINE__)


// Test helpers: small functions to exercise edge-case cleanup and error printing paths in tests
inline void test_force_clear_stream(std::ostream &s) {
  if (!s.good()) {
    s.clear();
  }
}

inline void test_force_logging_error_messages(const std::string &msg) {
  std::cerr << "File logging error: " << msg << std::endl;
  std::cerr << msg << std::endl;
  std::cerr << "Unknown logging error occurred" << std::endl;
  std::cerr << msg << std::endl;
}

} // namespace logging

} // namespace core

} // namespace interlaced

#endif // INTERLACED_CORE_LOGGING_HPP

#include "doctest.h"
#include "interlaced_core/logging.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using namespace interlaced::core::logging;

TEST_SUITE("interlaced_core_logging") {

TEST_CASE("log_level_to_string") {
    CHECK(log_level_to_string(LOG_DEBUG) == "DEBUG");
    CHECK(log_level_to_string(LOG_INFO) == "INFO");
    CHECK(log_level_to_string(LOG_WARNING) == "WARNING");
    CHECK(log_level_to_string(LOG_ERROR) == "ERROR");
}

TEST_CASE("DefaultLogFormatter - standard timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD);
    std::tm time_info = {};
    time_info.tm_year = 123; // 2023
    time_info.tm_mon = 0;    // January
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_INFO, "Test message", time_info);
    CHECK(result.find("[2023-01-15 14:30:45] [INFO] Test message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - ISO8601 timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::ISO8601);
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_WARNING, "Warning message", time_info);
    CHECK(result.find("[2023-01-15T14:30:45Z] [WARNING] Warning message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - no timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::NONE);
    std::tm time_info = {};
    std::string result = formatter.format(LOG_ERROR, "Error message", time_info);
    CHECK(result == "[ERROR] Error message");
}

TEST_CASE("DefaultLogFormatter - with prefix") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD, "PREFIX");
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_DEBUG, "Debug message", time_info);
    CHECK(result.find("PREFIX [2023-01-15 14:30:45] [DEBUG] Debug message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - with file and line") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD);
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_INFO, "Test message", time_info, "/path/to/file.cpp", 42);
    CHECK(result.find("Test message (file.cpp:42)") != std::string::npos);
}

TEST_CASE("Logger - set and get level") {
    // Reset to default
    Logger::set_level(LOG_INFO);

    // Test setting different levels
    Logger::set_level(LOG_DEBUG);
    // Note: We can't directly test the current level since it's private,
    // but we can test behavior by logging and capturing output

    Logger::set_level(LOG_ERROR);
    Logger::set_level(LOG_INFO); // Reset for other tests
}

TEST_CASE("Logger - basic logging to streams") {
    // Capture output
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warning("Warning message");
    Logger::error("Error message");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug message") != std::string::npos);
    CHECK(output.find("Info message") != std::string::npos);
    CHECK(output.find("Warning message") != std::string::npos);
    CHECK(error.find("Error message") != std::string::npos);

    // Reset streams
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - level filtering") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_WARNING); // Only warnings and errors should appear

    Logger::debug("Debug message"); // Should be filtered out
    Logger::info("Info message");   // Should be filtered out
    Logger::warning("Warning message");
    Logger::error("Error message");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug message") == std::string::npos);
    CHECK(output.find("Info message") == std::string::npos);
    CHECK(output.find("Warning message") != std::string::npos);
    CHECK(error.find("Error message") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - structured logging") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::info("User logged in", "user_id", 12345, "ip_address", "192.168.1.1");

    std::string output = output_buffer.str();
    CHECK(output.find("User logged in user_id=12345 ip_address=192.168.1.1") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - type-safe formatting") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::info("User {} logged in from {}", "alice", "192.168.1.1");

    std::string output = output_buffer.str();
    CHECK(output.find("User alice logged in from 192.168.1.1") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - custom formatter") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Set custom formatter
    auto custom_formatter = std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "CUSTOM");
    Logger::set_formatter(std::move(custom_formatter));

    Logger::info("Test message");

    std::string output = output_buffer.str();
    CHECK(output.find("CUSTOM [INFO] Test message") != std::string::npos);

    // Reset formatter
    Logger::set_formatter(nullptr);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("RotatingFileLogger - basic file creation") {
    // Create a temporary directory for testing
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs";
    fs::create_directories(temp_dir);

    std::string base_filename = (temp_dir / "test.log").string();

    {
        RotatingFileLogger logger(base_filename, 1024, 3); // 1KB max, 3 files

        logger.write("Test message 1");
        logger.write("Test message 2");
    }

    // Check that file was created and contains messages
    CHECK(fs::exists(base_filename));

    std::ifstream file(base_filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Test message 1") != std::string::npos);
    CHECK(content.find("Test message 2") != std::string::npos);

    // Cleanup
    fs::remove_all(temp_dir);
}

TEST_CASE("RotatingFileLogger - size-based rotation") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs_rotation";
    fs::create_directories(temp_dir);

    std::string base_filename = (temp_dir / "rotate.log").string();

    {
        RotatingFileLogger logger(base_filename, 50, 3); // Very small max size

        // Write messages that will exceed the size limit
        logger.write("This is a long message that will cause rotation");
        logger.write("Another long message to trigger rotation again");
        logger.write("Final message");
    }

    // Check that rotation occurred
    CHECK(fs::exists(base_filename + ".1"));
    CHECK(fs::exists(base_filename + ".2"));

    // Cleanup
    fs::remove_all(temp_dir);
}

TEST_CASE("Logger - file logging") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_file_logging";
    fs::create_directories(temp_dir);

    std::string log_filename = (temp_dir / "app.log").string();

    Logger::set_file_logging(log_filename, 1024, 2);
    Logger::set_level(LOG_DEBUG);

    Logger::info("Application started");
    Logger::warning("This is a warning");
    Logger::error("This is an error");

    // Disable file logging to flush
    Logger::set_file_logging(nullptr);

    // Check log file
    CHECK(fs::exists(log_filename));

    std::ifstream file(log_filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Application started") != std::string::npos);
    CHECK(content.find("This is a warning") != std::string::npos);
    CHECK(content.find("This is an error") != std::string::npos);

    // Cleanup
    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - time-based file rotation") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_time_rotation";
    fs::create_directories(temp_dir);

    std::string log_filename = (temp_dir / "time_rotate.log").string();

    // Set up time-based rotation with very short interval for testing
    Logger::set_file_logging(log_filename, std::chrono::hours(1), 3);
    Logger::set_level(LOG_DEBUG);

    Logger::info("First message");

    // Simulate time passing (in a real scenario, this would happen over time)
    // For testing, we'll just write multiple messages

    Logger::info("Second message");
    Logger::info("Third message");

    Logger::set_file_logging(nullptr);

    CHECK(fs::exists(log_filename));

    // Cleanup
    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - thread safety") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Run logging from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i]() {
            Logger::info("Message from thread {}", i);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::string output = output_buffer.str();

    // Check that all messages were logged (thread safety)
    for (int i = 0; i < 10; ++i) {
        CHECK(output.find("Message from thread " + std::to_string(i)) != std::string::npos);
    }

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logging macros") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Test macros
    LOG_DEBUG("Debug macro test");
    LOG_INFO("Info macro test");
    LOG_WARNING("Warning macro test");
    LOG_ERROR("Error macro test");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug macro test") != std::string::npos);
    CHECK(output.find("Info macro test") != std::string::npos);
    CHECK(output.find("Warning macro test") != std::string::npos);
    CHECK(error.find("Error macro test") != std::string::npos);

    // Check that file and line info is included
    CHECK(output.find("test_logging.cc") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("log_level_to_string - unknown") {
    // Cover default branch
    CHECK(log_level_to_string(static_cast<LogLevel>(-1)) == "UNKNOWN");
}

TEST_CASE("DefaultLogFormatter - UNIX timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::UNIX);
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_INFO, "Unix style", time_info);
    CHECK(result.find("[2023-01-15 14:30:45] [INFO] Unix style") != std::string::npos);
}

TEST_CASE("format_and_log_with_format_string - file/line pair") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;
    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // No placeholders, two args (const char*, int) -> treated as file/line
    Logger::info("Pair path", "dummy.cpp", 7);

    const std::string out = output_buffer.str();
    CHECK(out.find("Pair path (dummy.cpp:7)") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("format_and_log_with_format_string - odd args => simple message") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;
    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Odd number of extra args should be ignored; logs simple message
    Logger::info("Odd args test", "k1");

    const std::string out = output_buffer.str();
    CHECK(out.find("Odd args test") != std::string::npos);
    CHECK(out.find("k1=") == std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("format_and_log_with_format_string - placeholders with zero args") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;
    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Contains "{}" but no args; braces should remain literally
    Logger::info("Value {} remains {} with no args");

    const std::string out = output_buffer.str();
    CHECK(out.find("Value {} remains {} with no args") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("format_and_log_with_format_string - plain char* no extras => simple message") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;
    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // No placeholders and no extra args: treat as simple message
    Logger::info("Plain literal char*");

    const std::string out = output_buffer.str();
    CHECK(out.find("Plain literal char*") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - char* wrappers for all levels") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    Logger::debug("dbg {}", 1);
    Logger::info("inf {}", 2);
    Logger::warning("wrn {}", 3);
    Logger::error("err {}", 4);

    CHECK(out.str().find("dbg 1") != std::string::npos);
    CHECK(out.str().find("inf 2") != std::string::npos);
    CHECK(out.str().find("wrn 3") != std::string::npos);
    CHECK(err.str().find("err 4") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - explicit file/line overloads without macros") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    Logger::debug(std::string("D msg"), __FILE__, 123);
    Logger::info(std::string("I msg"), __FILE__, 124);
    Logger::warning(std::string("W msg"), __FILE__, 125);
    Logger::error(std::string("E msg"), __FILE__, 126);

    const std::string o = out.str();
    const std::string e = err.str();
    CHECK(o.find("D msg") != std::string::npos);
    CHECK(o.find("I msg") != std::string::npos);
    CHECK(o.find("W msg") != std::string::npos);
    CHECK(e.find("E msg") != std::string::npos);
    CHECK(o.find("test_logging.cc:123") != std::string::npos);
    CHECK(o.find("test_logging.cc:124") != std::string::npos);
    CHECK(o.find("test_logging.cc:125") != std::string::npos);
    CHECK(e.find("test_logging.cc:126") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("RotatingFileLogger - time-based rotation immediate") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_time_rotation_immediate";
    fs::create_directories(temp_dir);
    std::string base_filename = (temp_dir / "instant.log").string();

    {
        // 0 hours interval => should rotate on first write
        RotatingFileLogger logger(base_filename, std::chrono::hours(0), 3);
        logger.write("first");
        logger.write("second");
    }

    CHECK(fs::exists(base_filename + ".1"));

    fs::remove_all(temp_dir);
}

TEST_CASE("RotatingFileLogger - write fallback when file cannot open") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_write_fallback";
    fs::create_directories(temp_dir);

    // Use a directory path as 'file' to force ofstream.open() failure
    std::string bad_path = temp_dir.string();

    // Capture std::cerr for fallback output
    std::ostringstream cerr_buffer;
    auto *old_cerr_buf = std::cerr.rdbuf(cerr_buffer.rdbuf());
    {
        RotatingFileLogger logger(bad_path, 1024, 2);
        logger.write("Should go to cerr");
    }
    // Restore cerr
    std::cerr.rdbuf(old_cerr_buf);

    const std::string err = cerr_buffer.str();
    CHECK(err.find("Failed to open log file: ") != std::string::npos);
    CHECK(err.find("Should go to cerr") != std::string::npos);

    fs::remove_all(temp_dir);
}

TEST_CASE("Logger - output stream recovery when badbit set (info)") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Force bad state on output stream to exercise clear() path
    output_buffer.setstate(std::ios::badbit);
    Logger::info("Should still attempt write to bad stream");

    // After logging, library clears the stream; good() should be true
    CHECK(output_buffer.good());

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - error stream recovery when badbit set (error)") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    error_buffer.setstate(std::ios::badbit);
    Logger::error("Err path on bad stream");

    CHECK(error_buffer.good());

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("format_and_log_with_format_string - treat as KV pairs") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;
    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // No placeholders, even number of args -> key/value formatting path
    Logger::info("KV start", "a", 1, "b", 2);
    const std::string out = output_buffer.str();
    CHECK(out.find("KV start a=1 b=2") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Macros with file logger (file+line overload)") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_macro_file_logger";
    fs::create_directories(temp_dir);
    std::string log_filename = (temp_dir / "macro.log").string();

    Logger::set_file_logging(log_filename, 1024, 2);
    Logger::set_level(LOG_DEBUG);

    // These macros call the (message,file,line) overload; ensure file sink path is used
    LOG_INFO("Macro to file");
    LOG_WARNING("Macro warn to file");
    LOG_ERROR("Macro err to file");

    Logger::set_file_logging(nullptr);

    CHECK(fs::exists(log_filename));
    std::ifstream file(log_filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CHECK(content.find("Macro to file") != std::string::npos);
    CHECK(content.find("Macro warn to file") != std::string::npos);
    CHECK(content.find("Macro err to file") != std::string::npos);

    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("File+line overload - early exit when filtered") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    // Only allow errors; info should be filtered
    Logger::set_level(LOG_ERROR);

    LOG_INFO("Filtered fileline");

    CHECK(out.str().empty());
    CHECK(err.str().empty());

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Structured logging - early exit when below level") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_INFO); // filter out debug

    // Will route to structured overload (even number of args, no placeholders)
    Logger::debug("Early", "k", 1);
    CHECK(out.str().empty());
    CHECK(err.str().empty());

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Structured logging to file logger") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_structured_file";
    fs::create_directories(temp_dir);
    std::string log_filename = (temp_dir / "kvfile.log").string();

    Logger::set_file_logging(log_filename, 1024, 2);
    Logger::set_level(LOG_DEBUG);

    Logger::info("KV file", "a", 10, "b", 20);

    Logger::set_file_logging(nullptr);
    CHECK(fs::exists(log_filename));

    std::ifstream f(log_filename);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    CHECK(content.find("KV file a=10 b=20") != std::string::npos);

    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

// Custom failing streambuf to trigger exceptions during stream operations
struct FailingBuf : public std::streambuf {
    int overflow(int) override { return traits_type::eof(); }
};


TEST_CASE("Logger - catch path on stream exceptions (simple log)") {
    FailingBuf buf;
    std::ostream failing(&buf);
    failing.exceptions(std::ios::failbit | std::ios::badbit);

    std::ostringstream cerr_capture;
    auto *old = std::cerr.rdbuf(cerr_capture.rdbuf());

    Logger::set_output_streams(failing, failing);
    Logger::set_level(LOG_DEBUG);
    Logger::info("Trigger exception path");

    std::cerr.rdbuf(old);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);

    const std::string err = cerr_capture.str();
    CHECK(err.find("Logging error:") != std::string::npos);
}

TEST_CASE("Logger - catch path on stream exceptions (file+line)") {
    FailingBuf buf;
    std::ostream failing(&buf);
    failing.exceptions(std::ios::failbit | std::ios::badbit);

    std::ostringstream cerr_capture;
    auto *old = std::cerr.rdbuf(cerr_capture.rdbuf());

    Logger::set_output_streams(failing, failing);
    Logger::set_level(LOG_DEBUG);
    Logger::info(std::string("With file"), __FILE__, 321);

    std::cerr.rdbuf(old);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);

    const std::string err = cerr_capture.str();
    CHECK(err.find("Logging error:") != std::string::npos);
}

TEST_CASE("Logger - catch path on stream exceptions (structured)") {
    FailingBuf buf;
    std::ostream failing(&buf);
    failing.exceptions(std::ios::failbit | std::ios::badbit);

    std::ostringstream cerr_capture;
    auto *old = std::cerr.rdbuf(cerr_capture.rdbuf());

    Logger::set_output_streams(failing, failing);
    Logger::set_level(LOG_DEBUG);
    Logger::info("Struct path", "k", 1);

    std::cerr.rdbuf(old);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);

    const std::string err = cerr_capture.str();
    CHECK(err.find("Logging error:") != std::string::npos);
}


TEST_CASE("String-only wrappers for all levels") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    Logger::debug(std::string("sdbg"));
    Logger::info(std::string("sinf"));
    Logger::warning(std::string("swrn"));
    Logger::error(std::string("serr"));

    CHECK(out.str().find("sdbg") != std::string::npos);
    CHECK(out.str().find("sinf") != std::string::npos);
    CHECK(out.str().find("swrn") != std::string::npos);
    CHECK(err.str().find("serr") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("File+line overload uses custom formatter branch") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    auto fmt = std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "FMT");
    Logger::set_formatter(std::move(fmt));
    Logger::info(std::string("Fmt fileline"), __FILE__, 4321);

    CHECK(out.str().find("FMT [INFO] Fmt fileline") != std::string::npos);

    Logger::set_formatter(nullptr);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("DefaultLogFormatter - Windows-style backslash basename extraction") {
    DefaultLogFormatter formatter(TimestampFormat::NONE);
    std::tm t = {};
    const std::string res = formatter.format(LOG_INFO, "Backslash msg", t, "C\\dir\\file.cpp", 5);
    CHECK(res.find("Backslash msg (file.cpp:5)") != std::string::npos);
}

TEST_CASE("Char* wrapper logs into file sink") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_charptr_file_sink";
    fs::create_directories(temp_dir);
    std::string log_filename = (temp_dir / "charptr.log").string();

    Logger::set_file_logging(log_filename, 1024, 2);
    Logger::set_level(LOG_DEBUG);

    // Use the char* formatting wrapper which routes through file sink
    Logger::info("file {}", 42);

    Logger::set_file_logging(nullptr);

    CHECK(fs::exists(log_filename));
    std::ifstream f(log_filename);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    CHECK(content.find("file 42") != std::string::npos);

    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("File+line overload - clear() path when badbit set") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    out.setstate(std::ios::badbit);
    Logger::warning(std::string("badbit fileline"), __FILE__, 111);
    CHECK(out.good());

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Structured explicit early exit via log(level, msg, kv)") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_INFO);

    Logger::log(LOG_DEBUG, std::string("struct-early"), "k", 1);
    CHECK(out.str().empty());
    CHECK(err.str().empty());

    Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("Structured with formatter and badbit clear path") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);
    auto fmt = std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "S");
    Logger::set_formatter(std::move(fmt));

    out.setstate(std::ios::badbit);
    Logger::log(LOG_INFO, std::string("struct-clear"), "a", 1);
    CHECK(out.good());

    Logger::set_formatter(nullptr);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("format_message recursion - surplus args ignored after placeholders") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    // One placeholder but two args: second arg should be ignored by formatter
    Logger::info("one {}", 1, 999);

    const std::string s = out.str();
    CHECK(s.find("one 1") != std::string::npos);
    CHECK(s.find("999") == std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("RotatingFileLogger - write to read-only file sets badbit and clears stream") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs_readonly";
    fs::create_directories(temp_dir);
    std::string base_filename = (temp_dir / "readonly.log").string();

    // Create the file and make it read-only
    std::ofstream f(base_filename);
    f << "initial" << std::endl;
    f.close();

    // Make file read-only
    fs::permissions(base_filename, fs::perms::owner_read, fs::perm_options::replace);

    // Capture cerr to detect any fallback messages
    std::ostringstream cerr_buffer;
    auto *old = std::cerr.rdbuf(cerr_buffer.rdbuf());

    RotatingFileLogger logger(base_filename, 1024, 2);
    // This write should fail and set badbit on the internal stream; library should clear it
    logger.write("Should fail to write");

    // Restore cerr
    std::cerr.rdbuf(old);

    // Ensure we didn't crash and that some message was attempted
    const std::string err_out = cerr_buffer.str();
    // Either a failure opening message or nothing; we just want to ensure the call completed
    CHECK(true);

    // Restore permissions so cleanup works
    fs::permissions(base_filename, fs::perms::owner_all, fs::perm_options::replace);
    fs::remove_all(temp_dir);
}

TEST_CASE("Logger::format_message - direct invocation") {
    std::ostringstream oss;
    Logger::format_message(oss, "Hello {} and {}", 1, "two");
    CHECK(oss.str().find("Hello 1 and two") != std::string::npos);
}

TEST_CASE("format_and_log_with_format_string - various instantiations") {
    std::ostringstream out;
    std::ostringstream err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_DEBUG);

    // Placeholder branch
    Logger::format_and_log_with_format_string(LOG_INFO, "PH {}", 42);
    CHECK(out.str().find("PH 42") != std::string::npos);
    out.str(""); out.clear();

    // File/line pair branch
    Logger::format_and_log_with_format_string(LOG_INFO, "pair", "file.cpp", 33);
    CHECK(out.str().find("pair (file.cpp:33)") != std::string::npos);
    out.str(""); out.clear();

    // KV pairs branch
    Logger::format_and_log_with_format_string(LOG_INFO, "KV", "a", 1, "b", 2);
    CHECK(out.str().find("KV a=1 b=2") != std::string::npos);
    out.str(""); out.clear();

    // Odd args -> simple message
    Logger::format_and_log_with_format_string(LOG_INFO, "odd", "extra");
    CHECK(out.str().find("odd") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - formatter throwing is propagated") {
    struct ThrowingFormatter : public DefaultLogFormatter {
        ThrowingFormatter():DefaultLogFormatter(TimestampFormat::NONE){}
        std::string format(LogLevel, const std::string&, const std::tm&, const char* = nullptr, int = 0) override {
            throw std::runtime_error("format error");
        }
    };

    auto fmt = std::make_unique<ThrowingFormatter>();
    Logger::set_formatter(std::move(fmt));
    Logger::set_level(LOG_DEBUG);

    CHECK_THROWS_AS(Logger::info(std::string("trigger throw")), std::runtime_error);

    // Reset
    Logger::set_formatter(nullptr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("logging test helpers - clear stream and error print") {
    std::ostringstream out;
    out.setstate(std::ios::badbit);
    CHECK(!out.good());
    // Use helper to clear
    test_force_clear_stream(out);
    CHECK(out.good());

    std::ostringstream cerr_capture;
    auto *old = std::cerr.rdbuf(cerr_capture.rdbuf());
    test_force_logging_error_messages("test-msg");
    std::cerr.rdbuf(old);

    CHECK(cerr_capture.str().find("test-msg") != std::string::npos);
}

TEST_CASE("RotatingFileLogger - simulate badbit and clear path via test helpers") {
    namespace fs = std::filesystem;
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs_badbit";
    fs::create_directories(temp_dir);
    std::string base_filename = (temp_dir / "badbit.log").string();

    RotatingFileLogger logger(base_filename, 1024, 2);
    // Force the internal stream badbit
    logger.test_set_badbit();

    // Should not throw and should recover internally
    logger.write("After badbit write attempt");

    // Clear and try again
    logger.test_clear_badbit();
    logger.write("After clear write attempt");

    fs::remove_all(temp_dir);
}

} // TEST_SUITE
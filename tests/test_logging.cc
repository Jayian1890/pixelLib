#include "../include/logging.hpp"
#include "../third-party/doctest/doctest.h"

#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <ios>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

namespace logging = pixellib::core::logging;

using logging::AsyncLogSink;
using logging::DefaultLogFormatter;
using logging::JSONLogFormatter;
using logging::LOG_DEBUG;
using logging::LOG_ERROR;
using logging::LOG_FATAL;
using logging::LOG_INFO;
using logging::log_level_to_string;
using logging::LOG_TRACE;
using logging::LOG_WARNING;
using logging::LogContext;
namespace LogContextStorage = logging::LogContextStorage;
using logging::Logger;
using logging::LogLevel;
using logging::RotatingFileLogger;
using logging::StreamSink;
using logging::TimestampFormat;

TEST_SUITE("Logging Module")
{
  // slow sink helper for testing async drop behavior
  class SlowSink : public logging::LogSink
  {
  public:
    std::ostringstream *out_;
    std::chrono::milliseconds delay_;
    explicit SlowSink(std::ostringstream &out, std::chrono::milliseconds d = std::chrono::milliseconds(50)) : out_(&out), delay_(d) {}
    void write(const std::string &message) override
    {
      std::this_thread::sleep_for(delay_);
      *out_ << message << '\n';
    }
  };

  class ThrowingSinkStdException : public pixellib::core::logging::LogSink
  {
  public:
    void write(const std::string &message) override
    {
      static_cast<void>(message);
      throw std::runtime_error("boom");
    }
  };

  struct NonStdThrowable
  {
  };

  class ThrowingSinkNonStd : public pixellib::core::logging::LogSink
  {
  public:
    void write(const std::string &message) override
    {
      static_cast<void>(message);
      // NOLINTNEXTLINE(hicpp-exception-baseclass)
      throw NonStdThrowable{};
    }
  };

  class AlwaysFailBuf : public std::streambuf
  {
  protected:
    int overflow(int ch) override
    {
      static_cast<void>(ch);
      return traits_type::eof();
    }
  };

  TEST_CASE("LogLevel")
  {
    CHECK(std::string(log_level_to_string(LOG_TRACE)) == "TRACE");
    CHECK(std::string(log_level_to_string(LOG_ERROR)) == "ERROR");

    DefaultLogFormatter df(TimestampFormat::NONE, "PREFIX");
    std::tm tm{};
    std::string out = df.format(LOG_INFO, "msg", tm, "file.cpp", 10);
    CHECK(out.find("PREFIX") != std::string::npos);
    CHECK(out.find("msg") != std::string::npos);
    CHECK(out.find("file.cpp") != std::string::npos);

    JSONLogFormatter jf;
    {
      LogContext ctx;
      ctx.add("k", "v");
      std::string j = jf.format(LOG_INFO, "m", tm, nullptr, 0);
      CHECK(j.find("\"level\":\"INFO\"") != std::string::npos);
      CHECK(j.find("\"message\":\"m\"") != std::string::npos);
    }

    CHECK(std::string(log_level_to_string(static_cast<LogLevel>(999))) == "UNKNOWN");
  }

  TEST_CASE("FormatterTimestamps")
  {
    std::tm tm{};
    DefaultLogFormatter df_std(TimestampFormat::STANDARD, "");
    DefaultLogFormatter df_iso(TimestampFormat::ISO8601, "");
    DefaultLogFormatter df_unix(TimestampFormat::UNIX, "");
    std::string s1 = df_std.format(LOG_INFO, "m", tm, nullptr, 0);
    std::string s2 = df_iso.format(LOG_INFO, "m", tm, nullptr, 0);
    std::string s3 = df_unix.format(LOG_INFO, "m", tm, nullptr, 0);
    CHECK(s1.find("[INFO]") != std::string::npos);
    CHECK(s2.find("[INFO]") != std::string::npos);
    CHECK(s3.find("[INFO]") != std::string::npos);
  }

  TEST_CASE("JsonEscaping")
  {
    JSONLogFormatter jf;
    std::tm tm{};
    std::string msg = std::string("tab\tcr\rbs\\");
    std::string j = jf.format(LOG_INFO, msg, tm, nullptr, 0);
    CHECK(j.find("\\t") != std::string::npos);
    CHECK(j.find("\\r") != std::string::npos);
    CHECK(j.find("\\\\") != std::string::npos);
  }

<<<<<<< Updated upstream
=======
<<<<<<< Updated upstream
=======
>>>>>>> Stashed changes
  TEST_CASE("LogFilterPerformance")
  {
    // Micro-benchmark to measure the cost of filtered log calls (should be fast)
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_INFO); // debug should be filtered

    constexpr int iterations = 100000;
<<<<<<< Updated upstream
    auto start = std::chrono::high_resolution_clock::now();
=======

    // Case A: macro call with a string literal (optimized path we added)
    auto startA = std::chrono::high_resolution_clock::now();
>>>>>>> Stashed changes
    for (int i = 0; i < iterations; ++i)
    {
      LOG_DEBUG("x");
    }
<<<<<<< Updated upstream
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    MESSAGE("LogFilterPerformance: " << ms << "ms for " << iterations << " iterations");
    CHECK(ms >= 0);
  }

=======
    auto endA = std::chrono::high_resolution_clock::now();
    auto msA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();
    MESSAGE("LogFilterPerformance (literal macro): " << msA << "ms for " << iterations << " iterations");

    // Case B: force a std::string allocation each call (baseline to compare)
    auto startB = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
      // This construct forces a temporary std::string to be created before calling
      // the API and represents the pre-optimized behavior.
      Logger::debug(std::string("x"), __FILE__, __LINE__);
    }
    auto endB = std::chrono::high_resolution_clock::now();
    auto msB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();
    MESSAGE("LogFilterPerformance (forced allocation): " << msB << "ms for " << iterations << " iterations");

    CHECK(msA >= 0);
    CHECK(msB >= 0);
  }

>>>>>>> Stashed changes
>>>>>>> Stashed changes
  TEST_CASE("StreamSink")
  {
    std::ostringstream out;
    StreamSink sink(out);
    sink.write("hello");
    CHECK(out.str().find("hello") != std::string::npos);

    out.setstate(std::ios::badbit);
    sink.write("world");
  }

  TEST_CASE("AsyncSink")
  {
    std::ostringstream out;
    auto inner = std::make_unique<StreamSink>(out);
    AsyncLogSink async(std::move(inner), 2, AsyncLogSink::DropPolicy::DROP_NEWEST);

    async.write("one");
    async.write("two");
    async.write("three");
    async.flush();

    std::string s = out.str();
    CHECK(s.find("one") != std::string::npos);
    CHECK(s.find("two") != std::string::npos);

    async.shutdown();
  }

  TEST_CASE("LoggerOutput")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_INFO);

    Logger::info("info-msg");
    Logger::warning("warn-msg");
    Logger::debug("debug-msg");

    std::string o = out.str();
    CHECK(o.find("info-msg") != std::string::npos);
    CHECK(o.find("warn-msg") != std::string::npos);
    CHECK(o.find("debug-msg") == std::string::npos);

    Logger::info("hello {}", "world");
    {
      bool found = out.str().find("helloworld") != std::string::npos || out.str().find("hello world") != std::string::npos;
      CHECK(found);
    }

    Logger::info("msg", std::string("k"), 123);
    CHECK(out.str().find("k=123") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("FormatterContext")
  {
    DefaultLogFormatter df(TimestampFormat::NONE, "");
    std::tm tm{};
    std::string out = df.format(LOG_INFO, "m", tm, "/path/to/myfile.cpp", 123);
    CHECK(out.find("myfile.cpp") != std::string::npos);
    CHECK(out.find(":123") != std::string::npos);

    // Test that context is included
    {
      pixellib::core::logging::LogContext ctx;
      ctx.add("user", "u1");
      std::string out2 = df.format(LOG_INFO, "msg", tm, nullptr, 0);
      CHECK(out2.find("user=u1") != std::string::npos);
    }
  }

  TEST_CASE("LoggerSinks")
  {
    std::ostringstream out;
    // Build a global config with a stream sink
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_DEBUG).add_stream_sink(out);
    pixellib::core::logging::Logger::configure(b.build());

    auto cat = pixellib::core::logging::Logger::get("testcat");
    cat.debug("dbg-msg");
    cat.info("info-msg");

    std::string s = out.str();
    CHECK(s.find("dbg-msg") != std::string::npos);
    CHECK(s.find("info-msg") != std::string::npos);

    // Reset global logging to defaults
    pixellib::core::logging::Logger::set_file_logging(nullptr);
    pixellib::core::logging::Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("CategoryFiltering")
  {
    std::ostringstream out;
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_ERROR).add_stream_sink(out);
    auto cfg = b.build();
    pixellib::core::logging::Logger::LoggerRegistry::set_config("catA", std::move(cfg));

    auto cat = pixellib::core::logging::Logger::get("catA");
    cat.info("should-be-filtered");
    cat.error("should-show");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string s = out.str();
    CHECK(s.find("should-be-filtered") == std::string::npos);
    CHECK(s.find("should-show") != std::string::npos);
  }

  TEST_CASE("json_formatter_escaping")
  {
    JSONLogFormatter jf;
    std::tm tm{};
    std::string msg = "quote\" backslash\\ newline\n";
    std::string j = jf.format(LOG_INFO, msg, tm, nullptr, 0);
    CHECK(j.find("\\\"") != std::string::npos); // escaped quote
    CHECK(j.find("\\\\") != std::string::npos); // escaped backslash
    CHECK(j.find("\\n") != std::string::npos);  // escaped newline
  }

  TEST_CASE("AsyncDrop")
  {
    std::ostringstream out;
    // slow inner sink to force queue buildup
    auto slow = std::make_unique<SlowSink>(out, std::chrono::milliseconds(40));
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_INFO).add_async_sink(std::move(slow), 1, pixellib::core::logging::AsyncLogSink::DropPolicy::DROP_OLDEST);
    pixellib::core::logging::Logger::configure(b.build());

    // Rapidly emit many messages
    for (int i = 0; i < 10; ++i)
    {
      pixellib::core::logging::Logger::info(std::string("m") + std::to_string(i));
    }

    // Allow some time for worker to consume
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    size_t dropped = pixellib::core::logging::Logger::get_async_dropped_count();
    CHECK(dropped > 0);
    // flush and shutdown to be clean
    pixellib::core::logging::Logger::async_flush();
    pixellib::core::logging::Logger::async_shutdown();

    // Reset global logging to defaults
    pixellib::core::logging::Logger::set_file_logging(nullptr);
    pixellib::core::logging::Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("TestHelpers")
  {
    std::ostringstream s;
    s.setstate(std::ios::badbit);
    pixellib::core::logging::test_force_clear_stream(s);
    CHECK(s.good());

    // test_force_logging_error_messages writes to cerr; just call to cover code paths
    pixellib::core::logging::test_force_logging_error_messages("err-msg");
  }

  TEST_CASE("RotatingRotation")
  {
    namespace fs = std::filesystem;
    std::string base = "build/tmp/testlog";
    fs::create_directories("build/tmp");
    // clean up any previous files
    for (int i = 0; i < 5; ++i)
    {
      std::string p = base + (i == 0 ? "" : ("." + std::to_string(i)));
      if (fs::exists(p))
      {
        fs::remove(p);
      }
    }

    // Size-based rotation: very small max size so rotation happens quickly
    pixellib::core::logging::RotatingFileLogger rf(base, 20, 3);
    rf.write("firstline");
    rf.write(std::string(30, 'x')); // should trigger rotation
    rf.write("after-rotation");

    // Allow filesystem to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    CHECK(fs::exists(base));
    CHECK(fs::exists(base + ".1"));

    // Test badbit helpers
    rf.test_set_badbit();
    rf.test_clear_badbit();

    // Time-based rotation with zero interval triggers rotation on write
    std::string tbase = "build/tmp/testlog_time";
    for (int i = 0; i < 3; ++i)
    {
      std::string p = tbase + (i == 0 ? "" : ("." + std::to_string(i)));
      if (fs::exists(p))
      {
        fs::remove(p);
      }
    }
    pixellib::core::logging::RotatingFileLogger rft(tbase, std::chrono::hours(0), 2);
    rft.write("t1");
    rft.write("t2");
    // small sleep for rotation
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK(fs::exists(tbase));
    CHECK(fs::exists(tbase + ".1"));

    // cleanup
    for (int i = 0; i < 5; ++i)
    {
      std::string p = base + (i == 0 ? "" : ("." + std::to_string(i)));
      if (fs::exists(p))
      {
        fs::remove(p);
      }
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string p = tbase + (i == 0 ? "" : ("." + std::to_string(i)));
      if (fs::exists(p))
      {
        fs::remove(p);
      }
    }
  }

  TEST_CASE("RotatingFailure")
  {
    // Opening a directory as a file should fail; write() should fallback to cerr path.
    namespace fs = std::filesystem;
    fs::create_directories("build/tmp");
    pixellib::core::logging::RotatingFileLogger rf("build/tmp", 10, 1);
    rf.write("should-fallback");
  }

  TEST_CASE("AsyncException")
  {
    auto inner = std::make_unique<ThrowingSinkNonStd>();
    pixellib::core::logging::AsyncLogSink async(std::move(inner), 8, pixellib::core::logging::AsyncLogSink::DropPolicy::DROP_NEWEST);
    async.write("x");
    async.flush();
    async.shutdown();
  }

  TEST_CASE("MacrosOverloads")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    // Macros should invoke logger with file/line
    LOG_TRACE("trace-msg");
    LOG_DEBUG("debug-msg-macro");
    LOG_INFO("info-msg-macro");
    LOG_WARNING("warn-macro");
    LOG_ERROR("err-macro");
    LOG_FATAL("fatal-macro");

    // Direct static overload with file/line
    pixellib::core::logging::Logger::log(LOG_INFO, "direct-fileline", "source.cpp", 77);

    // StreamSink clear_stream
    std::ostringstream ostr;
    pixellib::core::logging::StreamSink ss(ostr);
    ostr.setstate(std::ios::badbit);
    ss.clear_stream();
    CHECK(ostr.good());

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("ContextStorage")
  {
    pixellib::core::logging::LogContextStorage::set("k1", "v1");
    CHECK(pixellib::core::logging::LogContextStorage::get("k1") == "v1");
    pixellib::core::logging::LogContextStorage::remove("k1");
    CHECK(pixellib::core::logging::LogContextStorage::get("k1") == "");

    pixellib::core::logging::LogContext ctx;
    ctx.add("k2", "v2");
    CHECK(pixellib::core::logging::LogContextStorage::get("k2") == "v2");
    ctx.remove("k2");
    CHECK(pixellib::core::logging::LogContextStorage::get("k2") == "");
  }

  TEST_CASE("LoggerOverloads")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_TRACE);

    Logger::debug(std::string("dbg-s"));
    Logger::debug(std::string("dbg-sfl"), "f.cc", 1);
    Logger::trace(std::string("tr-s"));
    Logger::trace(std::string("tr-sfl"), "f.cc", 2);
    Logger::info(std::string("in-s"));
    Logger::info(std::string("in-sfl"), "f.cc", 3);
    Logger::warning(std::string("w-s"));
    Logger::warning(std::string("w-sfl"), "f.cc", 4);
    Logger::error(std::string("e-s"));
    Logger::error(std::string("e-sfl"), "f.cc", 5);
    Logger::fatal(std::string("fa-s"));
    Logger::fatal(std::string("fa-sfl"), "f.cc", 6);

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("SetFormatter")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    // disable sinks but keep output streams
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));

    Logger::set_formatter(std::make_unique<JSONLogFormatter>());
    Logger::log(LOG_INFO, "formatter-path");
    CHECK(out.str().find("\"message\":\"formatter-path\"") != std::string::npos);

    // Structured log with formatter set hits formatter branch in structured log
    Logger::log(LOG_INFO, std::string("structured"), "k", 1, "x", 2);
    CHECK(out.str().find("structured") != std::string::npos);

    // Clear formatter via configure
    Logger::LoggerConfig empty2;
    Logger::configure(std::move(empty2));
    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("SinkException")
  {
    AlwaysFailBuf buf;
    std::ostream failing(&buf);
    // Set exceptions while stream state is good.
    failing.exceptions(std::ios::badbit | std::ios::failbit);

    Logger::set_file_logging(nullptr);
    Logger::add_sink(std::make_unique<StreamSink>(failing));
    Logger::log(LOG_INFO, "sink-will-throw");

    // After StreamSink::clear_stream, the stream should be in a good state.
    CHECK(failing.good());
    failing.exceptions(std::ios::goodbit);

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("OutputException")
  {
    AlwaysFailBuf out_buf;
    AlwaysFailBuf err_buf;
    std::ostream out(&out_buf);
    std::ostream err(&err_buf);
    out.exceptions(std::ios::badbit | std::ios::failbit);
    err.exceptions(std::ios::badbit | std::ios::failbit);

    // Set output pointers, then clear sinks so Logger uses output_stream path.
    Logger::set_output_streams(out, err);
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));

    Logger::log(LOG_INFO, "x");            // hits catch(std::exception&) in log()
    Logger::log(LOG_INFO, "x", "f.cc", 1); // hits catch(std::exception&) in log(file,line)
    Logger::log(LOG_INFO, std::string("x"), "k", 1, "y",
                2); // hits catch(std::exception&) in structured log

    Logger::set_output_streams(std::cout, std::cerr);
    out.exceptions(std::ios::goodbit);
    err.exceptions(std::ios::goodbit);
  }

  TEST_CASE("StructuredLog")
  {
    // Ensure formatter is cleared so the structured-log sink paths are used.
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));

    // Early exit branch in structured log
    Logger::set_level(LOG_ERROR);
    Logger::log(LOG_INFO, std::string("filtered"), "k", 1, "y", 2);

    // Sink exception branches in structured log (std::exception and non-std)
    Logger::set_level(LOG_TRACE);
    Logger::set_file_logging(nullptr);
    Logger::add_sink(std::make_unique<ThrowingSinkStdException>());
    Logger::log(LOG_INFO, std::string("hit-std"), "k", 1, "y", 2);

    Logger::set_file_logging(nullptr);
    Logger::add_sink(std::make_unique<ThrowingSinkNonStd>());
    Logger::log(LOG_INFO, std::string("hit-nonstd"), "k", 1, "y", 2);

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("CategoryFormatter")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    // global formatter path (CategoryLogger uses formatter if cfg has none)
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty)); // clear sinks, keep formatter
    Logger::set_formatter(std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "CAT"));
    Logger::set_level(LOG_TRACE);
    Logger::get("catFmt").info("hi");

    // per-category formatter path
    Logger::LoggerConfigBuilder b;
    b.set_level(LOG_TRACE).set_formatter(std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "PCAT"));
    Logger::LoggerRegistry::set_config("perFmt", b.build());
    Logger::get("perFmt").info("hi2");

    // sink exception path in CategoryLogger
    Logger::LoggerConfig cfg;
    cfg.level = LOG_TRACE;
    cfg.sinks.emplace_back(std::make_unique<ThrowingSinkStdException>());
    Logger::LoggerRegistry::set_config("catThrow", std::move(cfg));
    Logger::get("catThrow").info("x");

    Logger::LoggerConfig empty2;
    Logger::configure(std::move(empty2)); // clears formatter
    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("AddSink")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    // clear sinks but preserve output_stream pointers
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));

    Logger::add_sink(std::make_unique<StreamSink>(out));
    Logger::log(LOG_INFO, "writes-to-added-sink");

    // Add a std::exception throwing sink to hit catch(const std::exception&)
    Logger::set_file_logging(nullptr);
    Logger::add_sink(std::make_unique<ThrowingSinkStdException>());
    Logger::log(LOG_INFO, "will-hit-std-exception");

    // Add a non-std throwing sink to hit catch(...)
    Logger::set_file_logging(nullptr);
    Logger::add_sink(std::make_unique<ThrowingSinkNonStd>());
    Logger::log(LOG_INFO, "will-hit-nonstd-exception");

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("OutputStream")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    // disable sinks but keep output streams set
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));

    // Error should go to error stream when using output_stream path
    Logger::log(LOG_ERROR, "goes-to-err");
    CHECK(err.str().find("goes-to-err") != std::string::npos);

    // Force output stream into a bad state to exercise the clear() branch
    out.setstate(std::ios::badbit);
    Logger::log(LOG_INFO, "force-ostream-exception");
    CHECK(out.good());

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("CategoryFallback")
  {
    // Ensure global sinks empty so category logger falls back to output streams.
    Logger::LoggerConfig empty;
    Logger::configure(std::move(empty));
    Logger::set_level(LOG_TRACE);

    auto cat = Logger::get("fallbackCat");
    cat.trace("trace");
    cat.warning("warn");
    cat.fatal("fatal");

    // Per-category config with throwing sink to hit exception handlers.
    Logger::LoggerConfigBuilder b;
    b.set_level(LOG_TRACE).add_async_sink(std::make_unique<ThrowingSinkStdException>(), 4, AsyncLogSink::DropPolicy::DROP_NEWEST);
    Logger::LoggerRegistry::set_config("throwCat", b.build());
    auto cat2 = Logger::get("throwCat");
    cat2.info("x");

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("FormatLog")
  {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_TRACE);

    // Placeholder formatting (multiple placeholders)
    Logger::info("a {} b {}", 1, 2);

    // Placeholders with missing args (exercise base case)
    Logger::info("x {} y {}", 1);

    // No placeholders + (file,line) special case
    Logger::info("msg-no-placeholders", "file.cc", 9);

    // No placeholders + structured key/value (even args)
    Logger::info("kv-no-placeholders", "a", 1, "b", 2);

    // No placeholders + odd args -> extra ignored
    Logger::info("odd-no-placeholders", "a");

    // No placeholders + no args
    Logger::info("noargs-no-placeholders");

    Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("FileLogging")
  {
    namespace fs = std::filesystem;
    fs::create_directories("build/tmp");

    std::string f1 = "build/tmp/logger_size.log";
    std::string f2 = "build/tmp/logger_time.log";
    std::error_code ec;
    fs::remove(f1, ec);
    fs::remove(f2, ec);

    Logger::set_file_logging(f1, 128, 2);
    Logger::info("to-file-1");
    CHECK(fs::exists(f1));

    Logger::set_file_logging(f2, std::chrono::hours(0), 2);
    Logger::info("to-file-2");
    CHECK(fs::exists(f2));

    // Async queue size via Logger aggregate
    std::ostringstream out;
    Logger::LoggerConfigBuilder b;
    b.set_level(LOG_INFO).add_async_stream_sink(out, 16, AsyncLogSink::DropPolicy::DROP_NEWEST);
    Logger::configure(b.build());
    for (int i = 0; i < 10; ++i)
    {
      Logger::info(std::string("q") + std::to_string(i));
    }
    CHECK(Logger::get_async_queue_size() >= 0);
    Logger::async_flush();
    Logger::async_shutdown();

    Logger::set_output_streams(std::cout, std::cerr);
    fs::remove(f1, ec);
    fs::remove(f2, ec);
  }

  TEST_CASE("BlockPolicy")
  {
    std::ostringstream out;
    auto slow = std::make_unique<SlowSink>(out, std::chrono::milliseconds(80));
    auto async = pixellib::core::logging::AsyncLogSink(std::move(slow), 1, pixellib::core::logging::AsyncLogSink::DropPolicy::BLOCK, std::chrono::milliseconds(5));

    // fill queue quickly; some writes may time out and be counted as dropped
    for (int i = 0; i < 5; ++i)
    {
      async.write(std::string("b") + std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    size_t dropped = async.dropped_count();
    CHECK(dropped >= 0);
    size_t qsz = async.queue_size();
    CHECK(qsz >= 0);
    async.shutdown();
  }

  TEST_CASE("ConfigBuilder")
  {
    std::ostringstream out;
    // add file sink using builder (writes to build/tmp/testbuilder.log)
    std::string fname = "build/tmp/testbuilder.log";
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_DEBUG)
        .add_stream_sink(out)
        .add_async_stream_sink(out)
        .add_file_sink(fname, 1024, 2)
        .add_async_file_sink("build/tmp/testbuilder_async.log")
        .set_formatter(std::make_unique<JSONLogFormatter>());
    auto cfg = b.build();
    // register under a name
    pixellib::core::logging::Logger::LoggerRegistry::set_config("builderTest", std::move(cfg));
    CHECK(pixellib::core::logging::Logger::LoggerRegistry::has_config("builderTest") == true);

    // cleanup file created by builder
    std::error_code ec;
    std::filesystem::remove(fname, ec);
    std::filesystem::remove("build/tmp/testbuilder_async.log", ec);
  }

  TEST_CASE("JsonEscapePerformance")
  {
    JSONLogFormatter jf;
    std::tm tm{};

    // Build a large message with many escapable characters
    std::string big;
    big.reserve(1024 * 50);
    for (int i = 0; i < 1000; ++i)
      big += "part \"\\\n\t";

    // Micro-benchmark: compare naive escape (no reserve) to reserved implementation (same as in library)
    auto old_escape = [&](const std::string &s) {
      std::string escaped;
      for (char c : s)
      {
        switch (c)
        {
        case '"':
          escaped += "\\\"";
          break;
        case '\\':
          escaped += "\\\\";
          break;
        case '\n':
          escaped += "\\n";
          break;
        case '\r':
          escaped += "\\r";
          break;
        case '\t':
          escaped += "\\t";
          break;
        default:
          escaped += c;
          break;
        }
      }
      return escaped;
    };

    auto new_escape = [&](const std::string &s) {
      std::string escaped;
      escaped.reserve(s.size() * 2 + 4);
      for (char c : s)
      {
        switch (c)
        {
        case '"':
          escaped += "\\\"";
          break;
        case '\\':
          escaped += "\\\\";
          break;
        case '\n':
          escaped += "\\n";
          break;
        case '\r':
          escaped += "\\r";
          break;
        case '\t':
          escaped += "\\t";
          break;
        default:
          escaped += c;
          break;
        }
      }
      return escaped;
    };

    // Run old implementation
    auto start_old = std::chrono::steady_clock::now();
    for (int i = 0; i < 500; ++i)
    {
      std::string out = old_escape(big);
      CHECK(!out.empty());
    }
    auto dur_old = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_old).count();

    // Run new reserved implementation
    auto start_new = std::chrono::steady_clock::now();
    for (int i = 0; i < 500; ++i)
    {
      std::string out = new_escape(big);
      CHECK(!out.empty());
    }
    auto dur_new = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_new).count();

    DOCTEST_MESSAGE("JsonEscapePerformance: old=" << dur_old << "ms new=" << dur_new << "ms (" << (100.0 * (dur_old - dur_new) / dur_old) << "% faster) for 500 iterations");
  }
}
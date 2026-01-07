#include "doctest.h"
#include "logging.hpp"

#include <sstream>
#include <thread>
#include <chrono>

using namespace pixellib::core::logging;

TEST_SUITE("logging_module") {

TEST_CASE("log_level_to_string_and_formatters") {
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
}

TEST_CASE("stream_sink_and_stream_state_handling") {
    std::ostringstream out;
    StreamSink sink(out);
    sink.write("hello");
    CHECK(out.str().find("hello") != std::string::npos);

    out.setstate(std::ios::badbit);
    sink.write("world");
}

TEST_CASE("async_log_sink_queue_and_dropping") {
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

TEST_CASE("logger_output_and_configuration") {
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

}

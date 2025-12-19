#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "interlaced_core/network.hpp"
#include "interlaced_core/filesystem.hpp"
#include "interlaced_core/logging.hpp"
#include "interlaced_core/json.hpp"

// Top-level sanity tests
TEST_SUITE("interlaced_core_sanity") {

TEST_CASE("arithmetic") {
    CHECK(1 + 1 == 2);
}

TEST_CASE("filesystem") {
    CHECK(interlaced::core::filesystem::FileSystem::exists("/nonexistent") == false);
    CHECK(interlaced::core::filesystem::FileSystem::exists("/tmp") == true);
}

TEST_CASE("logging") {
    std::ostringstream out, err;
    // Redirect logger outputs
    interlaced::core::logging::Logger::set_output_streams(out, err);

    // Only WARNING and above should be emitted
    interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_WARNING);

    interlaced::core::logging::Logger::info("filtered out message");
    interlaced::core::logging::Logger::warning("visible warning");

    std::string out_str = out.str();
    CHECK(out_str.find("visible warning") != std::string::npos);
    CHECK(out_str.find("filtered out message") == std::string::npos);

    // Reset to default streams to avoid interfering with other tests
    interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("json") {
    std::string json_str = R"({"key": "value", "number": 42})";
    auto json = interlaced::core::json::JSON::parse_or_throw(json_str);
    CHECK(json["key"].as_string() == "value");
    CHECK(json["number"].as_number().to_int64() == 42);
}

TEST_CASE("network") {
    // Test the simplified HTTP GET implementation
    std::string response = interlaced::core::network::Network::http_get("http://example.com");
    CHECK(response.find("HTTP response from http://example.com") != std::string::npos);
}

} // TEST_SUITE

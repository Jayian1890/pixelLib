#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "interlaced_core/filesystem.hpp"
#include "interlaced_core/json.hpp"
#include "interlaced_core/logging.hpp"
#include "interlaced_core/network.hpp"

TEST_CASE("sanity: headers compile and basic functionality works") {
    // Basic arithmetic check
    CHECK(1 + 1 == 2);

    // Test filesystem module - basic file existence check
    SUBCASE("filesystem basics") {
        CHECK(interlaced::core::filesystem::FileSystem::exists("/nonexistent") == false);
        CHECK(interlaced::core::filesystem::FileSystem::exists("/tmp") == true); // Assuming /tmp exists on Unix-like systems
    }

    // Test JSON module - basic parsing
    SUBCASE("json basics") {
        std::string json_str = R"({"key": "value", "number": 42})";
        auto json = interlaced::core::json::JSON::parse_or_throw(json_str);
        CHECK(json["key"].as_string() == "value");
        CHECK(json["number"].as_number().to_int64() == 42);
    }

    // Test logging module - basic log call (detailed tests in test_logging.cc)
    SUBCASE("logging basics") {
        // Temporarily redirect to avoid console output
        std::ostringstream dummy;
        interlaced::core::logging::Logger::set_output_streams(dummy, dummy);
        interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_INFO);
        
        interlaced::core::logging::Logger::info("Sanity check log message");
        CHECK(dummy.str().find("Sanity check log message") != std::string::npos);
        
        // Reset
        interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
    }

    // Test network module - basic HTTP GET
    SUBCASE("network basics") {
        // Test the simplified HTTP GET implementation
        std::string response = interlaced::core::network::Network::http_get("http://example.com");
        CHECK(response.find("HTTP response from http://example.com") != std::string::npos);
    }
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "network.hpp"
#include "filesystem.hpp"
#include "logging.hpp"
#include "json.hpp"


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
    
    interlaced::core::logging::Logger::set_output_streams(out, err);

    
    interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_WARNING);

    interlaced::core::logging::Logger::info("filtered out message");
    interlaced::core::logging::Logger::warning("visible warning");

    std::string out_str = out.str();
    CHECK(out_str.find("visible warning") != std::string::npos);
    CHECK(out_str.find("filtered out message") == std::string::npos);

    
    interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("json") {
    std::string json_str = R"({"key": "value", "number": 42})";
    auto json = interlaced::core::json::JSON::parse_or_throw(json_str);
    CHECK(json["key"].as_string() == "value");
    CHECK(json["number"].as_number().to_int64() == 42);
}

TEST_CASE("network") {
    
    std::string response = interlaced::core::network::Network::http_get("http:
    CHECK(response.find("HTTP response from http:
}

} 

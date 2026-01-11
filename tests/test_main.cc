#include "../include/filesystem.hpp"
#include "../include/json.hpp"
#include "../include/logging.hpp"
#include "../include/network.hpp"
#include "../third-party/doctest/doctest.h"
#include <iostream>
#include <sstream>
#include <string>

// Cross-platform environment variable helpers
// Note: Error handling intentionally omitted for test simplicity
static void set_env_var(const char *name, const char *value)
{
#ifdef _WIN32
  _putenv_s(name, value);
#else
  setenv(name, value, 1);
#endif
}

static void unset_env_var(const char *name)
{
#ifdef _WIN32
  _putenv_s(name, "");
#else
  unsetenv(name);
#endif
}

TEST_SUITE("Sanity Checks")
{

  TEST_CASE("arithmetic")
  {
    CHECK(1 + 1 == 2);
  }

  TEST_CASE("filesystem")
  {
    CHECK(pixellib::core::filesystem::FileSystem::exists("/nonexistent") == false);
    CHECK(pixellib::core::filesystem::FileSystem::exists("/tmp") == true);
  }

  TEST_CASE("logging")
  {
    std::ostringstream out, err;
    pixellib::core::logging::Logger::set_output_streams(out, err);
    pixellib::core::logging::Logger::set_level(pixellib::core::logging::LOG_WARNING);
    pixellib::core::logging::Logger::info("filtered out message");
    pixellib::core::logging::Logger::warning("visible warning");
    std::string out_str = out.str();
    CHECK(out_str.find("visible warning") != std::string::npos);
    CHECK(out_str.find("filtered out message") == std::string::npos);
    pixellib::core::logging::Logger::set_output_streams(std::cout, std::cerr);
  }

  TEST_CASE("json")
  {
    std::string json_str = R"({"key": "value", "number": 42})";
    auto json = pixellib::core::json::JSON::parse_or_throw(json_str);
    CHECK(json["key"].as_string() == "value");
    CHECK(json["number"].as_number().to_int64() == 42);
  }

  TEST_CASE("network")
  {
    // Set test mode for deterministic behavior
    set_env_var("PIXELLIB_TEST_MODE", "1");

    std::string response = pixellib::core::network::Network::http_get("http://example/test");
    CHECK(response.find("Mock HTTP response from http://example/test") != std::string::npos);

    std::string r2 = pixellib::core::network::Network::http_post("http://example/post", "payload");
    CHECK(r2.find("payload") != std::string::npos);

    std::string r3 = pixellib::core::network::Network::https_get("https://example/test");
    CHECK(r3.find("https://example/test") != std::string::npos);

    std::string r4 = pixellib::core::network::Network::https_post("https://example/post", "p");
    CHECK(r4.find('p') != std::string::npos);

    double r5 = pixellib::core::network::Network::measure_bandwidth("http://example/test");
    CHECK(r5 > 0.0);

    // Clean up
    unset_env_var("PIXELLIB_TEST_MODE");
  }
}

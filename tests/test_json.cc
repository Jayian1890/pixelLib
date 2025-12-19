#include "doctest.h"
#include "interlaced_core/json.hpp"

TEST_SUITE("interlaced_core_json") {

TEST_CASE("test") {
    std::string json_str = R"({"key": "value", "number": 42})";
    auto json = interlaced::core::json::JSON::parse_or_throw(json_str);
    CHECK(json["key"].as_string() == "value");
    CHECK(json["number"].as_number().to_int64() == 42);
}

} // TEST_SUITE

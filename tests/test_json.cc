#include "doctest.h"
#include "json.hpp"
#include <sstream>
#include <limits>

using namespace pixellib::core::json;
using doctest::Approx;

TEST_SUITE("json_module") {

TEST_CASE("parse_literals_and_types") {
    JSON v;
    CHECK(JSON::parse("null", v) == true);
    CHECK(v.is_null());

    CHECK(JSON::parse("true", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == true);

    CHECK(JSON::parse("false", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == false);
}

TEST_CASE("parse_numbers_and_number_helpers") {
    JSON v;
    CHECK(JSON::parse("42", v));
    CHECK(v.is_number());
    CHECK(v.as_number().is_integral());
    CHECK(v.as_number().to_int64() == 42);
    CHECK(Approx(v.as_number().to_double(0.0)) == 42.0);

    CHECK(JSON::parse("-3.14", v));
    CHECK(v.is_number());
    CHECK_FALSE(v.as_number().is_integral());
    CHECK(Approx(v.as_number().to_double()) == -3.14);

    CHECK(JSON::parse("1e3", v));
    CHECK(v.is_number());
    CHECK(Approx(v.as_number().to_double()) == 1000.0);
}

TEST_CASE("parse_strings_and_escapes") {
    JSON v;
    CHECK(JSON::parse("\"hello\\nworld\"", v));
    CHECK(v.is_string());
    CHECK(v.as_string() == "hello\nworld");

    
    CHECK(JSON::parse("\"\\u0041\\u0042\\u0043\"", v));
    CHECK(v.as_string() == "ABC");

    
    CHECK(JSON::parse("\"\\uD83D\\uDE00\"", v));
    CHECK(v.as_string().size() >= 4); 
}

TEST_CASE("parse_arrays_and_objects") {
    JSON v;
    CHECK(JSON::parse("[1, 2, 3]", v));
    CHECK(v.is_array());
    CHECK(v.as_array().size() == 3);

    CHECK(JSON::parse("{\"a\":1, \"b\":[true, false]}", v));
    CHECK(v.is_object());
    const JSON *p = v.find("a");
    REQUIRE(p);
    CHECK(p->is_number());
    CHECK(p->as_number().to_int64() == 1);

    p = v.find("b");
    REQUIRE(p);
    REQUIRE(p->is_array());
    CHECK(p->as_array().size() == 2);
}

TEST_CASE("parse_errors_and_messages") {
    JSON v;
    JsonError err;
    CHECK_FALSE(JSON::parse("\"unterminated", v, &err));
    CHECK(err.message.find("Unterminated string") != std::string::npos);

    CHECK_FALSE(JSON::parse("[1, , 2]", v, &err));
    {
        bool found = (err.message.find("Unexpected character") != std::string::npos) || (err.message.find("Expected") != std::string::npos);
        CHECK(found);
    }

    CHECK_THROWS_AS(JSON::parse_or_throw("{invalid}"), std::invalid_argument);
}

TEST_CASE("stringify_and_escape_options") {
    JSON obj = JSON::object(JSON::object_t{{"k1", JSON(std::string("v1"))},{"k2", JSON::array(JSON::array_t{JSON::number("1"), JSON::number("2")})}});
    std::string compact = obj.stringify({false, 2, false});
    INFO(compact);
    CHECK(compact.find("\"k1\"") != std::string::npos);
    CHECK(compact.find("v1") != std::string::npos);

    std::string pretty = obj.stringify({true, 4, false});
    CHECK(pretty.find("\n") != std::string::npos);

    
    JSON s = JSON(std::string("/path/"));
    std::string escaped = s.stringify({false,2,true});
    CHECK(escaped.find("\\/") != std::string::npos);
}

TEST_CASE("validate_and_parse_or_throw") {
    CHECK(JSON::validate("{\"x\": 1}"));
    CHECK_FALSE(JSON::validate("{\"x\": }"));

    CHECK_NOTHROW(JSON::parse_or_throw("[true,false]"););
}

TEST_CASE("number_conversion_fallbacks") {
    
    JSON n = JSON::number("notanumber");
    CHECK(Approx(n.as_number().to_double(3.14)) == 3.14);
    CHECK(n.as_number().to_int64(42) == 42);

    
    JSON big = JSON::number("9999999999999999999999999999");
    CHECK(big.as_number().to_int64(123) == std::numeric_limits<int64_t>::max());

    
    JSON frac = JSON::number("1.23");
    CHECK_FALSE(frac.as_number().is_integral());
}

TEST_CASE("unicode_escape_error_cases") {
    JSON v;
    JsonError err;

    
    CHECK_FALSE(JSON::parse("\"\\uZZZZ\"", v, &err));
    CHECK(err.message.find("Invalid hex in unicode escape") != std::string::npos);

    
    CHECK_FALSE(JSON::parse("\"\\uD800\\u0041\"", v, &err));
    CHECK(err.message.find("Invalid low surrogate") != std::string::npos);

    
    CHECK_FALSE(JSON::parse("\"\\uD83D\"", v, &err));
    CHECK(err.message.find("Missing low surrogate") != std::string::npos);
}
} 

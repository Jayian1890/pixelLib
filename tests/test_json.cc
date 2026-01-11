#include "../include/json.hpp"
#include "../third-party/doctest/doctest.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

TEST_SUITE("JSON Module")
{
  TEST_CASE("ParseLiterals")
  {
    pixellib::core::json::JSON v;
    CHECK(pixellib::core::json::JSON::parse("null", v) == true);
    CHECK(v.is_null());

    CHECK(pixellib::core::json::JSON::parse("true", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == true);

    CHECK(pixellib::core::json::JSON::parse("false", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == false);
  }

  TEST_CASE("ParseNumbers")
  {
    pixellib::core::json::JSON v;
    CHECK(pixellib::core::json::JSON::parse("42", v));
    CHECK(v.is_number());
    CHECK(v.as_number().is_integral());
    CHECK(v.as_number().to_int64() == 42);
    CHECK(v.as_number().to_double(0.0) == 42.0);

    CHECK(pixellib::core::json::JSON::parse("-3.14", v));
    CHECK(v.is_number());
    CHECK_FALSE(v.as_number().is_integral());
    CHECK(v.as_number().to_double() == -3.14);

    CHECK(pixellib::core::json::JSON::parse("1e3", v));
    CHECK(v.is_number());
    CHECK(v.as_number().to_double() == 1000.0);
  }

  TEST_CASE("ParseStrings")
  {
    pixellib::core::json::JSON v;
    CHECK(pixellib::core::json::JSON::parse("\"hello\\nworld\"", v));
    CHECK(v.is_string());
    CHECK(v.as_string() == "hello\nworld");

    CHECK(pixellib::core::json::JSON::parse("\"\\u0041\\u0042\\u0043\"", v));
    CHECK(v.as_string() == "ABC");

    CHECK(pixellib::core::json::JSON::parse("\"\\uD83D\\uDE00\"", v));
    CHECK(v.as_string().size() >= 4);
  }

  TEST_CASE("ParseArrays")
  {
    pixellib::core::json::JSON v;
    CHECK(pixellib::core::json::JSON::parse("[1, 2, 3]", v));
    CHECK(v.is_array());
    CHECK(v.as_array().size() == 3);

    CHECK(pixellib::core::json::JSON::parse("{\"a\":1, \"b\":[true, false]}", v));
    CHECK(v.is_object());
    const pixellib::core::json::JSON *p = v.find("a");
    REQUIRE(p);
    CHECK(p->is_number());
    CHECK(p->as_number().to_int64() == 1);

    p = v.find("b");
    REQUIRE(p);
    REQUIRE(p->is_array());
    CHECK(p->as_array().size() == 2);
  }

  TEST_CASE("ParseErrors")
  {
    pixellib::core::json::JSON v;
    pixellib::core::json::JsonError err;
    CHECK_FALSE(pixellib::core::json::JSON::parse("\"unterminated", v, &err));
    CHECK(err.message.find("Unterminated string") != std::string::npos);

    CHECK_FALSE(pixellib::core::json::JSON::parse("[1, , 2]", v, &err));
    {
      bool found = (err.message.find('E') != std::string::npos) || (err.message.find("Unexpected character") != std::string::npos);
      CHECK(found);
    }

    CHECK_THROWS_AS(pixellib::core::json::JSON::parse_or_throw("{invalid}"), std::invalid_argument);
  }

  TEST_CASE("Stringify")
  {
    pixellib::core::json::JSON obj = pixellib::core::json::JSON::object(pixellib::core::json::JSON::object_t{
        {"k1", pixellib::core::json::JSON(std::string("v1"))},
        {"k2", pixellib::core::json::JSON::array(pixellib::core::json::JSON::array_t{pixellib::core::json::JSON::number("1"), pixellib::core::json::JSON::number("2")})}});
    std::string compact = obj.stringify({false, 2, false});
    INFO(compact);
    CHECK(compact.find("\"k1\"") != std::string::npos);
    CHECK(compact.find("v1") != std::string::npos);

    std::string pretty = obj.stringify({true, 4, false});
    CHECK(pretty.find('\n') != std::string::npos);

    pixellib::core::json::JSON s = pixellib::core::json::JSON(std::string("/path/"));
    std::string escaped = s.stringify({false, 2, true});
    CHECK(escaped.find("\\/") != std::string::npos);
  }

  TEST_CASE("Validate")
  {
    CHECK(pixellib::core::json::JSON::validate("{\"x\": 1}"));
    CHECK_FALSE(pixellib::core::json::JSON::validate("{\"x\": }"));

    CHECK_NOTHROW(pixellib::core::json::JSON::parse_or_throw("[true,false]"));
  }

  TEST_CASE("NumberConversion")
  {

    pixellib::core::json::JSON n = pixellib::core::json::JSON::number("notanumber");
    CHECK(n.as_number().to_double(3.14) == 3.14);
    CHECK(n.as_number().to_int64(42) == 42);

    pixellib::core::json::JSON big = pixellib::core::json::JSON::number("9999999999999999999999999999");
    CHECK(big.as_number().to_int64(123) == std::numeric_limits<int64_t>::max());

    pixellib::core::json::JSON frac = pixellib::core::json::JSON::number("1.23");
    CHECK_FALSE(frac.as_number().is_integral());
  }

  TEST_CASE("UnicodeErrors")
  {
    pixellib::core::json::JSON v;
    pixellib::core::json::JsonError err;

    CHECK_FALSE(pixellib::core::json::JSON::parse("\"\\uZZZZ\"", v, &err));
    CHECK(err.message.find("Invalid hex in unicode escape") != std::string::npos);

    CHECK_FALSE(pixellib::core::json::JSON::parse("\"\\uD800\\u0041\"", v, &err));
    CHECK(err.message.find("Invalid low surrogate") != std::string::npos);

    CHECK_FALSE(pixellib::core::json::JSON::parse("\"\\uD83D\"", v, &err));
    CHECK(err.message.find("Missing low surrogate") != std::string::npos);
  }
}
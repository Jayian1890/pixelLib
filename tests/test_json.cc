#include "doctest.h"
#include "interlaced_core/json.hpp"
#include <sstream>
#include <string>

using interlaced::core::json::JSON;
using interlaced::core::json::JsonError;
using interlaced::core::json::StringifyOptions;

TEST_CASE("json: parse primitives") {
    JSON v;
    CHECK(JSON::parse("null", v) == true);
    CHECK(v.is_null());

    CHECK(JSON::parse("true", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == true);

    CHECK(JSON::parse("false", v) == true);
    CHECK(v.is_bool());
    CHECK(v.as_bool() == false);

    CHECK(JSON::parse("0", v) == true);
    CHECK(v.is_number());
    CHECK(v.as_number().repr == "0");
    CHECK(v.as_number().is_integral() == true);
    CHECK(v.as_number().to_int64() == 0);
    CHECK(v.as_number().to_double() == doctest::Approx(0.0));

    CHECK(JSON::parse("-123", v) == true);
    CHECK(v.as_number().is_integral() == true);
    CHECK(v.as_number().to_int64() == -123);

    CHECK(JSON::parse("3.14159", v) == true);
    CHECK(v.as_number().is_integral() == false);
    CHECK(v.as_number().to_double() == doctest::Approx(3.14159));

    CHECK(JSON::parse("1e3", v) == true);
    CHECK(v.as_number().to_double() == doctest::Approx(1000.0));

    CHECK(JSON::parse("-2.5E-3", v) == true);
    CHECK(v.as_number().to_double() == doctest::Approx(-0.0025));
}

TEST_CASE("json: parse strings and escapes") {
    JSON v;
    CHECK(JSON::parse("\"hello\"", v) == true);
    CHECK(v.is_string());
    CHECK(v.as_string() == "hello");

    CHECK(JSON::parse("\"\\\\\\\"/\\b\\f\\n\\r\\t\"", v) == true);
    CHECK(v.as_string() == "\\\"/\b\f\n\r\t");

    // Unicode BMP
    CHECK(JSON::parse("\"\u0041\u00DF\u4E2D\uFF21\"", v) == true);
    CHECK(v.as_string() == std::string("A\xC3\x9F\xE4\xB8\xAD\xEF\xBC\xA1"));

    // Unicode surrogate pair (U+1D11E MUSICAL SYMBOL G CLEF)
    CHECK(JSON::parse("\"\\uD834\\uDD1E\"", v) == true);
    // UTF-8 for U+1D11E
    CHECK(v.as_string() == std::string("\xF0\x9D\x84\x9E"));
}

TEST_CASE("json: parse arrays and objects") {
    JSON v;
    std::string arr = "[null,true,false,123,\"x\",[1,2],{\"a\":1}]";
    CHECK(JSON::parse(arr, v) == true);
    CHECK(v.is_array());
    const auto &a = v.as_array();
    CHECK(a.size() == 7);
    CHECK(a[0].is_null());
    CHECK(a[1].as_bool() == true);
    CHECK(a[2].as_bool() == false);
    CHECK(a[3].as_number().to_int64() == 123);
    CHECK(a[4].as_string() == "x");
    CHECK(a[5].is_array());
    CHECK(a[5].as_array().size() == 2);
    CHECK(a[5].as_array()[1].as_number().to_int64() == 2);
    CHECK(a[6].is_object());
    CHECK(a[6].find("a") != nullptr);
    CHECK(a[6].find("a")->as_number().to_int64() == 1);

    std::string obj = "{\"k1\":\"v\",\"k2\":[1,2],\"k3\":{}}";
    CHECK(JSON::validate(obj) == true);
    CHECK_NOTHROW(JSON::parse_or_throw(obj));
}

TEST_CASE("json: stringify primitives and structures") {
    // Build object manually to control order
    JSON o = JSON::object({
        {"a", JSON(true)},
        {"b", JSON(123.0)},
        {"c", JSON(std::string("/"))}
    });

    StringifyOptions opts;
    opts.pretty = false;
    opts.escape_solidus = false;
    std::string s = o.stringify(opts);
    CHECK(s == "{\"a\":true,\"b\":123,\"c\":\"/\"}");

    opts.pretty = true;
    opts.indent = 2;
    s = o.stringify(opts);
    CHECK(s == std::string("{\n  \"a\": true,\n  \"b\": 123,\n  \"c\": \"/\"\n}"));

    // escape solidus
    opts.escape_solidus = true;
    s = o.stringify(opts);
    CHECK(s.find("\\/\"") != std::string::npos);

    // Array pretty formatting
    JSON arr = JSON::array({ JSON(1.0), JSON(2.0), JSON(3.0) });
    s = arr.stringify(opts);
    CHECK(s == std::string("[\n  1,\n  2,\n  3\n]"));
}

TEST_CASE("json: number conversion bounds") {
    using I = std::numeric_limits<int64_t>;
    JSON big = JSON::number("9223372036854775808"); // > INT64_MAX
    CHECK(big.as_number().to_int64() == I::max());
    CHECK(big.as_number().to_int64(-123) == I::max());

    JSON small = JSON::number("-9223372036854775809"); // < INT64_MIN
    CHECK(small.as_number().to_int64() == I::min());
    CHECK(small.as_number().to_int64(42) == I::min());

    // Non-numeric falls back
    JSON bad = JSON::number("abc");
    CHECK(bad.as_number().to_int64(777) == 777);
}

TEST_CASE("json: mutation via array and object helpers") {
    JSON arr = JSON::array({});
    arr.push_back(JSON(true));
    arr.push_back(JSON(std::string("ok")));
    CHECK(arr.as_array().size() == 2);
    CHECK(arr.as_array()[0].as_bool() == true);
    CHECK(arr.as_array()[1].as_string() == "ok");

    JSON obj = JSON::object({});
    obj["x"] = JSON(42.0);
    obj["y"] = JSON(std::string("str"));
    const JSON *px = obj.find("x");
    const JSON *py = obj.find("y");
    const JSON *pz = obj.find("z");
    CHECK(px != nullptr);
    CHECK(py != nullptr);
    CHECK(pz == nullptr);
    CHECK(px->as_number().to_int64() == 42);
    CHECK(py->as_string() == "str");
}

TEST_CASE("json: parse errors and positions") {
    JSON v; JsonError err;

    // Trailing characters
    CHECK(JSON::parse("true x", v, &err) == false);
    CHECK(err.message.find("Trailing characters") != std::string::npos);
    CHECK(err.position == 5);

    // Invalid number
    CHECK(JSON::parse("-", v, &err) == false);
    CHECK(err.message.find("Invalid number") != std::string::npos);

    // Invalid fraction
    CHECK(JSON::parse("1.", v, &err) == false);
    CHECK(err.message.find("Invalid fraction") != std::string::npos);

    // Invalid exponent
    CHECK(JSON::parse("1e", v, &err) == false);
    CHECK(err.message.find("Invalid exponent") != std::string::npos);

    // Missing colon in object
    CHECK(JSON::parse("{\"a\" \"b\"}", v, &err) == false);
    CHECK(err.message.find("Expected ':' after object key") != std::string::npos);

    // Expected comma in array
    CHECK(JSON::parse("[1 2]", v, &err) == false);
    CHECK(err.message.find("Expected ',' between array elements") != std::string::npos);

    // Unterminated string
    CHECK(JSON::parse("\"unterminated", v, &err) == false);
    CHECK(err.message.find("Unterminated string") != std::string::npos);

    // Invalid escape
    CHECK(JSON::parse("\"\\q\"", v, &err) == false);
    CHECK(err.message.find("Invalid escape") != std::string::npos);

    // Incomplete unicode escape
    CHECK(JSON::parse("\"\\u12\"", v, &err) == false);
    CHECK(err.message.find("Incomplete unicode escape") != std::string::npos);

    // Surrogate pair: missing low surrogate
    CHECK(JSON::parse("\"\\uD834x\"", v, &err) == false);
    CHECK(err.message.find("Missing low surrogate") != std::string::npos);
}

TEST_CASE("json: round-trip parse and stringify") {
    std::string text = R"({"a": [1, 2, 3], "b": {"c": "/"}})";
    JSON j = JSON::parse_or_throw(text);
    StringifyOptions opts; opts.pretty = false; opts.escape_solidus = true;
    std::string s = j.stringify(opts);
    JSON j2 = JSON::parse_or_throw(s);
    // Check a few fields
    CHECK(j2.find("a") != nullptr);
    CHECK(j2.find("a")->is_array());
    CHECK(j2.find("a")->as_array().size() == 3);
    CHECK(j2.find("b") != nullptr);
    CHECK(j2.find("b")->is_object());
    CHECK(j2.find("b")->find("c") != nullptr);
    CHECK(j2.find("b")->find("c")->as_string() == "/");
}

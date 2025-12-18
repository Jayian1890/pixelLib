#include <string>

#include <gtest/gtest.h>

#include "interlaced_core/json.hpp"

using namespace interlaced::core::json;

static const JSON *get_member(const JSON &obj, const std::string &key) {
  return obj.find(key);
}

TEST(JSON, ParseSimpleObject) {
  const std::string json = R"({"name":"John","age":30})";
  JSON value = JSON::parse_or_throw(json);
  ASSERT_TRUE(value.is_object());
  const auto *name = get_member(value, "name");
  const auto *age = get_member(value, "age");
  ASSERT_NE(name, nullptr);
  ASSERT_NE(age, nullptr);
  EXPECT_TRUE(name->is_string());
  EXPECT_EQ(name->as_string(), "John");
  EXPECT_TRUE(age->is_number());
  EXPECT_EQ(age->as_number().repr, "30");
}

TEST(JSON, ParseArrayRoot) {
  const std::string json = R"([1,true,{"k":"v"}])";
  JSON value = JSON::parse_or_throw(json);
  ASSERT_TRUE(value.is_array());
  const auto &arr = value.as_array();
  ASSERT_EQ(arr.size(), 3u);
  EXPECT_TRUE(arr[0].is_number());
  EXPECT_EQ(arr[0].as_number().repr, "1");
  EXPECT_TRUE(arr[1].is_bool());
  EXPECT_TRUE(arr[1].as_bool());
  ASSERT_TRUE(arr[2].is_object());
  const auto *k = get_member(arr[2], "k");
  ASSERT_NE(k, nullptr);
  EXPECT_EQ(k->as_string(), "v");
}

TEST(JSON, ParseUnicodeAndEscapes) {
  const std::string json = R"({"text":"hi \"snowman\" \u2603"})";
  JSON value = JSON::parse_or_throw(json);
  const auto *text = get_member(value, "text");
  ASSERT_NE(text, nullptr);
  EXPECT_EQ(text->as_string(), "hi \"snowman\" \xE2\x98\x83");
}

TEST(JSON, ValidateAndFailures) {
  EXPECT_TRUE(JSON::validate(R"({"ok":true})"));
  EXPECT_TRUE(JSON::validate(R"([1,2,3])"));
  EXPECT_FALSE(JSON::validate(""));
  EXPECT_FALSE(JSON::validate(R"({"a":1,})"));
}

TEST(JSON, NumberFormats) {
  JSON value = JSON::parse_or_throw(R"({"n1":-15,"n2":1.5e10,"n3":0.5})");
  const auto *n1 = get_member(value, "n1");
  const auto *n2 = get_member(value, "n2");
  const auto *n3 = get_member(value, "n3");
  ASSERT_NE(n1, nullptr);
  ASSERT_NE(n2, nullptr);
  ASSERT_NE(n3, nullptr);
  EXPECT_EQ(n1->as_number().repr, "-15");
  EXPECT_EQ(n2->as_number().repr, "1.5e10");
  EXPECT_EQ(n3->as_number().repr, "0.5");
}

TEST(JSON, InvalidInputsThrow) {
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":1.2.3})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":1e})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":-})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":tr})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":fal})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"value":nu})"), std::invalid_argument);
  EXPECT_THROW(JSON::parse_or_throw(R"({"text":"unterminated})"), std::invalid_argument);
}

TEST(JSON, StringifyCompactAndPretty) {
  JSON value = JSON::object({
      {"name", JSON(std::string("John"))},
      {"age", JSON::number("30")},
      {"tags", JSON::array({JSON(std::string("a")), JSON(std::string("b"))})}
  });

  const std::string compact = value.stringify();
  EXPECT_EQ(compact, R"({"name":"John","age":30,"tags":["a","b"]})");

  StringifyOptions pretty{true, 2, false};
  const std::string pretty_out = value.stringify(pretty);
  EXPECT_NE(pretty_out.find('\n'), std::string::npos);
}

TEST(JSON, RoundTrip) {
  const std::string original = R"({"user":{"name":"John","scores":[1,2,3]},"active":true})";
  JSON parsed = JSON::parse_or_throw(original);
  const std::string emitted = parsed.stringify();
  JSON reparsed = JSON::parse_or_throw(emitted);
  const auto *user = get_member(reparsed, "user");
  ASSERT_NE(user, nullptr);
  const auto *name = get_member(*user, "name");
  ASSERT_NE(name, nullptr);
  EXPECT_EQ(name->as_string(), "John");
}

// Tests for Number conversion methods
TEST(JSON, NumberToDouble) {
  JSON::Number num1{"3.14159"};
  EXPECT_NEAR(num1.to_double(), 3.14159, 0.00001);
  
  JSON::Number num2{"-42.5"};
  EXPECT_NEAR(num2.to_double(), -42.5, 0.00001);
  
  JSON::Number num3{"1.5e10"};
  EXPECT_NEAR(num3.to_double(), 1.5e10, 1e6);
  
  // Test fallback for invalid string
  JSON::Number invalid{"not_a_number"};
  EXPECT_EQ(invalid.to_double(99.9), 99.9);
  EXPECT_EQ(invalid.to_double(), 0.0);
}

TEST(JSON, NumberToInt64) {
  JSON::Number num1{"42"};
  EXPECT_EQ(num1.to_int64(), 42);
  
  JSON::Number num2{"-123"};
  EXPECT_EQ(num2.to_int64(), -123);
  
  JSON::Number num3{"0"};
  EXPECT_EQ(num3.to_int64(), 0);
  
  // Test fallback for invalid string
  JSON::Number invalid{"not_a_number"};
  EXPECT_EQ(invalid.to_int64(77), 77);
  EXPECT_EQ(invalid.to_int64(), 0);
  
  // Test fallback for non-integral value
  JSON::Number floating{"3.14"};
  EXPECT_EQ(floating.to_int64(88), 88);
}

TEST(JSON, NumberIsIntegral) {
  JSON::Number int_num{"42"};
  EXPECT_TRUE(int_num.is_integral());
  
  JSON::Number float_num{"3.14"};
  EXPECT_FALSE(float_num.is_integral());
  
  JSON::Number invalid{"abc"};
  EXPECT_FALSE(invalid.is_integral());
  
  JSON::Number zero{"0"};
  EXPECT_TRUE(zero.is_integral());
  
  JSON::Number negative{"-999"};
  EXPECT_TRUE(negative.is_integral());
}

// Tests for constructors
TEST(JSON, ConstructorWithNumber) {
  JSON::Number num{"123.456"};
  JSON json(num);
  EXPECT_TRUE(json.is_number());
  EXPECT_EQ(json.as_number().repr, "123.456");
}

TEST(JSON, ConstructorWithDouble) {
  JSON json(3.14);
  EXPECT_TRUE(json.is_number());
  // Should contain "3.14" in the representation
  const std::string &repr = json.as_number().repr;
  EXPECT_NE(repr.find("3.14"), std::string::npos);
}

// Tests for type() method
TEST(JSON, TypeMethod) {
  JSON null_val(nullptr);
  EXPECT_EQ(null_val.type(), JSON::Type::Null);
  
  JSON bool_val(true);
  EXPECT_EQ(bool_val.type(), JSON::Type::Bool);
  
  JSON num_val = JSON::number("42");
  EXPECT_EQ(num_val.type(), JSON::Type::Number);
  
  JSON str_val(std::string("hello"));
  EXPECT_EQ(str_val.type(), JSON::Type::String);
  
  JSON arr_val = JSON::array({});
  EXPECT_EQ(arr_val.type(), JSON::Type::Array);
  
  JSON obj_val = JSON::object({});
  EXPECT_EQ(obj_val.type(), JSON::Type::Object);
}

// Tests for is_null()
TEST(JSON, IsNull) {
  JSON null_val(nullptr);
  EXPECT_TRUE(null_val.is_null());
  
  JSON bool_val(true);
  EXPECT_FALSE(bool_val.is_null());
  
  JSON default_val;
  EXPECT_TRUE(default_val.is_null());
}

// Tests for as_bool() with fallback
TEST(JSON, AsBoolWithFallback) {
  JSON bool_true(true);
  EXPECT_TRUE(bool_true.as_bool());
  EXPECT_TRUE(bool_true.as_bool(false));
  
  JSON bool_false(false);
  EXPECT_FALSE(bool_false.as_bool());
  EXPECT_FALSE(bool_false.as_bool(true));
  
  // Non-bool values should return fallback
  JSON num_val = JSON::number("42");
  EXPECT_TRUE(num_val.as_bool(true));
  EXPECT_FALSE(num_val.as_bool(false));
  
  JSON str_val(std::string("test"));
  EXPECT_TRUE(str_val.as_bool(true));
  EXPECT_FALSE(str_val.as_bool(false));
}

// Tests for push_back()
TEST(JSON, PushBack) {
  JSON arr = JSON::array({});
  
  JSON &first = arr.push_back(JSON(true));
  EXPECT_TRUE(first.is_bool());
  EXPECT_TRUE(first.as_bool());
  EXPECT_EQ(arr.as_array().size(), 1u);
  
  JSON &second = arr.push_back(JSON(std::string("hello")));
  EXPECT_TRUE(second.is_string());
  EXPECT_EQ(second.as_string(), "hello");
  EXPECT_EQ(arr.as_array().size(), 2u);
  
  JSON &third = arr.push_back(JSON::number("99"));
  EXPECT_TRUE(third.is_number());
  EXPECT_EQ(arr.as_array().size(), 3u);
}

// Tests for operator[]
TEST(JSON, OperatorBracketInsertAndAccess) {
  JSON obj = JSON::object({});
  
  // Insert and access via operator[]
  JSON &val1 = obj["name"];
  val1 = JSON(std::string("Alice"));
  EXPECT_EQ(val1.as_string(), "Alice");
  
  JSON &val2 = obj["age"];
  val2 = JSON::number("30");
  EXPECT_EQ(val2.as_number().repr, "30");
  
  // Access existing keys
  EXPECT_EQ(obj["name"].as_string(), "Alice");
  EXPECT_EQ(obj["age"].as_number().repr, "30");
  
  // New key should create null entry
  JSON &val3 = obj["city"];
  EXPECT_TRUE(val3.is_null());
  obj["city"] = JSON(std::string("NYC"));
  EXPECT_EQ(obj["city"].as_string(), "NYC");
}

// Tests for mutable as_array()
TEST(JSON, MutableAsArray) {
  JSON arr = JSON::array({JSON(std::string("a")), JSON(std::string("b"))});
  
  auto &mutable_arr = arr.as_array();
  EXPECT_EQ(mutable_arr.size(), 2u);
  
  mutable_arr.push_back(JSON(std::string("c")));
  EXPECT_EQ(arr.as_array().size(), 3u);
  EXPECT_EQ(arr.as_array()[2].as_string(), "c");
}

// Tests for mutable as_object()
TEST(JSON, MutableAsObject) {
  JSON obj = JSON::object({{"key1", JSON(std::string("val1"))}});
  
  auto &mutable_obj = obj.as_object();
  EXPECT_EQ(mutable_obj.size(), 1u);
  
  mutable_obj.emplace_back("key2", JSON(std::string("val2")));
  EXPECT_EQ(obj.as_object().size(), 2u);
  EXPECT_EQ(obj.find("key2")->as_string(), "val2");
}

// Tests for error handling in type conversion
TEST(JSON, AsNumberThrows) {
  JSON str_val(std::string("not a number"));
  EXPECT_THROW(str_val.as_number(), std::logic_error);
}

TEST(JSON, AsStringThrows) {
  JSON num_val = JSON::number("42");
  EXPECT_THROW(num_val.as_string(), std::logic_error);
}

TEST(JSON, AsArrayThrows) {
  JSON obj_val = JSON::object({});
  EXPECT_THROW(obj_val.as_array(), std::logic_error);
}

TEST(JSON, MutableAsArrayThrows) {
  JSON num_val = JSON::number("42");
  EXPECT_THROW(num_val.as_array(), std::logic_error);
}

TEST(JSON, AsObjectThrows) {
  JSON arr_val = JSON::array({});
  EXPECT_THROW(arr_val.as_object(), std::logic_error);
}

TEST(JSON, MutableAsObjectThrows) {
  JSON str_val(std::string("test"));
  EXPECT_THROW(str_val.as_object(), std::logic_error);
}

// Tests for escape_solidus option
TEST(JSON, EscapeSolidus) {
  JSON json(std::string("a/b"));
  
  StringifyOptions normal{false, 2, false};
  std::string normal_out = json.stringify(normal);
  EXPECT_EQ(normal_out, R"("a/b")");
  
  StringifyOptions escape{false, 2, true};
  std::string escape_out = json.stringify(escape);
  EXPECT_EQ(escape_out, R"("a\/b")");
}

// Tests for edge cases in parsing
TEST(JSON, ParseEmptyArray) {
  JSON arr = JSON::parse_or_throw("[]");
  EXPECT_TRUE(arr.is_array());
  EXPECT_EQ(arr.as_array().size(), 0u);
}

TEST(JSON, ParseEmptyObject) {
  JSON obj = JSON::parse_or_throw("{}");
  EXPECT_TRUE(obj.is_object());
  EXPECT_EQ(obj.as_object().size(), 0u);
}

TEST(JSON, ParseNestedStructures) {
  const std::string json_str = R"({
    "users": [
      {"id": 1, "name": "Alice"},
      {"id": 2, "name": "Bob"}
    ]
  })";
  
  JSON json = JSON::parse_or_throw(json_str);
  const auto *users = json.find("users");
  ASSERT_NE(users, nullptr);
  EXPECT_TRUE(users->is_array());
  EXPECT_EQ(users->as_array().size(), 2u);
}

// Tests for pretty printing
TEST(JSON, PrettyPrintingWithIndent) {
  JSON json = JSON::object({
    {"name", JSON(std::string("John"))},
    {"nested", JSON::object({
      {"key", JSON::number("42")}
    })}
  });
  
  StringifyOptions pretty{true, 4, false};
  std::string output = json.stringify(pretty);
  
  // Should contain newlines and indentation
  EXPECT_NE(output.find('\n'), std::string::npos);
  EXPECT_NE(output.find("    "), std::string::npos);
}

// Additional tests for comprehensive unicode coverage
TEST(JSON, UnicodeEscapesSingleByte) {
  // \u0041 = 'A'
  const std::string json = R"({"ch":"\u0041"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *ch = parsed.find("ch");
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(ch->as_string(), "A");
}

TEST(JSON, UnicodeEscapesTwoByte) {
  // \u00E9 = é (U+00E9)
  const std::string json = R"({"ch":"\u00E9"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *ch = parsed.find("ch");
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(ch->as_string().length(), 2u); // UTF-8 encoded as 2 bytes
}

TEST(JSON, UnicodeEscapesThreeByte) {
  // \u2665 = ♥ (U+2665)
  const std::string json = R"({"ch":"\u2665"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *ch = parsed.find("ch");
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(ch->as_string().length(), 3u); // UTF-8 encoded as 3 bytes
}

TEST(JSON, UnicodeEscapesSurrogate) {
  // \uD834\uDD1E = 𝄞 (U+1D11E - musical symbol)
  const std::string json = R"({"symbol":"\uD834\uDD1E"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *symbol = parsed.find("symbol");
  ASSERT_NE(symbol, nullptr);
  EXPECT_EQ(symbol->as_string().length(), 4u); // UTF-8 encoded as 4 bytes
}

TEST(JSON, StringWithControlCharacters) {
  // Test parsing strings with various escape sequences
  const std::string json = R"({"text":"line1\nline2\ttab\b\f\r"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *text = parsed.find("text");
  ASSERT_NE(text, nullptr);
  EXPECT_NE(text->as_string().find('\n'), std::string::npos);
  EXPECT_NE(text->as_string().find('\t'), std::string::npos);
}

TEST(JSON, StringRoundTripWithEscapes) {
  const std::string original = R"({"path":"C:\\Users\\Name","json":"{\"key\":\"value\"}"})";
  JSON parsed = JSON::parse_or_throw(original);
  std::string stringified = parsed.stringify();
  JSON reparsed = JSON::parse_or_throw(stringified);
  
  const auto *path = reparsed.find("path");
  const auto *json_str = reparsed.find("json");
  ASSERT_NE(path, nullptr);
  ASSERT_NE(json_str, nullptr);
  EXPECT_EQ(path->as_string(), "C:\\Users\\Name");
  EXPECT_EQ(json_str->as_string(), "{\"key\":\"value\"}");
}

// Tests for all escape sequences
TEST(JSON, AllEscapeSequences) {
  const std::string json = R"({"esc":"\"\\\/\b\f\n\r\t"})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *esc = parsed.find("esc");
  ASSERT_NE(esc, nullptr);
  const auto &str = esc->as_string();
  EXPECT_NE(str.find('"'), std::string::npos);
  EXPECT_NE(str.find('\\'), std::string::npos);
  EXPECT_NE(str.find('/'), std::string::npos);
  EXPECT_NE(str.find('\b'), std::string::npos);
  EXPECT_NE(str.find('\f'), std::string::npos);
  EXPECT_NE(str.find('\n'), std::string::npos);
  EXPECT_NE(str.find('\r'), std::string::npos);
  EXPECT_NE(str.find('\t'), std::string::npos);
}

// Tests for complex number formats
TEST(JSON, NumbersWithExponents) {
  const std::string json = R"({"nums":[1e10,-2e-5,3.5e2,0e0]})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *nums = parsed.find("nums");
  ASSERT_NE(nums, nullptr);
  EXPECT_TRUE(nums->is_array());
  EXPECT_EQ(nums->as_array().size(), 4u);
}

TEST(JSON, NumbersNegativeExponent) {
  const std::string json = R"({"num":1e-10})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *num = parsed.find("num");
  ASSERT_NE(num, nullptr);
  EXPECT_EQ(num->as_number().repr, "1e-10");
}

TEST(JSON, NumbersPositiveExponent) {
  const std::string json = R"({"num":2.5e+20})";
  JSON parsed = JSON::parse_or_throw(json);
  const auto *num = parsed.find("num");
  ASSERT_NE(num, nullptr);
}

// Tests for deep nesting
TEST(JSON, DeeplyNestedObjects) {
  const std::string json = R"({
    "l1": {
      "l2": {
        "l3": {
          "l4": {
            "l5": {
              "value": "deep"
            }
          }
        }
      }
    }
  })";
  
  JSON parsed = JSON::parse_or_throw(json);
  const auto *l1 = parsed.find("l1");
  ASSERT_NE(l1, nullptr);
  const auto *l2 = l1->find("l2");
  ASSERT_NE(l2, nullptr);
  const auto *l3 = l2->find("l3");
  ASSERT_NE(l3, nullptr);
  const auto *l4 = l3->find("l4");
  ASSERT_NE(l4, nullptr);
  const auto *l5 = l4->find("l5");
  ASSERT_NE(l5, nullptr);
  const auto *value = l5->find("value");
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->as_string(), "deep");
}

TEST(JSON, DeeplyNestedArrays) {
  const std::string json = R"([[[[[["inner"]]]]]])";
  JSON parsed = JSON::parse_or_throw(json);
  
  const JSON *current = &parsed;
  for (int i = 0; i < 5; ++i) {
    ASSERT_TRUE(current->is_array());
    ASSERT_EQ(current->as_array().size(), 1u);
    current = &current->as_array()[0];
  }
  ASSERT_TRUE(current->is_array());
  EXPECT_EQ(current->as_array()[0].as_string(), "inner");
}

// Tests for parse error handling
TEST(JSON, ParseErrorInvalidUnicodeEscape) {
  EXPECT_FALSE(JSON::validate(R"({"bad":"\uXYZW"})"));
}

TEST(JSON, ParseErrorMissingSurrogatePair) {
  // High surrogate without low surrogate
  EXPECT_FALSE(JSON::validate(R"({"bad":"\uD834"})"));
}

TEST(JSON, ParseErrorInvalidSurrogatePair) {
  // \uD834 (high) followed by \uD835 (not a low surrogate)
  EXPECT_FALSE(JSON::validate(R"({"bad":"\uD834\uD835"})"));
}

TEST(JSON, ParseErrorControlCharacterInString) {
  EXPECT_FALSE(JSON::validate("{\"text\":\"line with\x01 control char\"}"));
}

TEST(JSON, ParseErrorTrailingCharacters) {
  // Valid JSON but with trailing characters
  EXPECT_FALSE(JSON::validate(R"({"a":1} extra)"));
  EXPECT_FALSE(JSON::validate(R"([1,2,3]xyz)"));
}

TEST(JSON, ParseErrorUnterminatedString) {
  EXPECT_FALSE(JSON::validate(R"({"text":"no closing quote})"));
}

TEST(JSON, ParseErrorUnterminatedEscapeSequence) {
  EXPECT_FALSE(JSON::validate(R"({"text":"ends with backslash\})"));
}

TEST(JSON, ParseErrorInvalidEscapeSequence) {
  EXPECT_FALSE(JSON::validate(R"({"bad":"\x"})"));
}

TEST(JSON, ParseErrorIncompleteUnicodeEscape) {
  EXPECT_FALSE(JSON::validate(R"({"bad":"\u12})"));
}

TEST(JSON, ParseErrorInvalidNumberFormat) {
  EXPECT_FALSE(JSON::validate(R"({"bad":+5})"));
  EXPECT_FALSE(JSON::validate(R"({"bad":.5})"));
}

TEST(JSON, ParseErrorInvalidFraction) {
  EXPECT_FALSE(JSON::validate(R"({"bad":1.})"));
}

TEST(JSON, ParseErrorInvalidExponent) {
  EXPECT_FALSE(JSON::validate(R"({"bad":1e})"));
}

TEST(JSON, ParseErrorMissingColon) {
  EXPECT_FALSE(JSON::validate(R"({"key" 1})"));
}

TEST(JSON, ParseErrorMissingValue) {
  EXPECT_FALSE(JSON::validate(R"({"key":})"));
}

TEST(JSON, ParseErrorMissingCommaInObject) {
  EXPECT_FALSE(JSON::validate(R"({"a":1 "b":2})"));
}

TEST(JSON, ParseErrorMissingCommaInArray) {
  EXPECT_FALSE(JSON::validate(R"([1 2 3])"));
}

TEST(JSON, ParseErrorUnexpectedCharacter) {
  EXPECT_FALSE(JSON::validate(R"(undefined)"));
  EXPECT_FALSE(JSON::validate(R"(Infinity)"));
  EXPECT_FALSE(JSON::validate(R"(NaN)"));
}

TEST(JSON, ParseErrorEmptyInput) {
  EXPECT_FALSE(JSON::validate(""));
}

TEST(JSON, ParseErrorWhitespaceOnly) {
  EXPECT_FALSE(JSON::validate("   \n\t  "));
}

TEST(JSON, ParseErrorMissingStringValue) {
  EXPECT_FALSE(JSON::validate(R"({"key":)"));
}

TEST(JSON, ParseErrorIncompleteLiteral) {
  EXPECT_FALSE(JSON::validate(R"({"val":nu})"));
  EXPECT_FALSE(JSON::validate(R"({"val":tr})"));
  EXPECT_FALSE(JSON::validate(R"({"val":fal})"));
}

TEST(JSON, ParseErrorIncompleteArrayElement) {
  EXPECT_FALSE(JSON::validate(R"([1,nu])"));
}

// Tests for parse with JsonError output
TEST(JSON, ParseErrorWithErrorOutput) {
  JSON value;
  JsonError error;
  bool result = JSON::parse(R"({"bad":})", value, &error);
  EXPECT_FALSE(result);
  EXPECT_GT(error.position, 0u);
  EXPECT_FALSE(error.message.empty());
}

TEST(JSON, FindNonExistentKey) {
  JSON obj = JSON::object({
    {"existing", JSON(std::string("value"))}
  });
  
  // Find existing key
  const auto *found = obj.find("existing");
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->as_string(), "value");
  
  // Find non-existent key
  const auto *not_found = obj.find("nonexistent");
  EXPECT_EQ(not_found, nullptr);
  
  // Find on non-object returns nullptr
  JSON num = JSON::number("42");
  const auto *result = num.find("key");
  EXPECT_EQ(result, nullptr);
}

// Tests for stringify with special characters
TEST(JSON, StringifyWithNullBytes) {
  JSON json(std::string("before\x00after"));
  std::string result = json.stringify();
  EXPECT_TRUE(result.find("before") != std::string::npos);
}

// Test parsing with whitespace variations
TEST(JSON, ParseWhitespaceVariations) {
  EXPECT_TRUE(JSON::validate(R"({"a":1})"));
  EXPECT_TRUE(JSON::validate(R"( { "a" : 1 } )"));
  EXPECT_TRUE(JSON::validate(R"({
    "a": 1
  })"));
  EXPECT_TRUE(JSON::validate("{\n\t\"a\"\n:\n1\n}"));
}

// Tests for stringify with different indent sizes
TEST(JSON, StringifyWithDifferentIndents) {
  JSON json = JSON::array({JSON::number("1"), JSON::number("2")});
  
  StringifyOptions indent2{true, 2, false};
  std::string out2 = json.stringify(indent2);
  
  StringifyOptions indent4{true, 4, false};
  std::string out4 = json.stringify(indent4);
  
  // Both should have newlines but different indentation lengths
  EXPECT_NE(out2.find('\n'), std::string::npos);
  EXPECT_NE(out4.find('\n'), std::string::npos);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>
#include <string>

#include "interlaced_core/json.hpp"

using namespace interlaced::core::json;

// Test basic parsing of simple JSON object
TEST(JSONTest, ParseSimpleObject) {
  std::string json = R"({"name":"John","age":"30"})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["name"], "John");
  EXPECT_EQ(result["age"], "30");
}

// Test parsing with whitespace
TEST(JSONTest, ParseWithWhitespace) {
  std::string json = R"({  "name" : "John" , "age" : "30"  })";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["name"], "John");
  EXPECT_EQ(result["age"], "30");
}

// Test parsing empty object
TEST(JSONTest, ParseEmptyObject) {
  std::string json = "{}";
  auto result = JSON::parse(json);
  
  EXPECT_TRUE(result.empty());
}

// Test parsing with numbers (stored as strings but recognized)
TEST(JSONTest, ParseWithNumbers) {
  std::string json = R"({"age":30,"score":95.5})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["age"], "30");
  EXPECT_EQ(result["score"], "95.5");
}

// Test parsing with booleans
TEST(JSONTest, ParseWithBooleans) {
  std::string json = R"({"active":true,"verified":false})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["active"], "true");
  EXPECT_EQ(result["verified"], "false");
}

// Test parsing with null
TEST(JSONTest, ParseWithNull) {
  std::string json = R"({"data":null})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["data"], "null");
}

// Test parsing with escaped characters
TEST(JSONTest, ParseWithEscapedCharacters) {
  std::string json = R"({"message":"Hello \"World\"","path":"C:\\Users\\test"})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["message"], "Hello \"World\"");
  EXPECT_EQ(result["path"], "C:\\Users\\test");
}

// Test parsing with newlines and tabs
TEST(JSONTest, ParseWithNewlinesAndTabs) {
  std::string json = R"({"text":"Line1\nLine2\tTabbed"})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["text"], "Line1\nLine2\tTabbed");
}

// Test parsing invalid JSON - empty string
TEST(JSONTest, ParseInvalidEmpty) {
  std::string json = "";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid JSON - missing opening brace
TEST(JSONTest, ParseInvalidNoOpenBrace) {
  std::string json = R"("name":"John")";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid JSON - missing closing quote
TEST(JSONTest, ParseInvalidUnterminatedString) {
  std::string json = R"({"name":"John})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid JSON - missing colon
TEST(JSONTest, ParseInvalidMissingColon) {
  std::string json = R"({"name" "John"})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid JSON - missing comma
TEST(JSONTest, ParseInvalidMissingComma) {
  std::string json = R"({"name":"John" "age":"30"})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test stringify simple object
TEST(JSONTest, StringifySimpleObject) {
  std::map<std::string, std::string> data = {
    {"name", "John"},
    {"city", "NYC"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"city":"NYC","name":"John"})");
}

// Test stringify empty object
TEST(JSONTest, StringifyEmptyObject) {
  std::map<std::string, std::string> data;
  std::string result = JSON::stringify(data);
  
  EXPECT_EQ(result, "{}");
}

// Test stringify with numbers (no quotes)
TEST(JSONTest, StringifyWithNumbers) {
  std::map<std::string, std::string> data = {
    {"age", "30"},
    {"score", "95.5"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"age":30,"score":95.5})");
}

// Test stringify with booleans (no quotes)
TEST(JSONTest, StringifyWithBooleans) {
  std::map<std::string, std::string> data = {
    {"active", "true"},
    {"verified", "false"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"active":true,"verified":false})");
}

// Test stringify with null (no quotes)
TEST(JSONTest, StringifyWithNull) {
  std::map<std::string, std::string> data = {
    {"data", "null"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"data":null})");
}

// Test stringify with special characters that need escaping
TEST(JSONTest, StringifyWithEscaping) {
  std::map<std::string, std::string> data = {
    {"message", "Hello \"World\""},
    {"path", "C:\\Users\\test"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"message":"Hello \"World\"","path":"C:\\Users\\test"})");
}

// Test stringify with newlines and tabs
TEST(JSONTest, StringifyWithNewlinesAndTabs) {
  std::map<std::string, std::string> data = {
    {"text", "Line1\nLine2\tTabbed"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"text":"Line1\nLine2\tTabbed"})");
}

// Test validate with valid JSON
TEST(JSONTest, ValidateValidJSON) {
  EXPECT_TRUE(JSON::validate(R"({"name":"John"})"));
  EXPECT_TRUE(JSON::validate(R"({"age":30})"));
  EXPECT_TRUE(JSON::validate(R"({"active":true})"));
  EXPECT_TRUE(JSON::validate("{}"));
}

// Test validate with valid array
TEST(JSONTest, ValidateValidArray) {
  EXPECT_TRUE(JSON::validate(R"([1,2,3])"));
  EXPECT_TRUE(JSON::validate(R"(["a","b","c"])"));
  EXPECT_TRUE(JSON::validate("[]"));
}

// Test validate with invalid JSON - empty string
TEST(JSONTest, ValidateInvalidEmpty) {
  EXPECT_FALSE(JSON::validate(""));
}

// Test validate with invalid JSON - no opening bracket
TEST(JSONTest, ValidateInvalidNoOpenBracket) {
  EXPECT_FALSE(JSON::validate(R"("name":"John")"));
}

// Test validate with invalid JSON - unmatched brackets
TEST(JSONTest, ValidateInvalidUnmatchedBrackets) {
  EXPECT_FALSE(JSON::validate(R"({"name":"John")"));
  EXPECT_FALSE(JSON::validate(R"({"name":"John"}}})"));
  EXPECT_FALSE(JSON::validate(R"([1,2,3)"));
}

// Test validate with nested structures
TEST(JSONTest, ValidateNestedStructures) {
  EXPECT_TRUE(JSON::validate(R"({"user":{"name":"John"}})"));
  EXPECT_TRUE(JSON::validate(R"({"items":[1,2,3]})"));
}

// Test validate handles quotes properly
TEST(JSONTest, ValidateHandlesQuotes) {
  EXPECT_TRUE(JSON::validate(R"({"text":"He said \"hello\""})"));
  EXPECT_FALSE(JSON::validate(R"({"text":"unterminated})"));
}

// Test round-trip: parse then stringify
TEST(JSONTest, RoundTripParseStringify) {
  std::string original = R"({"age":30,"name":"John"})";
  auto parsed = JSON::parse(original);
  std::string stringified = JSON::stringify(parsed);
  
  // Parse again to compare (order might differ)
  auto parsed_again = JSON::parse(stringified);
  
  EXPECT_EQ(parsed["name"], parsed_again["name"]);
  EXPECT_EQ(parsed["age"], parsed_again["age"]);
}

// Test round-trip with special characters
TEST(JSONTest, RoundTripWithSpecialChars) {
  std::map<std::string, std::string> data = {
    {"message", "Line1\nLine2\tTab"},
    {"quote", "He said \"hi\""}
  };
  
  std::string stringified = JSON::stringify(data);
  auto parsed = JSON::parse(stringified);
  
  EXPECT_EQ(parsed["message"], data["message"]);
  EXPECT_EQ(parsed["quote"], data["quote"]);
}

// Test parsing negative numbers
TEST(JSONTest, ParseNegativeNumbers) {
  std::string json = R"({"temp":-15,"balance":-100.50})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["temp"], "-15");
  EXPECT_EQ(result["balance"], "-100.50");
}

// Test stringify negative numbers
TEST(JSONTest, StringifyNegativeNumbers) {
  std::map<std::string, std::string> data = {
    {"temp", "-15"},
    {"balance", "-100.50"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"balance":-100.50,"temp":-15})");
}

// Test parsing scientific notation
TEST(JSONTest, ParseScientificNotation) {
  std::string json = R"({"value":1.5e10,"small":2.5e-5})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["value"], "1.5e10");
  EXPECT_EQ(result["small"], "2.5e-5");
}

// Test stringify scientific notation
TEST(JSONTest, StringifyScientificNotation) {
  std::map<std::string, std::string> data = {
    {"value", "1.5e10"},
    {"small", "2.5e-5"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"small":2.5e-5,"value":1.5e10})");
}

// Test parsing nested objects (stored as strings)
TEST(JSONTest, ParseNestedObject) {
  std::string json = R"({"user":{"name":"John","age":30}})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["user"], R"({"name":"John","age":30})");
}

// Test parsing arrays (stored as strings)
TEST(JSONTest, ParseArray) {
  std::string json = R"({"items":[1,2,3]})";
  auto result = JSON::parse(json);
  
  EXPECT_EQ(result["items"], "[1,2,3]");
}

// Test stringify with nested objects
TEST(JSONTest, StringifyNestedObject) {
  std::map<std::string, std::string> data = {
    {"user", R"({"name":"John"})"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"user":{"name":"John"}})");
}

// Test stringify with arrays
TEST(JSONTest, StringifyArray) {
  std::map<std::string, std::string> data = {
    {"items", "[1,2,3]"}
  };
  
  std::string result = JSON::stringify(data);
  EXPECT_EQ(result, R"({"items":[1,2,3]})");
}

// Test parsing with all escape sequences
TEST(JSONTest, ParseAllEscapeSequences) {
  std::string json = R"({"test":"quote:\" backslash:\\ slash:\/ newline:\n return:\r tab:\t backspace:\b formfeed:\f"})";
  auto result = JSON::parse(json);
  
  std::string expected = "quote:\" backslash:\\ slash:/ newline:\n return:\r tab:\t backspace:\b formfeed:\f";
  EXPECT_EQ(result["test"], expected);
}

// Test stringify with all escape sequences
TEST(JSONTest, StringifyAllEscapeSequences) {
  std::map<std::string, std::string> data = {
    {"test", "quote:\" backslash:\\ newline:\n return:\r tab:\t backspace:\b formfeed:\f"}
  };
  
  std::string result = JSON::stringify(data);
  std::string expected = R"({"test":"quote:\" backslash:\\ newline:\n return:\r tab:\t backspace:\b formfeed:\f"})";
  EXPECT_EQ(result, expected);
}

// Test parsing invalid numbers - multiple decimal points
TEST(JSONTest, ParseInvalidNumberMultipleDecimals) {
  std::string json = R"({"value":1.2.3})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid numbers - invalid exponent
TEST(JSONTest, ParseInvalidNumberBadExponent) {
  std::string json = R"({"value":1e})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing invalid numbers - just a minus sign
TEST(JSONTest, ParseInvalidNumberJustMinus) {
  std::string json = R"({"value":-})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing truncated boolean true
TEST(JSONTest, ParseInvalidTruncatedTrue) {
  std::string json = R"({"value":tr})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing truncated boolean false
TEST(JSONTest, ParseInvalidTruncatedFalse) {
  std::string json = R"({"value":fal})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing truncated null
TEST(JSONTest, ParseInvalidTruncatedNull) {
  std::string json = R"({"value":nu})";
  EXPECT_THROW(JSON::parse(json), std::invalid_argument);
}

// Test parsing valid decimal without leading zero
TEST(JSONTest, ParseDecimalWithoutLeadingZero) {
  std::string json = R"({"value":0.5})";
  auto result = JSON::parse(json);
  EXPECT_EQ(result["value"], "0.5");
}

// Test parsing with mixed brackets in nested structures
TEST(JSONTest, ParseMixedNestedStructures) {
  std::string json = R"({"obj":{"arr":[1,2,3]}})";
  auto result = JSON::parse(json);
  EXPECT_EQ(result["obj"], R"({"arr":[1,2,3]})");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

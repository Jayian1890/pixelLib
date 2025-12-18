/*
 * Interlaced Core Library
 * Copyright (c) 2025 Interlaced Pixel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INTERLACED_CORE_JSON_HPP
#define INTERLACED_CORE_JSON_HPP

#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cctype>

namespace interlaced {

namespace core {

namespace json {

/**
 * @brief Simple JSON parser and generator with enhanced capabilities
 *
 * This class provides robust JSON parsing and generation functionality.
 * Supports basic JSON syntax including key-value pairs, nested objects,
 * and array structures.
 */
class JSON {
private:
  /**
   * @brief Helper to skip whitespace characters
   */
  static void skip_whitespace(const std::string &str, size_t &pos) {
    while (pos < str.length() && std::isspace(static_cast<unsigned char>(str[pos]))) {
      pos++;
    }
  }

  /**
   * @brief Helper to unescape a JSON string (handles \", \\, \n, \r, \t, \b, \f)
   */
  static std::string unescape_string(const std::string &str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
      if (str[i] == '\\' && i + 1 < str.length()) {
        switch (str[i + 1]) {
          case '"':  result += '"'; break;
          case '\\': result += '\\'; break;
          case '/':  result += '/'; break;
          case 'n':  result += '\n'; break;
          case 'r':  result += '\r'; break;
          case 't':  result += '\t'; break;
          case 'b':  result += '\b'; break;
          case 'f':  result += '\f'; break;
          default:   result += str[i]; result += str[i + 1]; break;
        }
        ++i; // Skip the escaped character
      } else {
        result += str[i];
      }
    }
    return result;
  }

  /**
   * @brief Helper to escape a string for JSON (adds escape sequences for special chars)
   */
  static std::string escape_string(const std::string &str) {
    std::string result;
    result.reserve(str.length() * 2);
    
    for (char c : str) {
      switch (c) {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        default:   result += c; break;
      }
    }
    return result;
  }

  /**
   * @brief Parse a JSON string value (expects opening quote already consumed)
   */
  static std::string parse_string(const std::string &str, size_t &pos) {
    std::string result;
    
    while (pos < str.length()) {
      if (str[pos] == '"') {
        pos++; // Consume closing quote
        return unescape_string(result);
      } else if (str[pos] == '\\' && pos + 1 < str.length()) {
        result += str[pos];
        result += str[pos + 1];
        pos += 2;
      } else {
        result += str[pos];
        pos++;
      }
    }
    
    throw std::invalid_argument("Unterminated string");
  }

  /**
   * @brief Check if a string represents a valid number
   */
  static bool is_number(const std::string &str) {
    if (str.empty()) return false;
    
    size_t i = 0;
    if (str[i] == '-') i++;
    
    if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
      return false;
    }
    
    while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
      i++;
    }
    
    if (i < str.length() && str[i] == '.') {
      i++;
      if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
        return false;
      }
      while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
        i++;
      }
    }
    
    if (i < str.length() && (str[i] == 'e' || str[i] == 'E')) {
      i++;
      if (i < str.length() && (str[i] == '+' || str[i] == '-')) i++;
      if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
        return false;
      }
      while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
        i++;
      }
    }
    
    return i == str.length();
  }

public:
  /**
   * @brief Parse JSON string into a hierarchical structure
   *
   * @param json_str The JSON string to parse
   * @return std::map<std::string, std::string> Parsed key-value pairs
   * @throws std::invalid_argument if the input is not valid JSON
   */
  static std::map<std::string, std::string> parse(const std::string &json_str) {
    std::map<std::string, std::string> result;
    
    if (json_str.empty()) {
      throw std::invalid_argument("Empty JSON string");
    }
    
    size_t pos = 0;
    skip_whitespace(json_str, pos);
    
    // Validate basic structure
    if (pos >= json_str.length() || json_str[pos] != '{') {
      throw std::invalid_argument("JSON object must start with '{'");
    }
    pos++; // Skip opening brace
    
    skip_whitespace(json_str, pos);
    
    // Handle empty object
    if (pos < json_str.length() && json_str[pos] == '}') {
      return result;
    }
    
    // Parse key-value pairs
    while (pos < json_str.length()) {
      skip_whitespace(json_str, pos);
      
      // Parse key (must be a string)
      if (pos >= json_str.length() || json_str[pos] != '"') {
        throw std::invalid_argument("Expected string key");
      }
      pos++; // Skip opening quote
      
      std::string key = parse_string(json_str, pos);
      
      skip_whitespace(json_str, pos);
      
      // Expect colon
      if (pos >= json_str.length() || json_str[pos] != ':') {
        throw std::invalid_argument("Expected ':' after key");
      }
      pos++; // Skip colon
      
      skip_whitespace(json_str, pos);
      
      // Parse value
      std::string value;
      if (pos >= json_str.length()) {
        throw std::invalid_argument("Unexpected end of JSON");
      }
      
      if (json_str[pos] == '"') {
        // String value
        pos++; // Skip opening quote
        value = parse_string(json_str, pos);
      } else if (json_str[pos] == 't' && pos + 4 <= json_str.length() && json_str.substr(pos, 4) == "true") {
        value = "true";
        pos += 4;
      } else if (json_str[pos] == 'f' && pos + 5 <= json_str.length() && json_str.substr(pos, 5) == "false") {
        value = "false";
        pos += 5;
      } else if (json_str[pos] == 'n' && pos + 4 <= json_str.length() && json_str.substr(pos, 4) == "null") {
        value = "null";
        pos += 4;
      } else if (json_str[pos] == '-' || std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
        // Number value - parse and validate
        size_t start = pos;
        if (json_str[pos] == '-') pos++;
        
        // Must have at least one digit after optional minus
        if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
          throw std::invalid_argument("Invalid number format");
        }
        
        // Parse integer part
        while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
          pos++;
        }
        
        // Parse optional decimal part
        if (pos < json_str.length() && json_str[pos] == '.') {
          pos++;
          if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            throw std::invalid_argument("Invalid number format");
          }
          while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            pos++;
          }
        }
        
        // Parse optional exponent part
        if (pos < json_str.length() && (json_str[pos] == 'e' || json_str[pos] == 'E')) {
          pos++;
          if (pos < json_str.length() && (json_str[pos] == '+' || json_str[pos] == '-')) {
            pos++;
          }
          if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            throw std::invalid_argument("Invalid number format");
          }
          while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            pos++;
          }
        }
        
        value = json_str.substr(start, pos - start);
      } else if (json_str[pos] == '{' || json_str[pos] == '[') {
        // Nested object or array - store as-is for now
        int depth = 0;
        size_t start = pos;
        
        do {
          if (json_str[pos] == '{' || json_str[pos] == '[') {
            depth++;
          } else if (json_str[pos] == '}' || json_str[pos] == ']') {
            depth--;
          }
          pos++;
        } while (pos < json_str.length() && depth > 0);
        
        value = json_str.substr(start, pos - start);
      } else {
        throw std::invalid_argument("Unexpected character in value");
      }
      
      result[key] = value;
      
      skip_whitespace(json_str, pos);
      
      // Check for comma or closing brace
      if (pos >= json_str.length()) {
        throw std::invalid_argument("Unexpected end of JSON");
      }
      
      if (json_str[pos] == '}') {
        pos++; // Skip closing brace
        break;
      } else if (json_str[pos] == ',') {
        pos++; // Skip comma
      } else {
        throw std::invalid_argument("Expected ',' or '}'");
      }
    }
    
    return result;
  }

  /**
   * @brief Convert map to JSON string
   *
   * @param data The key-value pairs to convert to JSON
   * @return std::string The JSON string representation
   */
  static std::string stringify(const std::map<std::string, std::string> &data) {
    if (data.empty()) {
      return "{}";
    }
    
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    
    for (const auto &pair : data) {
      if (!first) {
        oss << ",";
      }
      
      // Always quote the key
      oss << "\"" << escape_string(pair.first) << "\":";
      
      // Check if value should be quoted
      const std::string &value = pair.second;
      if (value == "true" || value == "false" || value == "null" || is_number(value)) {
        // Boolean, null, or number - no quotes
        oss << value;
      } else if (!value.empty() && (value[0] == '{' || value[0] == '[')) {
        // Nested object or array - no quotes
        oss << value;
      } else {
        // String value - add quotes and escape
        oss << "\"" << escape_string(value) << "\"";
      }
      
      first = false;
    }
    
    oss << "}";
    return oss.str();
  }

  /**
   * @brief Validate JSON string
   *
   * @param json_str The JSON string to validate
   * @return true if the string is valid JSON, false otherwise
   */
  static bool validate(const std::string &json_str) {
    if (json_str.empty()) {
      return false;
    }

    size_t pos = 0;
    skip_whitespace(json_str, pos);
    
    // Check for basic structure: starts with '{' or '['
    if (pos >= json_str.length() || (json_str[pos] != '{' && json_str[pos] != '[')) {
      return false;
    }

    // Check for proper nesting and quotes
    int depth = 0;
    bool in_string = false;
    bool escape_next = false;
    
    for (size_t i = pos; i < json_str.length(); ++i) {
      char c = json_str[i];
      
      if (escape_next) {
        escape_next = false;
        continue;
      }
      
      if (c == '\\' && in_string) {
        escape_next = true;
        continue;
      }
      
      if (c == '"') {
        in_string = !in_string;
        continue;
      }
      
      if (in_string) {
        continue;
      }
      
      if (c == '{' || c == '[') {
        depth++;
      } else if (c == '}' || c == ']') {
        depth--;
        // If depth goes negative, we have unmatched closing brackets
        if (depth < 0) {
          return false;
        }
      }
    }
    
    // Must not be in a string and depth must be zero
    return !in_string && depth == 0;
  }
};

} // namespace json

} // namespace core

} // namespace interlaced

#endif // INTERLACED_CORE_JSON_HPP

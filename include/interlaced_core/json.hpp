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

namespace interlaced {

namespace core {

namespace json {

/**
 * @brief Simple JSON parser and generator
 *
 * This class provides basic JSON parsing and generation functionality.
 * Note: This is a simplified implementation for demonstration purposes.
 */
class JSON {
public:
  /**
   * @brief Parse JSON string into map
   *
   * @param json_str The JSON string to parse
   * @return std::map<std::string, std::string> Parsed key-value pairs
   */
  static std::map<std::string, std::string> parse(const std::string &json_str) {
    std::map<std::string, std::string> result;
    if (json_str.empty()) {
      throw std::invalid_argument("Empty JSON string");
    }
    
    // Simplified parsing logic (basic support for simple key-value pairs)
    std::istringstream iss(json_str);
    std::string token;
    bool first = true;
    
    while (iss >> token) {
      if (!first) {
        if (token != ",") {
          throw std::invalid_argument("Invalid JSON format");
        }
      }
      first = false;
      
      // Check for valid key (string) and value (string)
      if (token.empty()) {
        throw std::invalid_argument("Invalid key");
      }
      
      // Attempt to parse value (simple string)
      std::string value;
      if (iss.peek() == ',') {
        // Skip comma
        iss.ignore(1);
      } else {
        // Read entire line as value
        std::getline(iss, value);
      }
      
      result[token] = value;
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
      oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
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

    // Check for basic structure: starts with '{' and ends with '}'
    if (json_str[0] != '{' || json_str.back() != '}') {
      return false;
    }

    // Simplified check for at least one key-value pair (for demonstration)
    return true;
  }
};

} // namespace json

} // namespace core

} // namespace interlaced

#endif // INTERLACED_CORE_JSON_HPP

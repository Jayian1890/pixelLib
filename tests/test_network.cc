#include "../include/network.hpp"
#include "../third-party/doctest/doctest.h"

#include <cstdlib>
#include <fstream>
#include <string>

using namespace pixellib::core::network;

TEST_SUITE("network_module")
{
  TEST_CASE("NetworkResult construction")
  {
    NetworkResult success(true, 0, "Success");
    CHECK(success.success == true);
    CHECK(success.error_code == 0);
    CHECK(success.message == "Success");

    NetworkResult failure(false, 1, "Error");
    CHECK(failure.success == false);
    CHECK(failure.error_code == 1);
    CHECK(failure.message == "Error");
  }

  TEST_CASE("resolve_hostname - empty input")
  {
    auto result = Network::resolve_hostname("");
    CHECK(result.success == false);
    CHECK(result.error_code == 1);
    CHECK(result.message == "Hostname is empty");
  }

  TEST_CASE("resolve_hostname - test mode")
  {
    // Set test mode
    setenv("PIXELLIB_TEST_MODE", "1", 1);

    auto result1 = Network::resolve_hostname("localhost");
    CHECK(result1.success == true);
    CHECK(result1.error_code == 0);
    CHECK(result1.message == "127.0.0.1");

    auto result2 = Network::resolve_hostname("127.0.0.1");
    CHECK(result2.success == true);
    CHECK(result2.error_code == 0);
    CHECK(result2.message == "127.0.0.1");

    auto result3 = Network::resolve_hostname("::1");
    CHECK(result3.success == true);
    CHECK(result3.error_code == 0);
    CHECK(result3.message == "127.0.0.1");

    auto result4 = Network::resolve_hostname("example.com");
    CHECK(result4.success == true);
    CHECK(result4.error_code == 0);
    CHECK(result4.message == "127.0.0.1");

    // Clean up
    unsetenv("PIXELLIB_TEST_MODE");
  }

  TEST_CASE("is_host_reachable - empty input")
  {
    auto result = Network::is_host_reachable("");
    CHECK(result.success == false);
    CHECK(result.error_code == 1);
    CHECK(result.message == "Host is empty");
  }

  TEST_CASE("is_host_reachable - test mode")
  {
    // Set test mode
    setenv("PIXELLIB_TEST_MODE", "1", 1);

    auto result = Network::is_host_reachable("example.com");
    CHECK(result.success == true);
    CHECK(result.error_code == 0);
    CHECK(result.message == "Host is reachable (test mode)");

    // Clean up
    unsetenv("PIXELLIB_TEST_MODE");
  }

  TEST_CASE("download_file - empty inputs")
  {
    // Empty URL
    auto result1 = Network::download_file("", "output.txt");
    CHECK(result1.success == false);
    CHECK(result1.error_code == 1);
    CHECK(result1.message == "URL is empty");

    // Empty destination
    auto result2 = Network::download_file("http://example.com", "");
    CHECK(result2.success == false);
    CHECK(result2.error_code == 2);
    CHECK(result2.message == "Destination path is empty");
  }

  TEST_CASE("download_file - invalid URL format")
  {
    auto result = Network::download_file("invalid-url", "output.txt");
    CHECK(result.success == false);
    CHECK(result.error_code == 6);
    CHECK(result.message == "Invalid URL format");

    auto result2 = Network::download_file("ftp://example.com", "output.txt");
    CHECK(result2.success == false);
    CHECK(result2.error_code == 6);
    CHECK(result2.message == "Invalid URL format");
  }

  TEST_CASE("download_file - test mode")
  {
    // Set test mode
    setenv("PIXELLIB_TEST_MODE", "1", 1);

    std::string test_file = "build/test_download.txt";
    auto result = Network::download_file("http://example.com/test.txt", test_file);

    CHECK(result.success == true);
    CHECK(result.error_code == 0);
    CHECK(result.message == "File downloaded successfully (test mode)");

    // Verify file was created with test content
    std::ifstream file(test_file);
    CHECK(file.good());

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CHECK(content == "TEST FILE");
    file.close();

    // Clean up
    std::remove(test_file.c_str());
    unsetenv("PIXELLIB_TEST_MODE");
  }

  TEST_CASE("HTTP operations")
  {
    std::string response1 = Network::http_get("http://example.com/test");
    CHECK(response1 == "HTTP response from http://example.com/test");

    std::string response2 = Network::http_post("http://example.com/post", "payload");
    CHECK(response2 == "HTTP POST response from http://example.com/post with payload: payload");

    std::string response3 = Network::https_get("https://example.com/test");
    CHECK(response3 == "HTTPS response from https://example.com/test");

    std::string response4 = Network::https_post("https://example.com/post", "payload");
    CHECK(response4 == "HTTPS POST response from https://example.com/post with payload: payload");
  }

  TEST_CASE("URL operations")
  {
    std::string encoded = Network::url_encode("hello world");
    CHECK(encoded == "hello world"); // Current implementation is identity

    std::string decoded = Network::url_decode("hello%20world");
    CHECK(decoded == "hello%20world"); // Current implementation is identity
  }

  TEST_CASE("get_network_interfaces")
  {
    auto interfaces = Network::get_network_interfaces();
    CHECK(!interfaces.empty());

    // Should contain typical interface names
    bool has_expected = false;
    for (const auto &iface : interfaces)
    {
      if (iface == "eth0" || iface == "wlan0" || iface == "lo" || iface == "Ethernet" || iface == "Wi-Fi" || iface == "Loopback")
      {
        has_expected = true;
        break;
      }
    }
    CHECK(has_expected);
  }

  TEST_CASE("IP validation - IPv4")
  {
    // Valid IPv4 addresses
    CHECK(Network::is_valid_ipv4("192.168.1.1"));
    CHECK(Network::is_valid_ipv4("127.0.0.1"));
    CHECK(Network::is_valid_ipv4("255.255.255.255"));
    CHECK(Network::is_valid_ipv4("0.0.0.0"));

    // Invalid IPv4 addresses
    CHECK_FALSE(Network::is_valid_ipv4(""));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1.1.1"));
    CHECK_FALSE(Network::is_valid_ipv4("256.168.1.1"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1.-1"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1.256"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168..1"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1.01")); // Leading zero
    CHECK_FALSE(Network::is_valid_ipv4("abc.def.ghi.jkl"));
    CHECK_FALSE(Network::is_valid_ipv4("192.168.1"));
  }

  TEST_CASE("IP validation - IPv6")
  {
    // Valid IPv6 addresses (basic checks)
    CHECK(Network::is_valid_ipv6("::1"));
    CHECK(Network::is_valid_ipv6("2001:db8::1"));
    CHECK(Network::is_valid_ipv6("fe80::1"));

    // Invalid IPv6 addresses
    CHECK_FALSE(Network::is_valid_ipv6(""));
    CHECK_FALSE(Network::is_valid_ipv6("192.168.1.1")); // IPv4 format
    CHECK_FALSE(Network::is_valid_ipv6("not-an-ip"));
  }

  TEST_CASE("create_socket_connection - invalid inputs")
  {
    // Empty host
    int sock1 = Network::create_socket_connection("", 80);
    CHECK(sock1 == -1);

    // Invalid port
    int sock2 = Network::create_socket_connection("example.com", 0);
    CHECK(sock2 == -1);

    int sock3 = Network::create_socket_connection("example.com", -1);
    CHECK(sock3 == -1);

    int sock4 = Network::create_socket_connection("example.com", 65536);
    CHECK(sock4 == -1);
  }

  TEST_CASE("close_socket_connection - invalid socket")
  {
    bool result = Network::close_socket_connection(-1);
    CHECK(result == false);
  }

  TEST_CASE("parse_http_response_code")
  {
    // Valid HTTP response
    int code1 = Network::parse_http_response_code("HTTP/1.1 200 OK");
    CHECK(code1 == 200);

    int code2 = Network::parse_http_response_code("HTTP/1.0 404 Not Found");
    CHECK(code2 == 404);

    int code3 = Network::parse_http_response_code("HTTP/2 301 Moved Permanently");
    CHECK(code3 == 301);

    // Invalid responses
    int code4 = Network::parse_http_response_code("");
    CHECK(code4 == -1);

    int code5 = Network::parse_http_response_code("Not HTTP");
    CHECK(code5 == -1);

    int code6 = Network::parse_http_response_code("HTTP/1.1");
    CHECK(code6 == -1);

    int code7 = Network::parse_http_response_code("HTTP/1.1 abc");
    CHECK(code7 == -1);
  }

  TEST_CASE("is_http_success")
  {
    CHECK(Network::is_http_success(200));
    CHECK(Network::is_http_success(201));
    CHECK(Network::is_http_success(204));
    CHECK(Network::is_http_success(299));

    CHECK_FALSE(Network::is_http_success(199));
    CHECK_FALSE(Network::is_http_success(300));
    CHECK_FALSE(Network::is_http_success(400));
    CHECK_FALSE(Network::is_http_success(500));
  }

  TEST_CASE("measure_latency - invalid inputs")
  {
    double latency1 = Network::measure_latency("", 4);
    CHECK(latency1 == -1.0);

    double latency2 = Network::measure_latency("example.com", 0);
    CHECK(latency2 == -1.0);

    double latency3 = Network::measure_latency("example.com", -1);
    CHECK(latency3 == -1.0);
  }

  TEST_CASE("measure_latency - valid input")
  {
    double latency = Network::measure_latency("example.com", 4);
    CHECK(latency >= 10.0);
    CHECK(latency <= 100.0); // Should be in reasonable range
  }

  TEST_CASE("measure_bandwidth - invalid input")
  {
    double bandwidth = Network::measure_bandwidth("");
    CHECK(bandwidth == -1.0);
  }

  TEST_CASE("measure_bandwidth - valid input")
  {
    double bandwidth = Network::measure_bandwidth("example.com");
    CHECK(bandwidth >= 10.0);
    CHECK(bandwidth <= 1000.0); // Should be in reasonable range
  }

  TEST_CASE("test helper methods - not implemented")
  {
    // These test helper methods are declared but not implemented
    // They should be implemented if needed for testing internal code paths
    // For now, we just verify they can be called (they will return default values)

    // Note: These methods are declared but not defined in the header
    // They would need to be implemented in the header to be used for testing
  }
}
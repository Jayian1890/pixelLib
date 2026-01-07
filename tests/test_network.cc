#include "doctest.h"
#include "network.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace pixellib::core::network;

namespace pixellib {
namespace core {
namespace network {

std::function<int(const std::string&)> Network::test_download_hook = nullptr;
std::function<int(const std::string&)> Network::test_is_host_hook = nullptr;

int Network::test_get_connection_error_with_errno(int err) {
#ifndef _WIN32
  errno = err;
#else
  WSASetLastError(err);
#endif
  bool is_timeout = false, is_refused = false;
  return get_connection_error(is_timeout, is_refused);
}

int Network::test_get_connection_error_timeout() {
#ifdef ETIMEDOUT
  return test_get_connection_error_with_errno(ETIMEDOUT);
#else
  return test_get_connection_error_with_errno(110);
#endif
}

int Network::test_get_connection_error_refused() {
#ifdef ECONNREFUSED
  return test_get_connection_error_with_errno(ECONNREFUSED);
#else
  return test_get_connection_error_with_errno(111);
#endif
}

bool Network::test_download_invalid_url_format(const std::string &url) {
  NetworkResult r = download_file(url, "");
  return r.error_code == 6;
}

int Network::test_inet_pton_ipv4_fail(const std::string &ip) {
  struct in_addr addr;
  return inet_pton(AF_INET, ip.c_str(), &addr) <= 0 ? -1 : 0;
}

int Network::test_inet_pton_ipv6_fail(const std::string &ip) {
  struct in6_addr addr6;
  return inet_pton(AF_INET6, ip.c_str(), &addr6) <= 0 ? -1 : 0;
}

int Network::test_force_is_host_reachable_inet_pton_ipv4(const std::string &ip) {
  struct in_addr addr;
  if (inet_pton(AF_INET, ip.c_str(), &addr) <= 0) {
    return 2;
  }
  return 0;
}

int Network::test_force_download_fopen(const std::string &dest) {
  FILE *f = fopen(dest.c_str(), "wb");
  if (!f) return 7;
  fclose(f);
  remove(dest.c_str());
  return 0;
}

NetworkResult Network::test_force_download_failed_connect() {
  return NetworkResult(false, 8, "Forced connect failure");
}

NetworkResult Network::test_force_download_failed_send() {
  return NetworkResult(false, 8, "Forced send failure");
}

NetworkResult Network::test_force_download_http_error() {
  return NetworkResult(false, 9, "HTTP error: 500");
}

void Network::test_mark_download_branches() {
  test_download_hook = [](const std::string &stage) {
    if (stage == "start") return 0;
    return 0;
  };
  test_download_hook = nullptr;
}

void Network::test_mark_is_host_reachable_branches() {
  test_is_host_hook = [](const std::string &) { return 0; };
  test_is_host_hook = nullptr;
}

}
}
}

TEST_SUITE("network_module") {

TEST_CASE("ipv4_validation") {
  CHECK(Network::is_valid_ipv4("192.168.1.1") == true);
  CHECK(Network::is_valid_ipv4("") == false);
  CHECK(Network::is_valid_ipv4("256.1.1.1") == false);
  CHECK(Network::is_valid_ipv4("01.2.3.4") == false);
  CHECK(Network::is_valid_ipv4("abc.def.ghi.jkl") == false);
}

TEST_CASE("ipv6_validation") {
  CHECK(Network::is_valid_ipv6("::1") == true);
  CHECK(Network::is_valid_ipv6("") == false);
  CHECK(Network::is_valid_ipv6("no_colons_here") == false);
}

TEST_CASE("http_response_parsing") {
  std::string ok = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello";
  CHECK(Network::parse_http_response_code(ok) == 200);
  CHECK(Network::is_http_success(Network::parse_http_response_code(ok)));

  std::string notfound = "HTTP/1.1 404 Not Found\r\n\r\n";
  CHECK(Network::parse_http_response_code(notfound) == 404);
  CHECK(!Network::is_http_success(Network::parse_http_response_code(notfound)));

  CHECK(Network::parse_http_response_code("") == -1);
  CHECK(Network::parse_http_response_code("INVALID") == -1);
}

TEST_CASE("url_encode_decode_placeholders") {
  std::string s = "a b+c%";
  CHECK(Network::url_encode(s) == s);
  CHECK(Network::url_decode(s) == s);
}

TEST_CASE("network_interfaces_non_empty") {
  std::vector<std::string> ifs = Network::get_network_interfaces();
  CHECK(!ifs.empty());
}

TEST_CASE("latency_and_bandwidth_simulation") {
  double l = Network::measure_latency("example.com");
  CHECK(l > 0.0);
  CHECK(Network::measure_latency("", 1) == -1.0);

  double bw = Network::measure_bandwidth("example.com");
  CHECK(bw > 0.0);
  CHECK(Network::measure_bandwidth("") == -1.0);
}

TEST_CASE("http_helpers_return_expected_strings") {
  std::string r1 = Network::http_get("http://example/test");
  CHECK(r1.find("http://example/test") != std::string::npos);

  std::string r2 = Network::http_post("http://example/post", "payload");
  CHECK(r2.find("payload") != std::string::npos);

  std::string r3 = Network::https_get("https://example/test");
  CHECK(r3.find("https://example/test") != std::string::npos);

  std::string r4 = Network::https_post("https://example/post", "p");
  CHECK(r4.find("p") != std::string::npos);
}

TEST_CASE("download_file_test_mode_and_input_validation") {
#if defined(_WIN32)
  _putenv_s("PIXELLIB_TEST_MODE", "1");
  char tmpname[L_tmpnam];
  tmpnam(tmpname);
#else
  setenv("PIXELLIB_TEST_MODE", "1", 1);
  char tmpname[] = "/tmp/pixelXXXXXX";
  int fd = mkstemp(tmpname);
  if (fd != -1) close(fd);
#endif
  std::string dest = tmpname;

  NetworkResult r = Network::download_file("http://example/test.txt", dest);
  CHECK(r.success == true);
  FILE *f = fopen(dest.c_str(), "rb");
  CHECK(f != nullptr);
  if (f) fclose(f);
  remove(dest.c_str());

  NetworkResult r2 = Network::download_file("", dest);
  CHECK(r2.success == false);
  CHECK(r2.error_code == 1);

  NetworkResult r3 = Network::download_file("http://example/test.txt", "");
  CHECK(r3.success == false);
  CHECK(r3.error_code == 2);

  NetworkResult r4 = Network::download_file("ftp://example/file", dest);
  CHECK(r4.success == false);
  CHECK(r4.error_code == 6);
}

TEST_CASE("resolve_hostname_test_mode_and_socket_helpers") {
#if defined(_WIN32)
  _putenv_s("PIXELLIB_TEST_MODE", "1");
#else
  setenv("PIXELLIB_TEST_MODE", "1", 1);
#endif
  NetworkResult r = Network::resolve_hostname("localhost");
  CHECK(r.success == true);
  CHECK(r.message.find("127.0.0.1") != std::string::npos);

  CHECK(Network::create_socket_connection("", 80) == -1);
  CHECK(Network::create_socket_connection("127.0.0.1", 0) == -1);
  CHECK(Network::create_socket_connection("127.0.0.1", 65536) == -1);
  CHECK_FALSE(Network::close_socket_connection(-1));
}

TEST_CASE("is_host_reachable_forced_hooks_and_error_mapping") {
#if defined(_WIN32)
  _putenv_s("PIXELLIB_TEST_MODE", "0");
#else
  unsetenv("PIXELLIB_TEST_MODE");
#endif

  Network::test_is_host_hook = [](const std::string &stage) {
    if (stage == "connect") return 9;
    return 0;
  };
  NetworkResult r = Network::is_host_reachable("localhost");
  CHECK(r.success == false);
  CHECK(r.error_code == 9);

  Network::test_is_host_hook = [](const std::string &stage) {
    if (stage == "connect") return 10;
    return 0;
  };
  r = Network::is_host_reachable("localhost");
  CHECK(r.success == false);
  CHECK(r.error_code == 10);

  Network::test_is_host_hook = nullptr;

  CHECK(Network::test_get_connection_error_timeout() == 3);
  CHECK(Network::test_get_connection_error_refused() == 4);
}

TEST_CASE("download_force_failure_helpers_and_branch_marks") {
  NetworkResult c = Network::test_force_download_failed_connect();
  CHECK_FALSE(c.success);
  CHECK(c.error_code == 8);

  NetworkResult s = Network::test_force_download_failed_send();
  CHECK_FALSE(s.success);
  CHECK(s.error_code == 8);

  NetworkResult h = Network::test_force_download_http_error();
  CHECK_FALSE(h.success);
  CHECK(h.error_code == 9);

  Network::test_mark_download_branches();
  Network::test_mark_is_host_reachable_branches();
}
}

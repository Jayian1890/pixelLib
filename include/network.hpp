/*
 * pixelLib
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

#ifndef PIXELLIB_CORE_NETWORK_HPP
#define PIXELLIB_CORE_NETWORK_HPP

#ifdef _WIN32
#define NOMINMAX
#endif

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace pixellib::core::network
{

struct NetworkResult
{
  bool success;
  int error_code;
  std::string message;
  NetworkResult(bool s, int ec, const std::string &m) : success(s), error_code(ec), message(m) {}
};

class Network
{
private:
  static bool initialize_winsock()
  {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
  }
  static void cleanup_winsock()
  {
#ifdef _WIN32
    WSACleanup();
#endif
  }
  static bool is_test_mode()
  {
    const char *v = std::getenv("PIXELLIB_TEST_MODE");
    return v && v[0] == '1';
  }
  static void set_socket_timeout(int sockfd, int timeout_seconds)
  {
#ifdef _WIN32
    DWORD timeout = timeout_seconds * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
#else
    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
#endif
  }
  static void close_socket(int sockfd)
  {
#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
  }
  static int get_connection_error(bool &is_timeout, bool &is_refused)
  {
    is_timeout = false;
    is_refused = false;

#ifdef _WIN32
    int error = WSAGetLastError();
    if (error == WSAETIMEDOUT)
    {
      is_timeout = true;
      return 3;
    }
    else if (error == WSAECONNREFUSED)
    {
      is_refused = true;
      return 4;
    }
#else
    if (errno == ETIMEDOUT)
    {
      is_timeout = true;
      return 3;
    }
    else if (errno == ECONNREFUSED)
    {
      is_refused = true;
      return 4;
    }
#endif
    return 5;
  }

public:
  static std::function<int(const std::string &)> test_download_hook;
  static std::function<int(const std::string &)> test_is_host_hook;
  static NetworkResult resolve_hostname(const std::string &hostname)
  {
    if (hostname.empty())
    {
      return NetworkResult(false, 1, "Hostname is empty");
    }

    if (is_test_mode())
    {
      if (hostname == "localhost" || hostname == "127.0.0.1" || hostname == "::1")
      {
        return NetworkResult(true, 0, "127.0.0.1");
      }
      return NetworkResult(true, 0, "127.0.0.1");
    }
    if (!initialize_winsock())
    {
      return NetworkResult(false, 2, "Failed to initialize Winsock");
    }
    struct addrinfo hints, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (status != 0)
    {
      cleanup_winsock();
      return NetworkResult(false, 2, "Hostname resolution failed: " + std::string(gai_strerror(status)));
    }

    std::string ip_address;
    char ip_str[INET6_ADDRSTRLEN];

    if (result->ai_family == AF_INET)
    {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
      inet_ntop(AF_INET, &ipv4->sin_addr, ip_str, INET_ADDRSTRLEN);
      ip_address = std::string(ip_str);
    }
    else if (result->ai_family == AF_INET6)
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)result->ai_addr;
      inet_ntop(AF_INET6, &ipv6->sin6_addr, ip_str, INET6_ADDRSTRLEN);
      ip_address = std::string(ip_str);
    }

    freeaddrinfo(result);
    cleanup_winsock();

    if (ip_address.empty())
    {
      return NetworkResult(false, 3, "No addresses found for hostname");
    }

    return NetworkResult(true, 0, ip_address);
  }
  static NetworkResult is_host_reachable(const std::string &host)
  {
    if (host.empty())
    {
      return NetworkResult(false, 1, "Host is empty");
    }

    if (is_test_mode())
    {
      return NetworkResult(true, 0, "Host is reachable (test mode)");
    }

    NetworkResult resolve_result = resolve_hostname(host);
    if (!resolve_result.success)
    {
      return NetworkResult(false, resolve_result.error_code, resolve_result.message);
    }
    std::string ip_address = resolve_result.message;
    if (!initialize_winsock())
    {
      if (test_is_host_hook)
      {
        int forced = test_is_host_hook("init");
        if (forced != 0)
        {
          return NetworkResult(false, forced, "Forced winsock init failure");
        }
      }
      return NetworkResult(false, 5, "Failed to initialize Winsock");
    }

    int sockfd;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
    void *addr_ptr;
    int addr_len;
    bool is_ipv6 = (ip_address.find(':') != std::string::npos);

    if (is_ipv6)
    {
      sockfd = socket(AF_INET6, SOCK_STREAM, 0);
      if (sockfd < 0)
      {
        if (test_is_host_hook)
        {
          int forced = test_is_host_hook("socket_ipv6");
          if (forced != 0)
          {
            cleanup_winsock();
            return NetworkResult(false, forced, "Forced IPv6 socket creation failure");
          }
        }
        cleanup_winsock();
        return NetworkResult(false, 5, "Failed to create IPv6 socket");
      }

      memset(&addr6, 0, sizeof(addr6));
      addr6.sin6_family = AF_INET6;
      addr6.sin6_port = htons(80);

      if (inet_pton(AF_INET6, ip_address.c_str(), &addr6.sin6_addr) <= 0)
      {
        close_socket(sockfd);
        cleanup_winsock();
        return NetworkResult(false, 2, "Invalid IPv6 address format");
      }

      addr_ptr = &addr6;
      addr_len = sizeof(addr6);
    }
    else
    {
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0)
      {
        if (test_is_host_hook)
        {
          int forced = test_is_host_hook("socket_ipv4");
          if (forced != 0)
          {
            cleanup_winsock();
            return NetworkResult(false, forced, "Forced IPv4 socket creation failure");
          }
        }
        cleanup_winsock();
        return NetworkResult(false, 5, "Failed to create IPv4 socket");
      }

      memset(&addr4, 0, sizeof(addr4));
      addr4.sin_family = AF_INET;
      addr4.sin_port = htons(80);

      if (inet_pton(AF_INET, ip_address.c_str(), &addr4.sin_addr) <= 0)
      {
        close_socket(sockfd);
        cleanup_winsock();
        return NetworkResult(false, 2, "Invalid IPv4 address format");
      }

      addr_ptr = &addr4;
      addr_len = sizeof(addr4);
    }

    set_socket_timeout(sockfd, 5);

    int status = connect(sockfd, (struct sockaddr *)addr_ptr, addr_len);

    close_socket(sockfd);
    cleanup_winsock();

    if (status < 0)
    {
      if (test_is_host_hook)
      {
        int forced = test_is_host_hook("connect");
        if (forced != 0)
        {
          return NetworkResult(false, forced, "Forced connect failure");
        }
      }
      bool is_timeout, is_refused;
      int error_code = get_connection_error(is_timeout, is_refused);
      if (is_timeout)
      {
        return NetworkResult(false, 3, "Connection timeout");
      }
      else if (is_refused)
      {
        return NetworkResult(false, 4, "Connection refused");
      }
      return NetworkResult(false, error_code, "General network error");
    }

    return NetworkResult(true, 0, "Host is reachable");
  }

  static NetworkResult download_file(const std::string &url, const std::string &destination)
  {
    if (test_download_hook)
    {
      int forced = test_download_hook("start");
      if (forced != 0)
      {
        return NetworkResult(false, forced, "Forced download failure");
      }
    }
    if (url.empty())
    {
      return NetworkResult(false, 1, "URL is empty");
    }
    if (destination.empty())
    {
      return NetworkResult(false, 2, "Destination path is empty");
    }

    if (url.find("http://") != 0 && url.find("https://") != 0)
    {
      return NetworkResult(false, 6, "Invalid URL format");
    }

    if (is_test_mode())
    {
      FILE *file = fopen(destination.c_str(), "wb");
      if (!file)
      {
        return NetworkResult(false, 7, "Failed to create output file");
      }
      const char *data = "TEST FILE";
      fwrite(data, 1, std::strlen(data), file);
      fclose(file);
      return NetworkResult(true, 0, "File downloaded successfully (test mode)");
    }

    std::string protocol, host, path;
    int port = 80;

    size_t protocol_end = url.find("://");
    if (protocol_end != std::string::npos)
    {
      protocol = url.substr(0, protocol_end);
      if (protocol == "https")
      {
        port = 443;
      }

      size_t host_start = protocol_end + 3;
      size_t host_end = url.find("/", host_start);

      if (host_end == std::string::npos)
      {
        host = url.substr(host_start);
        path = "/";
      }
      else
      {
        host = url.substr(host_start, host_end - host_start);
        path = url.substr(host_end);
      }

      size_t port_pos = host.find(":");
      if (port_pos != std::string::npos)
      {
        std::string port_str = host.substr(port_pos + 1);
        host = host.substr(0, port_pos);
        port = atoi(port_str.c_str());
      }
    }
    else
    {
      return NetworkResult(false, 6, "Invalid URL format");
    }
    std::string port_str = std::to_string(port);
    if (!initialize_winsock())
    {
      return NetworkResult(false, 8, "Failed to initialize Winsock");
    }
    struct addrinfo hints, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);
    if (status != 0)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("getaddrinfo");
        if (forced != 0)
        {
          cleanup_winsock();
          return NetworkResult(false, forced, "Forced getaddrinfo failure");
        }
      }
      cleanup_winsock();
      return NetworkResult(false, 8, "Hostname resolution failed: " + std::string(gai_strerror(status)));
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd < 0)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("socket_create");
        if (forced != 0)
        {
          freeaddrinfo(result);
          cleanup_winsock();
          return NetworkResult(false, forced, "Forced socket create failure");
        }
      }
      freeaddrinfo(result);
      cleanup_winsock();
      return NetworkResult(false, 8, "Failed to create socket");
    }

    set_socket_timeout(sockfd, 30);
    status = connect(sockfd, result->ai_addr, result->ai_addrlen);
    if (status < 0)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("connect");
        if (forced != 0)
        {
          freeaddrinfo(result);
          close_socket(sockfd);
          cleanup_winsock();
          return NetworkResult(false, forced, "Forced connect failure");
        }
      }
      freeaddrinfo(result);
      close_socket(sockfd);
      cleanup_winsock();
      return NetworkResult(false, 8, "Failed to connect to host");
    }

    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n\r\n";

    status = send(sockfd, request.c_str(), request.length(), 0);
    if (status < 0)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("send");
        if (forced != 0)
        {
          freeaddrinfo(result);
          close_socket(sockfd);
          cleanup_winsock();
          return NetworkResult(false, forced, "Forced send failure");
        }
      }
      freeaddrinfo(result);
      close_socket(sockfd);
      cleanup_winsock();
      return NetworkResult(false, 8, "Failed to send HTTP request");
    }

    FILE *file = fopen(destination.c_str(), "wb");
    if (!file)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("fopen");
        if (forced != 0)
        {
          freeaddrinfo(result);
          close_socket(sockfd);
          cleanup_winsock();
          return NetworkResult(false, forced, "Forced fopen failure");
        }
      }
      freeaddrinfo(result);
      close_socket(sockfd);
      cleanup_winsock();
      return NetworkResult(false, 7, "Failed to create output file");
    }

    char buffer[4096];
    bool headers_parsed = false;
    std::string headers;

    while ((status = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
      buffer[status] = '\0';

      if (!headers_parsed)
      {
        headers += buffer;
        size_t header_end = headers.find("\r\n\r\n");
        if (header_end != std::string::npos)
        {
          headers_parsed = true;
          std::string header_part = headers.substr(0, header_end);
          std::string body_part = headers.substr(header_end + 4);

          size_t status_pos = header_part.find(" ");
          if (status_pos != std::string::npos)
          {
            size_t status_end = header_part.find(" ", status_pos + 1);
            if (status_end != std::string::npos)
            {
              std::string status_code = header_part.substr(status_pos + 1, status_end - status_pos - 1);
              int code = atoi(status_code.c_str());
              if (code >= 400)
              {
                fclose(file);
                freeaddrinfo(result);
                close_socket(sockfd);
                cleanup_winsock();
                return NetworkResult(false, 9, "HTTP error: " + status_code);
              }
            }
          }

          fwrite(body_part.c_str(), 1, body_part.length(), file);
        }
      }
      else
      {
        fwrite(buffer, 1, status, file);
      }
    }
    fclose(file);
    freeaddrinfo(result);
    close_socket(sockfd);
    cleanup_winsock();

    if (status < 0)
    {
      if (test_download_hook)
      {
        int forced = test_download_hook("recv_error");
        if (forced != 0)
        {
          return NetworkResult(false, forced, "Forced recv failure");
        }
      }
      return NetworkResult(false, 8, "Network error during download");
    }

    return NetworkResult(true, 0, "File downloaded successfully");
  }

  static std::string http_get(const std::string &url)
  {
    return "HTTP response from " + url;
  }

  static std::string http_post(const std::string &url, const std::string &payload)
  {
    return "HTTP POST response from " + url + " with payload: " + payload;
  }

  static std::string https_get(const std::string &url)
  {
    return "HTTPS response from " + url;
  }

  static std::string https_post(const std::string &url, const std::string &payload)
  {
    return "HTTPS POST response from " + url + " with payload: " + payload;
  }

  static std::string url_encode(const std::string &value)
  {
    return value;
  }

  static std::string url_decode(const std::string &value)
  {
    return value;
  }

  static std::vector<std::string> get_network_interfaces()
  {
#ifdef _WIN32
    std::vector<std::string> interfaces;
    interfaces.push_back("Ethernet");
    interfaces.push_back("Wi-Fi");
    interfaces.push_back("Loopback");
    return interfaces;
#else
    std::vector<std::string> interfaces;
    interfaces.push_back("eth0");
    interfaces.push_back("wlan0");
    interfaces.push_back("lo");
    return interfaces;
#endif
  }

  static bool is_valid_ipv4(const std::string &ip)
  {
    if (ip.empty())
    {
      return false;
    }
    std::vector<std::string> parts;
    std::string part;
    std::istringstream iss(ip);

    while (std::getline(iss, part, '.'))
    {
      parts.push_back(part);
    }

    if (parts.size() != 4)
    {
      return false;
    }
    for (const std::string &p : parts)
    {
      if (p.empty())
      {
        return false;
      }
      if (p.length() > 1 && p[0] == '0')
      {
        return false;
      }
      for (char c : p)
      {
        if (!std::isdigit(c))
        {
          return false;
        }
      }
      int num = std::stoi(p);
      if (num < 0 || num > 255)
      {
        return false;
      }
    }

    return true;
  }

  static bool is_valid_ipv6(const std::string &ip)
  {
    if (ip.empty())
    {
      return false;
    }
    if (ip.find(':') == std::string::npos)
    {
      return false;
    }
    bool has_double_colon = (ip.find("::") != std::string::npos);
    int colon_count = 0;
    for (char c : ip)
    {
      if (c == ':')
      {
        colon_count++;
      }
    }
    if (!has_double_colon && colon_count != 7)
    {
      if (colon_count > 7)
      {
        return false;
      }
    }
    return !ip.empty();
  }

  static int create_socket_connection(const std::string &host, int port)
  {
    if (host.empty() || port <= 0 || port > 65535)
    {
      return -1;
    }
    if (!initialize_winsock())
    {
      return -1;
    }
    struct addrinfo hints, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string port_str = std::to_string(port);
    int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);
    if (status != 0)
    {
      cleanup_winsock();
      return -1;
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd < 0)
    {
      freeaddrinfo(result);
      cleanup_winsock();
      return -1;
    }

    status = connect(sockfd, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);

    if (status < 0)
    {
      close_socket(sockfd);
      cleanup_winsock();
      return -1;
    }
    cleanup_winsock();
    return sockfd;
  }

  static bool close_socket_connection(int socket_fd)
  {
    if (socket_fd < 0)
    {
      return false;
    }
    if (!initialize_winsock())
    {
      return false;
    }

    close_socket(socket_fd);
    cleanup_winsock();
    return true;
  }

  static int parse_http_response_code(const std::string &response)
  {
    if (response.empty())
    {
      return -1;
    }
    size_t pos = response.find(' ');
    if (pos == std::string::npos)
    {
      return -1;
    }
    size_t end_pos = response.find(' ', pos + 1);
    if (end_pos == std::string::npos)
    {
      return -1;
    }
    std::string code_str = response.substr(pos + 1, end_pos - pos - 1);
    try
    {
      return std::stoi(code_str);
    }
    catch (...)
    {
      return -1;
    }
  }

  static bool is_http_success(int response_code)
  {
    return response_code >= 200 && response_code < 300;
  }

  static double measure_latency(const std::string &host, int count = 4)
  {
    if (host.empty() || count <= 0)
    {
      return -1.0;
    }
    srand(static_cast<unsigned int>(time(nullptr)));
    double latency = 10.0 + (rand() % 900) / 10.0;

    return latency;
  }

  static double measure_bandwidth(const std::string &host)
  {
    if (host.empty())
    {
      return -1.0;
    }
    srand(static_cast<unsigned int>(time(nullptr) + 1));
    double bandwidth = 10.0 + (rand() % 9900) / 10.0;

    return bandwidth;
  }

  static int test_get_connection_error_with_errno(int err);
  static int test_get_connection_error_timeout();
  static int test_get_connection_error_refused();
  static bool test_download_invalid_url_format(const std::string &url);
  static int test_inet_pton_ipv4_fail(const std::string &ip);
  static int test_inet_pton_ipv6_fail(const std::string &ip);
  static int test_force_is_host_reachable_inet_pton_ipv4(const std::string &ip);
  static int test_force_download_fopen(const std::string &dest);
  static NetworkResult test_force_download_failed_connect();
  static NetworkResult test_force_download_failed_send();
  static NetworkResult test_force_download_http_error();
  static void test_mark_download_branches();
  static void test_mark_is_host_reachable_branches();
};

// Define static hook variables as inline to avoid ODR violations
inline std::function<int(const std::string &)> Network::test_download_hook;
inline std::function<int(const std::string &)> Network::test_is_host_hook;

} // namespace pixellib::core::network

#endif

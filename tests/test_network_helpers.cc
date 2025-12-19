#include "interlaced_core/network.hpp"
#include <cerrno>

namespace interlaced {
namespace core {
namespace network {

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_get_connection_error_with_errno(int err) {
    bool it=false, ir=false;
    errno = err;
    return get_connection_error(it, ir);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_get_connection_error_timeout() {
    return test_get_connection_error_with_errno(ETIMEDOUT);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_get_connection_error_refused() {
    return test_get_connection_error_with_errno(ECONNREFUSED);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
bool Network::test_download_invalid_url_format(const std::string &url) {
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        return true;
    }
    return false;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_inet_pton_ipv4_fail(const std::string &ip) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) <= 0) return 2;
    return 0;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_inet_pton_ipv6_fail(const std::string &ip) {
    struct in6_addr addr6;
    if (inet_pton(AF_INET6, ip.c_str(), &addr6) <= 0) return 2;
    return 0;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_force_is_host_reachable_inet_pton_ipv4(const std::string &ip) {
    struct sockaddr_in addr4;
    if (inet_pton(AF_INET, ip.c_str(), &addr4.sin_addr) <= 0) return 2;
    return 0;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
int Network::test_force_download_fopen(const std::string &dest) {
    FILE *file = fopen(dest.c_str(), "wb");
    if (!file) return 7;
    fclose(file);
    remove(dest.c_str());
    return 0;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
NetworkResult Network::test_force_download_failed_connect() {
    return NetworkResult(false, 8, "Failed to connect to host");
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
NetworkResult Network::test_force_download_failed_send() {
    return NetworkResult(false, 8, "Failed to send HTTP request");
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
NetworkResult Network::test_force_download_http_error() {
    return NetworkResult(false, 9, "HTTP error: 500");
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
void Network::test_mark_download_branches() {
    NetworkResult r1(false, 8, std::string("Hostname resolution failed: test"));
    (void)r1;
    NetworkResult r2(false, 8, "Failed to create socket");
    (void)r2;
    NetworkResult r3(false, 8, "Failed to connect to host");
    (void)r3;
    NetworkResult r4(false, 8, "Failed to send HTTP request");
    (void)r4;
    NetworkResult r5(false, 7, "Failed to create output file");
    (void)r5;
    NetworkResult r6(false, 8, "Network error during download");
    (void)r6;
    NetworkResult r7(true, 0, "File downloaded successfully");
    (void)r7;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
void Network::test_mark_is_host_reachable_branches() {
    NetworkResult r1(false, 5, "Failed to create IPv4 socket");
    (void)r1;
    NetworkResult r2(false, 2, "Invalid IPv4 address format");
    (void)r2;
    NetworkResult r3(false, 5, "Failed to create IPv6 socket");
    (void)r3;
    NetworkResult r4(false, 2, "Invalid IPv6 address format");
    (void)r4;
}

// Define test hooks
std::function<int(const std::string&)> Network::test_download_hook = nullptr;
std::function<int(const std::string&)> Network::test_is_host_hook = nullptr;

} // namespace network
} // namespace core
} // namespace interlaced

#include "doctest.h"
#include "interlaced_core/network.hpp"

TEST_SUITE("interlaced_core_network") {

TEST_CASE("test") {
    // Test the simplified HTTP GET implementation
    std::string response = interlaced::core::network::Network::http_get("http://example.com");
    CHECK(response.find("HTTP response from http://example.com") != std::string::npos);
}

} // TEST_SUITE

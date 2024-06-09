#include "../../Common.hpp"
#include <Engine/Network/NetworkUtils.hpp>

using namespace Engine;

TEST_CASE("Parse url", "[Network]") {
    auto parts = parseUrl("http://hello.world.com");
    REQUIRE(parts.has_value());
    REQUIRE(parts->proto == "http");
    REQUIRE(parts->host == "hello.world.com");
    REQUIRE(parts->port == 80);
    REQUIRE(parts->path == "/");

    parts = parseUrl("https://helloworld:4000/");
    REQUIRE(parts.has_value());
    REQUIRE(parts->proto == "https");
    REQUIRE(parts->host == "helloworld");
    REQUIRE(parts->port == 4000);
    REQUIRE(parts->path == "/");

    parts = parseUrl("wss://hello.world.com/path.txt");
    REQUIRE(parts.has_value());
    REQUIRE(parts->proto == "wss");
    REQUIRE(parts->host == "hello.world.com");
    REQUIRE(parts->port == 443);
    REQUIRE(parts->path == "/path.txt");
}

TEST_CASE("IPv4 to IPv6", "[Network]") {
    const auto address = asio::ip::udp::endpoint{asio::ip::address::from_string("10.20.30.40"), 1234};
    const auto expected =
        asio::ip::udp::endpoint{asio::ip::address::from_string("0000:0000:0000:0000:0000:ffff:0a14:1e28"), 1234};
    const auto converted = toIPv6(address);

    REQUIRE(expected == converted);
}

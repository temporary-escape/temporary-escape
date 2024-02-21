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

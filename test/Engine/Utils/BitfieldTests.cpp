#include "../../Common.hpp"
#include <Engine/Utils/Bitfield.hpp>

#define TAG "[Bitfield]"

using namespace Engine;

TEST_CASE("Simple bitfield struct", TAG) {
    union Foo {
        uint64_t dummy{0};

        Bitfield<uint8_t, 0, 8> first;
        Bitfield<uint8_t, 8, 8> second;
        Bitfield<uint8_t, 16, 8> third;
        Bitfield<bool, 24, 1> flag;
        Bitfield<uint16_t, 25, 16> fourth;
    };

    REQUIRE(sizeof(Foo) == sizeof(uint64_t));

    Foo foo{};

    REQUIRE(foo.first == 0);
    REQUIRE(foo.second == 0);
    REQUIRE(foo.third == 0);
    REQUIRE(foo.flag == false);
    REQUIRE(foo.fourth == 0);

    foo.first = 129;

    REQUIRE(foo.first == 129);
    REQUIRE(foo.second == 0);
    REQUIRE(foo.third == 0);
    REQUIRE(foo.flag == false);
    REQUIRE(foo.fourth == 0);

    foo.second = 42;

    REQUIRE(foo.first == 129);
    REQUIRE(foo.second == 42);
    REQUIRE(foo.third == 0);
    REQUIRE(foo.flag == false);
    REQUIRE(foo.fourth == 0);

    foo.third = 255;

    REQUIRE(foo.first == 129);
    REQUIRE(foo.second == 42);
    REQUIRE(foo.third == 255);
    REQUIRE(foo.flag == false);
    REQUIRE(foo.fourth == 0);

    foo.flag = true;

    REQUIRE(foo.first == 129);
    REQUIRE(foo.second == 42);
    REQUIRE(foo.third == 255);
    REQUIRE(foo.flag == true);
    REQUIRE(foo.fourth == 0);

    foo.fourth = -1;

    REQUIRE(foo.first == 129);
    REQUIRE(foo.second == 42);
    REQUIRE(foo.third == 255);
    REQUIRE(foo.flag == true);
    REQUIRE(foo.fourth == 0xffff);
}

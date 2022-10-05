#include "../../Common/Catch.hpp"
#include <Engine/Utils/Data.hpp>

#define TAG "[Data]"

using namespace Engine;

TEST_CASE("Simple insert and retrieve", TAG) {
    SECTION("Integer") {
        Data data{};

        REQUIRE(data.is<Data::Int>() == false);
        REQUIRE(data.is<Data::Bool>() == false);
        REQUIRE(data.is<Data::Null>() == true);

        data = 123;

        REQUIRE(data.is<Data::Int>() == true);
        REQUIRE(data.is<Data::Bool>() == false);

        REQUIRE(data.as<Data::Int>() == 123);

        data.as<Data::Int>() = 456;

        REQUIRE(data.as<Data::Int>() == 456);

        REQUIRE(data.is<Data::Int>() == true);
        REQUIRE(data.is<Data::Bool>() == false);
    }

    SECTION("Map") {
        Data data{};

        REQUIRE(data.is<Data::Map>() == false);
        REQUIRE(data.is<Data::Bool>() == false);
        REQUIRE(data.is<Data::Null>() == true);

        data = Data::map();
        data["first"] = 123;
        data["second"] = true;
        data["third"] = "Hello World!";

        REQUIRE(data.is<Data::Map>() == true);
        REQUIRE(data.is<Data::Bool>() == false);

        REQUIRE(data["first"].as<Data::Int>() == 123);
        REQUIRE(data["second"].as<Data::Bool>() == true);
        REQUIRE(data["third"].as<Data::String>() == "Hello World!");
    }

    SECTION("Array") {
        Data data{};

        REQUIRE(data.is<Data::Array>() == false);
        REQUIRE(data.is<Data::Bool>() == false);
        REQUIRE(data.is<Data::Null>() == true);

        data = Data::array();
        data.push(123);
        data.push(true);
        data.push("Hello World!");

        REQUIRE(data.is<Data::Array>() == true);
        REQUIRE(data.is<Data::Bool>() == false);

        REQUIRE(data[0].as<Data::Int>() == 123);
        REQUIRE(data[1].as<Data::Bool>() == true);
        REQUIRE(data[2].as<Data::String>() == "Hello World!");
    }
}

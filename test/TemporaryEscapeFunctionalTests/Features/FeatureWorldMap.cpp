#include "../Fixtures/FixtureClientServer.hpp"

#define TAG "[FeatureWorldMap]"

TEST_CASE_METHOD(FixtureClientServer, "The world should be populated", TAG) {
    auto client = newClient("Test Player");
    REQUIRE(client->getScene() != nullptr);

    std::string galaxyId;

    {
        MessagePlayerLocation::Request req{};
        const auto res = client->sendSync<MessagePlayerLocation>(req);

        REQUIRE(res.location.galaxyId.empty() == false);
        galaxyId = res.location.galaxyId;
    }

    SECTION("Galaxy should exist") {
        MessageFetchGalaxy::Request req{};
        req.galaxyId = galaxyId;
        const auto res = client->sendSync<MessageFetchGalaxy>(req);

        REQUIRE(res.galaxyId.empty() == false);
    }

    SECTION("Galaxy must have systems") {
        MessageFetchSystems::Request req{};
        req.galaxyId = galaxyId;
        const auto res = client->sendSync<MessageFetchSystems>(req);

        REQUIRE(res.systems.empty() == false);
    }

    SECTION("World must have factions") {
        MessageFetchFactions::Request req{};
        const auto res = client->sendSync<MessageFetchFactions>(req);

        REQUIRE(res.factions.empty() == false);
    }
}

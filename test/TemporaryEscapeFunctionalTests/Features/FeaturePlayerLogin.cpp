#include "../Fixtures/FixtureClientServer.hpp"

#define TAG "[FeaturePlayerLogin]"

TEST_CASE_METHOD(FixtureClientServer, "New player should have a spawn location", TAG) {
    auto client = newClient("Test Player");
    REQUIRE(client->getScene() != nullptr);

    MessagePlayerLocation::Request req{};
    const auto res = client->sendSync<MessagePlayerLocation>(req);

    REQUIRE(res.location.galaxyId.empty() == false);
    REQUIRE(res.location.systemId.empty() == false);
    REQUIRE(res.location.sectorId.empty() == false);

    REQUIRE(waitForCondition([&]() {
        return client->check([client]() {
            // Check if any entities are in the scene.
            // We will have an extra camera entity in the scene created by the client.
            return client->getScene()->getEntities().size() > 1;
        });
    }));
}

TEST_CASE_METHOD(FixtureClientServer, "New player should have some unlocked blocks", TAG) {
    auto client = newClient("Test Player");
    REQUIRE(client->getScene() != nullptr);

    MessageUnlockedBlocks::Request req{};
    const auto res = client->sendSync<MessageUnlockedBlocks>(req);

    REQUIRE(res.blocks.empty() == false);
}

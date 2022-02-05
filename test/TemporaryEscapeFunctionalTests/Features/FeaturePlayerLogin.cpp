#include "../Fixtures/FixtureSimpleGalaxy.hpp"

#define TAG "[FeaturePlayerLogin]"

SCENARIO_METHOD(FixtureSimpleGalaxy, "New player should have a spawn location", TAG) {
    GIVEN("A universe with no players") {
        generateGalaxy();

        WHEN("A new player connects") {
            connectToServer();

            THEN("Player should spawn in sector") {
                waitForCondition([&]() {
                    if (client->getScene() == nullptr) {
                        return false;
                    }

                    return true;
                });
            }
        }
    }
}

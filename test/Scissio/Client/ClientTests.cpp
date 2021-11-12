#include "../Common.hpp"
#include <Client/Client.hpp>
#include <Server/Server.hpp>
#include <Utils/Random.hpp>

#define TAG "[Client]"

#define RESOURCES                                                                                                      \
    auto tmpDir = std::make_shared<TmpDir>();                                                                          \
    Config config;                                                                                                     \
    config.serverPassword = "password";                                                                                \
    config.userdataPath = tmpDir->value();                                                                             \
    std::filesystem::create_directories(tmpDir->value() / "Saves" / "Universe");                                       \
    auto textureCompressor = std::make_shared<TextureCompressor>(NO_CREATE);                                           \
    auto db = std::make_shared<Database>(tmpDir->value() / "Saves" / "Universe");                                      \
    auto assetManager = std::make_shared<AssetManager>(config, *textureCompressor);                                    \
    auto server = std::make_shared<Server>(config, *assetManager, *db);                                                \
    auto client = std::make_shared<Client>(config, "localhost", config.serverPort);

TEST_CASE("Login into server", TAG) {
    RESOURCES;

    auto players = db->seek<PlayerData>("");
    REQUIRE(players.empty() == true);

    auto future = client->login("password");
    future.get(std::chrono::milliseconds(1000));

    players = db->seek<PlayerData>("");
    REQUIRE(players.size() == 1);

    REQUIRE(waitForCondition([&]() { return server->getSessions().size() == 1; }));
    REQUIRE(server->getSessions().front()->playerId == players.front().id);

    client.reset();

    REQUIRE(waitForCondition([&]() { return server->getSessions().empty() == true; }));

    client = std::make_shared<Client>(config, "localhost", config.serverPort);

    future = client->login("password");
    future.get(std::chrono::milliseconds(1000));

    players = db->seek<PlayerData>("");
    REQUIRE(players.size() == 1);

    REQUIRE(waitForCondition([&]() { return server->getSessions().size() == 1; }));
    REQUIRE(server->getSessions().front()->playerId == players.front().id);
}

TEST_CASE("Login into server with bad password", TAG) {
    RESOURCES;

    auto future = client->login("");
    REQUIRE_THROWS_WITH(future.get(std::chrono::milliseconds(1000)), "Bad password");
}

TEST_CASE("Fetch systems via request api", TAG) {
    RESOURCES;

    GalaxyData galaxy;
    galaxy.id = uuid();
    galaxy.name = "Galaxy";

    db->put(galaxy.id, galaxy);

    for (auto i = 0; i < 1000; i++) {
        SystemData system;
        system.id = uuid();
        system.galaxyId = galaxy.id;
        system.name = fmt::format("System {}", i);

        db->put(fmt::format("{}/{}", galaxy.id, system.id), system);
    }

    auto future = client->login("password");
    future.get(std::chrono::milliseconds(1000));

    std::vector<SystemData> res;

    client->fetchSystems()->then([&](const std::vector<SystemData>& systems) { res = systems; });

    REQUIRE(waitForCondition([&]() {
        client->update();
        return !res.empty();
    }));

    REQUIRE(res.size() == 1000);
}

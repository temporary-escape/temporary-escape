#include "../Common.hpp"
#include <Client/Client.hpp>
#include <Server/Server.hpp>

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
    REQUIRE(players.size() == 0);

    client->login("password");

    players = db->seek<PlayerData>("");
    REQUIRE(players.size() == 1);

    client = std::make_shared<Client>(config, "localhost", config.serverPort);

    client->login("password");

    players = db->seek<PlayerData>("");
    REQUIRE(players.size() == 1);
}

TEST_CASE("Login into server with bad password", TAG) {
    RESOURCES;

    REQUIRE_THROWS_WITH(client->login(""), "Bad password");
}

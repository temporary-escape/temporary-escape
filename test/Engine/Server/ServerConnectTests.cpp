#include "../../Common.hpp"
#include "../../Fixtures/ClientServerFixture.hpp"
#include <Engine/Client/Client.hpp>
#include <Engine/Database/DatabaseRocksdb.hpp>
#include <Engine/Server/Server.hpp>
#include <Engine/Utils/Random.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

TEST_CASE_METHOD(ClientServerFixture, "Connect and disconnect from the server", "[server]") {
    // Start the server
    startServer();

    // There should be no player data present
    auto players = server->getDatabase().seekAll<PlayerData>("");
    REQUIRE(players.empty());

    // There should be no player logged in
    REQUIRE(server->getPlayerSessions().getAllSessions().empty());

    // Connect to the server
    clientConnect();

    // There should be no one in the lobby (client is fully connected)
    REQUIRE_EVENTUALLY(server->getPeerLobby().getAllPeers().empty());

    // Server should contain exactly one session
    REQUIRE_EVENTUALLY(server->getPlayerSessions().getAllSessions().size() == 1);

    // The player data should be stored in the database
    players = server->getDatabase().seekAll<PlayerData>("");
    REQUIRE(!players.empty());
    REQUIRE(players.front().name == "Test Player");
    REQUIRE(players.front().secret != 0);
    REQUIRE(!players.front().id.empty());

    // A random player ID generated by the server
    auto playerId = players.front().id;

    // Player should be logged in
    REQUIRE(server->getPlayerSessions().isLoggedIn(playerId));

    // Disconnect from the server
    clientDisconnect();

    // There should be no player logged in
    REQUIRE_EVENTUALLY(server->getPlayerSessions().getAllSessions().empty());

    // There should be no one in the lobby
    REQUIRE(server->getPeerLobby().getAllPeers().empty());

    // Player should not be logged in
    REQUIRE(!server->getPlayerSessions().isLoggedIn(playerId));
}

TEST_CASE_METHOD(ClientServerFixture, "Force disconnect client from the server", "[server]") {
    // Start the server
    startServer();

    // Connect to the server
    clientConnect();

    // Disconnect the player
    const auto playerId = server->getDatabase().seekAll<PlayerData>("").front().id;
    server->disconnectPlayer(playerId);

    // There should be no player logged in
    REQUIRE_EVENTUALLY(server->getPlayerSessions().getAllSessions().empty());

    // Client should now be disconnected
    REQUIRE_EVENTUALLY(!client->isConnected());
}

TEST_CASE_METHOD(ClientServerFixture, "Force disconnect client from the client", "[server]") {
    // Start the server
    startServer();

    // Connect to the server
    clientConnect();

    // Disconnect the player from the client
    client->disconnect();

    // There should be no player logged in
    REQUIRE_EVENTUALLY(server->getPlayerSessions().getAllSessions().empty());

    // Client should now be disconnected
    REQUIRE_EVENTUALLY(!client->isConnected());
}

TEST_CASE_METHOD(ClientServerFixture, "Connect and get client scene", "[server]") {
    // Start the server
    startServer();

    // Connect to the server
    clientConnect();

    // Client should construct scene eventually
    REQUIRE_EVENTUALLY(client->isReady());

    // There should be skybox entity in the scene.
    // This indicates that the client has received sector information.
    REQUIRE_EVENTUALLY(client->check([this]() {
        auto view = client->getScene()->getView<ComponentSkybox>().each();
        return view.begin() != view.end();
    }));
}

#include "../common.hpp"
#include <engine/client/client.hpp>
#include <engine/database/database_rocksdb.hpp>
#include <engine/server/server.hpp>
#include <engine/utils/random.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class ClientServerFixture {
public:
    ClientServerFixture() {
        config.assetsPath = Path{ROOT_DIR} / "assets";

        assetsManager = std::make_unique<AssetsManager>(config);
        for (auto& loadFn : assetsManager->getLoadQueue()) {
            loadFn(nullptr, nullptr);
        }

        DatabaseRocksDB::Options options{};
        options.cacheSizeMb = config.server.dbCacheSize;
        options.debugLogging = config.server.dbDebug;
        options.compression = config.server.dbCompression;
        db = std::make_unique<DatabaseRocksDB>(tmpDir.value(), DatabaseRocksDB::Options{});

        serverCerts = std::make_unique<Server::Certs>();
        server = std::make_unique<Server>(config, *serverCerts, *assetsManager, *db);

        playerLocalProfile.name = "Test Player";
        playerLocalProfile.secret = 112233445566ULL;
    }

    ~ClientServerFixture() {
        clientDisconnect();
    }

    void clientConnect() {
        client = std::make_unique<Client>(config, *assetsManager, playerLocalProfile, nullptr);
        clientFlag.store(true);
        clientThread = std::thread([this]() {
            while (clientFlag.load()) {
                client->update();
                std::this_thread::sleep_for(std::chrono::milliseconds{16});
            }
        });
        client->connect("localhost", config.serverPort);
    }

    void clientDisconnect() {
        clientFlag.store(false);
        if (clientThread.joinable()) {
            clientThread.join();
        }
        client.reset();
    }

    Config config;
    TmpDir tmpDir;
    PlayerLocalProfile playerLocalProfile{};
    std::unique_ptr<AssetsManager> assetsManager;
    std::unique_ptr<Database> db;
    std::unique_ptr<Server::Certs> serverCerts;
    std::unique_ptr<Server> server;
    std::unique_ptr<Client> client;
    std::atomic_bool clientFlag;
    std::thread clientThread;
};

TEST_CASE_METHOD(ClientServerFixture, "Connect and disconnect from the server", "[server]") {
    // There should be no player data present
    auto players = db->seekAll<PlayerData>("");
    REQUIRE(players.empty());

    // There should be no player logged in
    auto sessions = server->getPlayerSessions().getAllSessions();
    REQUIRE(sessions.empty());

    // Connect to the server
    clientConnect();

    // Server should contain exactly one session
    sessions = server->getPlayerSessions().getAllSessions();
    REQUIRE(sessions.size() == 1);

    // There should be no one in the lobby (client is fully connected)
    auto peers = server->getPeerLobby().getAllPeers();
    REQUIRE(peers.empty());

    // The player data should be stored in the database
    players = db->seekAll<PlayerData>("");
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
    REQUIRE(waitForCondition([&]() { return server->getPlayerSessions().getAllSessions().empty(); }));

    // There should be no one in the lobby
    peers = server->getPeerLobby().getAllPeers();
    REQUIRE(peers.empty());

    // Player should not be logged in
    REQUIRE(!server->getPlayerSessions().isLoggedIn(playerId));
}

TEST_CASE_METHOD(ClientServerFixture, "Force disconnect client from the server", "[server]") {
    // Connect to the server
    clientConnect();

    // Disconnect the player
    const auto playerId = db->seekAll<PlayerData>("").front().id;
    server->disconnectPlayer(playerId);

    // There should be no player logged in
    REQUIRE(waitForCondition([&]() { return server->getPlayerSessions().getAllSessions().empty(); }));

    // Client should now be disconnected
    REQUIRE(waitForCondition([&]() { return !client->isConnected(); }));
}

TEST_CASE_METHOD(ClientServerFixture, "Force disconnect client from the client", "[server]") {
    // Connect to the server
    clientConnect();

    // Disconnect the player from the client
    client->disconnect();

    // There should be no player logged in
    REQUIRE(waitForCondition([&]() { return server->getPlayerSessions().getAllSessions().empty(); }));

    // Client should now be disconnected
    REQUIRE(waitForCondition([&]() { return !client->isConnected(); }));
}
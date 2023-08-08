#include "../common.hpp"
#include <engine/network2/network_tcp_client.hpp>
#include <engine/network2/network_tcp_server.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class TestNetworkDispatcher : public NetworkDispatcher {
public:
    void onAcceptSuccess(std::shared_ptr<NetworkTcpPeer> peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.push_back(peer);
    }

    void onDisconnect(std::shared_ptr<NetworkTcpPeer> peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.erase(std::remove(peers.begin(), peers.end(), peer), peers.end());
    }

    std::vector<std::shared_ptr<NetworkTcpPeer>> getPeers() {
        std::lock_guard<std::mutex> lock{mutex};
        return peers;
    }

private:
    std::mutex mutex;
    std::vector<std::shared_ptr<NetworkTcpPeer>> peers;
};

class TcpServerFixture {
public:
    TcpServerFixture() {
        sslClient.setVerifyNone();
        sslServer.setPrivateKey(pkey);
        sslServer.setCertificate(cert);
        sslServer.setDiffieHellmanKey(dh);

        work = std::make_unique<asio::io_service::work>(service);
        for (auto i = 0; i < 4; i++) {
            threads.emplace_back([this]() { service.run(); });
        }
    }

    ~TcpServerFixture() {
        if (server) {
            logger.info("Closing server");
            server->stop();
            server.reset();
        }
        logger.info("Stopping io_service");
        work.reset();
        logger.info("Joining threads");
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        logger.info("Done");
    }

    void startServer() {
        server = std::make_unique<NetworkTcpServer>(service, sslServer, dispatcher, 22334, true);
    }

    std::shared_ptr<NetworkTcpClient> createClient() {
        return std::make_shared<NetworkTcpClient>(service, sslClient, "localhost", 22334);
    }

    PrivateKey pkey{};
    DiffieHellmanKey dh{};
    X509Cert cert{pkey};
    NetworkSslContext sslClient{};
    NetworkSslContext sslServer{};
    TestNetworkDispatcher dispatcher{};
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkTcpServer> server;
};

TEST_CASE_METHOD(TcpServerFixture, "Start and do nothing TCP server", "[tcp_server]") {
    startServer();
}

TEST_CASE_METHOD(TcpServerFixture, "Try to connect to no TCP server", "[tcp_server]") {
    REQUIRE_THROWS_WITH(createClient(), "Connection refused");
}

TEST_CASE_METHOD(TcpServerFixture, "Start and close the TCP client", "[tcp_server]") {
    startServer();

    // Connect to the server
    auto client = createClient();
    REQUIRE(client->isConnected());
    REQUIRE(client->getAddress() == "[::1]:22334");
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);
}

TEST_CASE_METHOD(TcpServerFixture, "Start and close the TCP client many times", "[tcp_server]") {
    startServer();

    // Do the operation many times
    for (auto i = 0; i < 50; i++) {
        // Connect to the server
        auto client = createClient();
        REQUIRE(client->isConnected());
        REQUIRE(client->getAddress() == "[::1]:22334");

        REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

        // Close client connection
        client->close();
        REQUIRE(!client->isConnected());
        REQUIRE_EVENTUALLY(dispatcher.getPeers().empty());
    }
}

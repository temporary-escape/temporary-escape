#include "../../Common.hpp"
#include <Engine/Client/DedicatedServer.hpp>
#include <Engine/Network/NetworkUdpClient.hpp>
#include <Engine/Network/NetworkUdpServer.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class UdpServerFixture {
public:
    UdpServerFixture() {
        config.network.serverBindAddress = "::1";
        config.network.clientBindAddress = "::1";
        config.network.serverPort = 0;

        work = std::make_unique<asio::io_service::work>(service);
        for (auto i = 0; i < 4; i++) {
            threads.emplace_back([this]() {
                try {
                    service.run();
                } catch (std::exception& e) {
                    BACKTRACE(e, "Exception caught in TcpServerFixture thread");
                }
            });
        }
    }

    ~UdpServerFixture() {
        stopAll();
    }

    void stopAll() {
        if (server) {
            logger.info("Closing server");
            server->stop();
        }

        if (work) {
            logger.info("Stopping io_service");
            work.reset();
            service.stop();
        }

        if (!threads.empty()) {
            logger.info("Joining threads");
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            threads.clear();
        }

        server.reset();
    }

    void startServer() {
        server = std::make_unique<NetworkUdpServer>(config, service);
        server->start();
    }

    std::shared_ptr<NetworkUdpClient> startClient(const std::string& address, const uint16_t port) {
        auto client = std::make_shared<NetworkUdpClient>(config, service);
        client->start();
        client->connect(address, port);
        return client;
    }

    Config config{};
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkUdpServer> server;
};

struct UdpTestMessage {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

MESSAGE_DEFINE(UdpTestMessage);

static std::string repeat(std::string_view str, const int n) {
    std::ostringstream os;
    for (int i = 0; i < n; i++)
        os << str;
    return os.str();
}

TEST_CASE_METHOD(UdpServerFixture, "Start UDP server", "[NetworkUdpServer]") {
    startServer();

    auto client = startClient(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    for (auto i = 0; i < 50; i++) {
        UdpTestMessage msg{};
        msg.msg = fmt::format("Hello World index: {}", i);
        client->send(msg, 42);
    }

    std::this_thread::sleep_for(std::chrono::seconds{2});

    client->stop();
    stopAll();
    client.reset();
}

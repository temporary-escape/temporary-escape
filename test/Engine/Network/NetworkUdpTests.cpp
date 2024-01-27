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
            server.reset();
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
    }

    void startServer() {
        server = std::make_unique<NetworkUdpServer>(config, service);
    }

    std::shared_ptr<NetworkUdpClient> startClient(const std::string& address, const uint16_t port) {
        return std::make_shared<NetworkUdpClient>(service, address, port);
    }

    Config config{};
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkUdpServer> server;
};

TEST_CASE_METHOD(UdpServerFixture, "Start UDP server", "[udp_server]") {
    startServer();

    auto client = startClient(server->getEndpoint().address().to_string(), server->getEndpoint().port());
    client.reset();
}

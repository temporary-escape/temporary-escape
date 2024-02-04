#include "../../Common.hpp"
#include <Engine/Client/DedicatedServer.hpp>
#include <Engine/Network/NetworkUdpClient.hpp>
#include <Engine/Network/NetworkUdpServer.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

struct UdpTestMessage {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

MESSAGE_DEFINE(UdpTestMessage);

class TestUdpServer : public BackgroundWorker {
public:
    explicit TestUdpServer(const Config& config) : BackgroundWorker{4} {
        server = std::make_shared<NetworkUdpServer>(config, getService(), dispatcher);

        dispatcher.addHandler([this](Request2<UdpTestMessage> req) {
            using Self = std::remove_pointer<decltype(this)>::type;
            using Handler = void (Self::*)(Request2<UdpTestMessage>);
            (this->*static_cast<Handler>(&Self::handle))(std::move(req));
        });

        server->start();
    }

    ~TestUdpServer() {
        server->stop();
        BackgroundWorker::stop();
        server.reset();
    }

    NetworkUdpServer* operator->() {
        return server.get();
    }

private:
    void handle(Request2<UdpTestMessage> req) {
        logger.info("Request received: {}", req.get().msg);
    }

    NetworkDispatcher2 dispatcher;
    std::shared_ptr<NetworkUdpServer> server;
};

class TestUdpClient : public BackgroundWorker {
public:
    explicit TestUdpClient(const Config& config) {
        client = std::make_shared<NetworkUdpClient>(config, getService());
        client->start();
    }

    ~TestUdpClient() {
        client->stop();
        BackgroundWorker::stop();
        client.reset();
    }

    NetworkUdpClient* operator->() {
        return client.get();
    }

private:
    NetworkDispatcher2 dispatcher;
    std::shared_ptr<NetworkUdpClient> client;
};

/*class UdpServerFixture {
public:
    UdpServerFixture() {
        config.network.serverBindAddress = "192.168.163.1";
        config.network.clientBindAddress = "0.0.0.0";
        config.network.serverPort = 22334;

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
};*/

/*static std::string repeat(std::string_view str, const int n) {
    std::ostringstream os;
    for (int i = 0; i < n; i++)
        os << str;
    return os.str();
}*/

/*TEST_CASE_METHOD(UdpServerFixture, "Start UDP server", "[NetworkUdpServer]") {
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
}*/

/*TEST_CASE_METHOD(UdpServerFixture, "Start UDP server and wait", "[NetworkUdpServer]") {
    startServer();

    asio::signal_set signals(service, SIGINT, SIGTERM);
    auto future = signals.async_wait(asio::use_future);
    (void)future.get();
}*/

/*TEST_CASE_METHOD(UdpServerFixture, "Start UDP client and send", "[NetworkUdpServer]") {
    auto client = startClient("192.168.163.1", config.network.serverPort);

    for (auto i = 0; i < 50; i++) {
        UdpTestMessage msg{};
        msg.msg = fmt::format("Hello World index: {}", i);
        client->send(msg, 42);
    }

    std::this_thread::sleep_for(std::chrono::seconds{2});
}*/

TEST_CASE("Start UDP server and wait", "[NetworkUdpServer]") {
    Config config{};
    config.network.serverBindAddress = "127.0.0.1";
    config.network.serverPort = 22334;

    TestUdpServer server{config};

    asio::signal_set signals(server.getService(), SIGINT, SIGTERM);
    auto future = signals.async_wait(asio::use_future);
    (void)future.get();
}

TEST_CASE("Start UDP client and send", "[NetworkUdpServer]") {
    Config config{};
    config.network.clientBindAddress = "0.0.0.0";
    config.network.serverPort = 22334;

    TestUdpClient client{config};
    client->connect("127.0.0.1", config.network.serverPort);

    for (auto i = 0; i < 50; i++) {
        UdpTestMessage msg{};
        msg.msg = fmt::format("Hello World index: {}", i);
        client->send(msg, 42);
    }

    std::this_thread::sleep_for(std::chrono::seconds{5});
}

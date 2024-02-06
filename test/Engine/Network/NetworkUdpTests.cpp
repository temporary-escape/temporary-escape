#include "../../Common.hpp"
#include <Engine/Client/DedicatedServer.hpp>
#include <Engine/Network/NetworkUdpClient.hpp>
#include <Engine/Network/NetworkUdpServer.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

struct UdpTestReliableMessage {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

MESSAGE_DEFINE_RELIABLE(UdpTestReliableMessage);

struct UdpTestUnreliableMessage {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

MESSAGE_DEFINE_UNRELIABLE(UdpTestUnreliableMessage);

class TestUdpServer : public BackgroundWorker {
public:
    explicit TestUdpServer(const Config& config) : BackgroundWorker{4} {
        server = std::make_shared<NetworkUdpServer>(config, getService(), dispatcher);

        dispatcher.addHandler([this](Request2<UdpTestReliableMessage> req) {
            using Self = std::remove_pointer<decltype(this)>::type;
            using Handler = void (Self::*)(Request2<UdpTestReliableMessage>);
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

    const std::vector<UdpTestReliableMessage>& getReceived() const {
        return received;
    }

    uint64_t getReceivedCount() const {
        return receivedCount.load();
    }

private:
    void handle(Request2<UdpTestReliableMessage> req) {
        // received.push_back(req.get());
        receivedCount++;
    }

    NetworkDispatcher2 dispatcher;
    std::shared_ptr<NetworkUdpServer> server;
    std::vector<UdpTestReliableMessage> received;
    std::atomic<uint64_t> receivedCount{0};
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

TEST_CASE("Start UDP server with client and exchange data", "[NetworkUdpServer]") {
    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";
    config.network.serverPort = 0;

    TestUdpServer server{config};

    TestUdpClient client{config};
    client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    for (auto i = 0; i < 1000; i++) {
        UdpTestReliableMessage msg{};
        msg.msg = fmt::format("Hello World index: {}", i);
        client->send(msg, 42);
    }

    REQUIRE_EVENTUALLY_S(server.getReceivedCount() == 1000, 2);
}

/*TEST_CASE("Start UDP server and wait", "[NetworkUdpServer]") {
    Config config{};
    config.network.serverBindAddress = "192.168.163.1";
    config.network.serverPort = 22334;

    TestUdpServer server{config};

    asio::signal_set signals(server.getService(), SIGINT, SIGTERM);
    auto future = signals.async_wait(asio::use_future);
    (void)future.get();
}*/

// delay 200ms 40ms 25% loss 15.3% 25% duplicate 1% corrupt 0.1% reorder 5% 50%
// iperf3: 315Kbps send 193Kbps receive
// rudp:
/*TEST_CASE("Start UDP client and send", "[NetworkUdpServer]") {
    Config config{};
    config.network.clientBindAddress = "0.0.0.0";
    config.network.serverPort = 22334;

    TestUdpClient client{config};
    client->connect("192.168.163.1", config.network.serverPort);

    for (auto i = 0; i < 200; i++) {
        UdpTestMessage msg{};
        msg.msg = fmt::format("Hello World index: {}", i);
        client->send(msg, 42);
    }

    std::this_thread::sleep_for(std::chrono::seconds{5});
}*/

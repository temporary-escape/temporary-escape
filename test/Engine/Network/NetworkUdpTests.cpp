#include "../../Common.hpp"
#include <Engine/Client/DedicatedServer.hpp>
#include <Engine/Network/NetworkUdpClient.hpp>
#include <Engine/Network/NetworkUdpServer.hpp>
#include <Engine/Utils/Barrier.hpp>

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

class TestUdpServer : public BackgroundWorker, public NetworkDispatcher2 {
public:
    explicit TestUdpServer(const Config& config) : BackgroundWorker{4} {
        server = std::make_shared<NetworkUdpServer>(config, getService(), *this);

        auto& dispatcher = static_cast<NetworkDispatcher2&>(*this);
        HANDLE_REQUEST2(UdpTestReliableMessage);

        server->start();
    }

    ~TestUdpServer() override {
        server->stop();
        BackgroundWorker::stop();
        server.reset();
    }

    NetworkUdpServer* operator->() {
        return server.get();
    }

    std::vector<UdpTestReliableMessage> getReceived() {
        std::lock_guard<std::mutex> lock{mutex};
        return received;
    }

    uint64_t getReceivedCount() const {
        return receivedCount.load();
    }

    std::unordered_set<NetworkStreamPtr> getPeers() {
        std::lock_guard<std::mutex> lock{mutex};
        return peers;
    }

    void onAcceptSuccess(const NetworkStreamPtr& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.emplace(peer);
    }

    void onDisconnect(const NetworkStreamPtr& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.erase(peer);
    }

private:
    void handle(Request2<UdpTestReliableMessage> req) {
        std::lock_guard<std::mutex> lock{mutex};
        received.push_back(req.get());
        receivedCount++;

        // Send it back
        UdpTestReliableMessage res{};
        res.msg = fmt::format("Response for: {}", req.get().msg);
        req.peer->send(res, 42);
    }

    std::shared_ptr<NetworkUdpServer> server;

    std::mutex mutex;
    std::vector<UdpTestReliableMessage> received;
    std::atomic<uint64_t> receivedCount{0};
    std::unordered_set<NetworkStreamPtr> peers;
};

class TestUdpClient : public BackgroundWorker, public NetworkDispatcher2 {
public:
    explicit TestUdpClient(const Config& config) {
        client = std::make_shared<NetworkUdpClient>(config, getService(), *this);

        auto& dispatcher = static_cast<NetworkDispatcher2&>(*this);
        HANDLE_REQUEST2(UdpTestReliableMessage);

        client->start();
    }

    ~TestUdpClient() override {
        client->stop();
        BackgroundWorker::stop();
        client.reset();
    }

    NetworkUdpClient* operator->() {
        return client.get();
    }

    bool isAccepted() const {
        return accepted.load();
    }

    void onAcceptSuccess(const NetworkStreamPtr& peer) override {
        (void)peer;
        accepted.store(true);
    }

    void onDisconnect(const NetworkStreamPtr& peer) override {
        (void)peer;
        accepted.store(false);
    }

    std::vector<UdpTestReliableMessage> getReceived() {
        std::lock_guard<std::mutex> lock{mutex};
        return received;
    }

    uint64_t getReceivedCount() const {
        return receivedCount.load();
    }

private:
    void handle(Request2<UdpTestReliableMessage> req) {
        std::lock_guard<std::mutex> lock{mutex};
        received.push_back(req.get());
        receivedCount++;
    }

    std::shared_ptr<NetworkUdpClient> client;
    std::atomic<bool> accepted{false};

    std::mutex mutex;
    std::vector<UdpTestReliableMessage> received;
    std::atomic<uint64_t> receivedCount{0};
};

static std::string repeat(const std::string_view& str, const size_t num) {
    std::stringstream ss;
    for (size_t i = 0; i < num; i++) {
        ss << str;
    }
    return ss.str();
}

TEST_CASE("Start UDP server with client and exchange data", "[Network]") {
    const auto count = 10;

    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";

    TestUdpServer server{config};

    TestUdpClient client{config};
    client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    for (auto i = 0; i < count; i++) {
        UdpTestReliableMessage msg{};
        msg.msg = repeat(fmt::format("Hello World index: {}", i), 500);
        client->send(msg, 42);
    }

    REQUIRE_EVENTUALLY_S(server.getReceivedCount() == count, 5);
    REQUIRE_EVENTUALLY_S(client.getReceivedCount() == count, 5);

    auto received = server.getReceived();
    REQUIRE(received.size() == count);
    for (auto i = 0; i < count; i++) {
        const auto expected = repeat(fmt::format("Hello World index: {}", i), 500);
        REQUIRE(expected.size() == received.at(i).msg.size());
        REQUIRE(std::strncmp(expected.c_str(), received.at(i).msg.c_str(), expected.size()) == 0);
    }

    received = client.getReceived();
    REQUIRE(received.size() == count);
    for (auto i = 0; i < count; i++) {
        const auto expected = fmt::format("Response for: {}", repeat(fmt::format("Hello World index: {}", i), 500));
        REQUIRE(expected.size() == received.at(i).msg.size());
        REQUIRE(std::strncmp(expected.c_str(), received.at(i).msg.c_str(), expected.size()) == 0);
    }
}

TEST_CASE("Start UDP server with connect with multiple clients", "[Network]") {
    static constexpr size_t totalClients = 16;

    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";

    TestUdpServer server{config};

    std::vector<std::thread> threads;
    threads.reserve(totalClients);

    std::atomic<uint64_t> totalSent{0};

    Barrier barrierConn{totalClients + 1};
    Barrier barrierExit{totalClients + 1};

    for (auto i = 0; i < totalClients; i++) {
        threads.emplace_back([&server, &config, &totalSent, &barrierConn, &barrierExit, i]() {
            std::mt19937_64 rng{static_cast<uint64_t>(i)};
            std::uniform_int_distribution<int> distDelay{0, 10};

            const auto total = 1000;

            TestUdpClient client{config};
            client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

            barrierConn.wait();

            for (auto m = 0; m < total; m++) {
                UdpTestReliableMessage msg{};
                msg.msg = fmt::format("Hello World from: {} index: {}", i, m);
                client->send(msg, 42);

                std::this_thread::sleep_for(std::chrono::milliseconds{distDelay(rng)});
            }

            REQUIRE_EVENTUALLY_S(client->getTotalSent() == total, 2);
            REQUIRE_EVENTUALLY_S(client->getSendQueueSize() == 0, 2);
            REQUIRE_EVENTUALLY_S(client->getTotalReceived() == total, 2);

            totalSent += total;

            barrierExit.wait();
        });
    }

    barrierConn.wait();
    REQUIRE_EVENTUALLY_S(server.getPeers().size() == totalClients, 5);

    barrierExit.wait();
    for (auto& thread : threads) {
        thread.join();
    }
    threads.clear();

    REQUIRE_EVENTUALLY_S(server.getPeers().empty(), 5);
    REQUIRE(server.getReceivedCount() == totalSent);
}

TEST_CASE("Connect to UDP server and receive ping", "[Network]") {
    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";

    TestUdpServer server{config};
    TestUdpClient client{config};
    client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    // 5 seconds into the future
    const auto now = std::chrono::steady_clock::now().time_since_epoch() + std::chrono::seconds{5};
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    REQUIRE_EVENTUALLY_S(server.getPeers().size() == 1, 1);
    REQUIRE_EVENTUALLY_S(client->getLastPingTime() > nowMs, 10);

    auto peer = std::dynamic_pointer_cast<NetworkUdpPeer>(*server.getPeers().begin());
    REQUIRE(peer);
    REQUIRE_EVENTUALLY_S(peer->getLastPingTime() > nowMs, 10);
}

TEST_CASE("Connect to UDP server and disconnect from client", "[Network]") {
    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";

    TestUdpServer server{config};
    TestUdpClient client{config};
    client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    REQUIRE_EVENTUALLY_S(server.getPeers().size() == 1, 1);
    REQUIRE_EVENTUALLY_S(client.isAccepted(), 1);
    REQUIRE_EVENTUALLY_S(client->isEstablished(), 1);

    client->stop();

    REQUIRE_EVENTUALLY_S(server.getPeers().empty(), 5);
    REQUIRE_EVENTUALLY_S(!client.isAccepted(), 1);
    REQUIRE_EVENTUALLY_S(!client->isEstablished(), 1);
}

TEST_CASE("Connect to UDP server and disconnect from server", "[Network]") {
    Config config{};
    config.network.clientBindAddress = "::1";
    config.network.serverBindAddress = "::1";

    TestUdpServer server{config};
    TestUdpClient client{config};
    client->connect(server->getEndpoint().address().to_string(), server->getEndpoint().port());

    REQUIRE_EVENTUALLY_S(server.getPeers().size() == 1, 1);
    REQUIRE_EVENTUALLY_S(client.isAccepted(), 1);
    REQUIRE_EVENTUALLY_S(client->isEstablished(), 1);

    server->stop();

    REQUIRE_EVENTUALLY_S(!client.isAccepted(), 5);
    REQUIRE_EVENTUALLY_S(!client->isEstablished(), 5);
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

#include "../Common.hpp"
#include <Network/NetworkTcpAcceptor.hpp>
#include <Network/NetworkTcpClient.hpp>
#include <Network/NetworkTcpConnector.hpp>
#include <Network/NetworkTcpServer.hpp>

#define TAG "[NetworkClient]"

static constexpr int serverPort = 22443;

template <typename T> class EventListener : public Network::EventListener {
public:
    EventListener(T& parent) : parent(parent) {
    }

    ~EventListener() = default;

    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override {
        parent.eventPacket(stream, std::move(packet));
    }

    void eventConnect(const Network::StreamPtr& stream) override {
        parent.eventConnect(stream);
    }

    void eventDisconnect(const Network::StreamPtr& stream) override {
        parent.eventDisconnect(stream);
    }

    T& parent;
};

class DummyClient {
public:
    DummyClient() : listener(*this), client(std::make_shared<Network::TcpClient>(listener, "localhost", serverPort)) {
    }

    ~DummyClient() {
        client.reset();
    }

    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
        packets.push_back(std::move(packet));
    }

    void eventConnect(const Network::StreamPtr& stream) {
        connect = true;
    }

    void eventDisconnect(const Network::StreamPtr& stream) {
        connect = false;
    }

    template <typename T> void send(const T& message) {
        client->send(message);
    }

    EventListener<DummyClient> listener;
    std::shared_ptr<Network::Client> client;
    std::vector<Network::Packet> packets;
    bool connect = false;
};

class DummyServer {
public:
    DummyServer() : listener(*this), server(std::make_shared<Network::TcpServer>(listener, serverPort)) {
    }

    ~DummyServer() {
        server.reset();
    }

    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
        packets.emplace_back(stream, std::move(packet));
    }

    void eventConnect(const Network::StreamPtr& stream) {
        streams.insert(stream);
    }

    void eventDisconnect(const Network::StreamPtr& stream) {
        streams.erase(stream);
    }

    EventListener<DummyServer> listener;
    std::shared_ptr<Network::Server> server;
    std::vector<std::tuple<Network::StreamPtr, Network::Packet>> packets;
    std::set<Network::StreamPtr> streams;
};

struct DummyPacket {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

REGISTER_MESSAGE(DummyPacket);

TEST_CASE("Connect to the server and send one packet", TAG) {
    auto server = std::make_unique<DummyServer>();
    auto client = std::make_unique<DummyClient>();

    REQUIRE(waitForCondition([&]() { return server->streams.size() == 1; }));

    auto msg = DummyPacket{"Hello from client!"};
    client->send(msg);

    REQUIRE(waitForCondition([&]() { return server->packets.size() == 1; }));

    auto& [stream, packet] = server->packets.back();
    REQUIRE(stream.get() == server->streams.begin()->get());
    REQUIRE(packet.id == Network::getMessageId<DummyPacket>());

    auto unpacked = Network::unpack<DummyPacket>(packet);
    REQUIRE(unpacked.msg == msg.msg);

    msg = DummyPacket{"Hello from server!"};
    stream->send(msg);

    REQUIRE(waitForCondition([&]() { return client->packets.size() == 1; }));
    const auto& packet2 = client->packets.back();

    REQUIRE(packet2.id == Network::getMessageId<DummyPacket>());

    unpacked = Network::unpack<DummyPacket>(packet2);
    REQUIRE(unpacked.msg == msg.msg);
}

TEST_CASE("Close client", TAG) {
    auto server = std::make_unique<DummyServer>();
    auto client = std::make_unique<DummyClient>();

    REQUIRE(waitForCondition([&]() { return server->streams.size() == 1; }));

    client.reset();

    REQUIRE(waitForCondition([&]() { return server->streams.size() == 0; }));
}

TEST_CASE("Close server", TAG) {
    auto server = std::make_unique<DummyServer>();
    auto client = std::make_unique<DummyClient>();

    REQUIRE(waitForCondition([&]() { return server->streams.size() == 1; }));

    server.reset();

    REQUIRE(waitForCondition([&]() { return client->connect == false; }));
}

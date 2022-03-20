#include "../Common.hpp"
#include <TemporaryEscape/Network/NetworkMessage.hpp>
#include <TemporaryEscape/Network/NetworkTcpClient.hpp>
#include <TemporaryEscape/Network/NetworkTcpServer.hpp>
#include <TemporaryEscape/Utils/Worker.hpp>
#include <thread>

#define TAG "[NetworkClient]"

struct CustomHelloMessage {
    struct Request : Message {
        std::string msg;

        MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Message), msg);
    };

    struct Response : Message {
        std::string msg;

        MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Message), msg);
        MESSAGE_APPEND_DEFAULT();
        MESSAGE_COPY_DEFAULT();
    };
};

struct CustomEventMessage {
    using Request = NoMessage;

    struct Response : Message {
        uint64_t event{0};

        MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Message), event);
        MESSAGE_APPEND_DEFAULT();
        MESSAGE_COPY_DEFAULT();
    };
};

struct CustomLongMessage {
    struct Request : Message {
        MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Message));
    };

    struct Response : Message {
        std::vector<uint64_t> data;

        MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Message), data);
        MESSAGE_APPEND_ARRAY(data);
        MESSAGE_COPY_DEFAULT();
    };
};

using CustomMessageSink = NetworkMessageSink<CustomEventMessage, CustomHelloMessage, CustomLongMessage>;

class CustomServer : public NetworkTcpServer<CustomMessageSink> {
public:
    void onPeerConnected(std::shared_ptr<NetworkTcpPeer<CustomMessageSink>> peer) override {
        peers.push_back(peer);
    }

    void handle(const PeerPtr& peer, CustomEventMessage::Request req, CustomEventMessage::Response& res) override {
    }

    void handle(const PeerPtr& peer, CustomHelloMessage::Request req, CustomHelloMessage::Response& res) override {
        res.msg = req.msg + " World!";
    }

    void handle(const PeerPtr& peer, CustomLongMessage::Request req, CustomLongMessage::Response& res) override {
        auto idx = std::stoll(req.token);
        while (idx < 2100 && res.data.size() < 64) {
            res.data.push_back(idx++);
        }

        if (res.data.size() == 64) {
            res.token = std::to_string(res.data.back() + 1);
        }
    }

    std::vector<std::shared_ptr<NetworkTcpPeer<CustomMessageSink>>> peers;
};

class CustomClient : public NetworkTcpClient<CustomMessageSink> {
public:
    template <typename M> void send(M& message) {
        NetworkTcpClient::send(message);
    }

    void handle(CustomHelloMessage::Response res) {
        helloResReceived.store(true);
        helloRes = std::move(res);
    }

    void handle(CustomEventMessage::Response res) {
        eventResReceived.store(true);
        eventRes = std::move(res);
    }

    void handle(CustomLongMessage::Response res) {
        longResReceived.store(true);
        longRes = std::move(res);
    }

    CustomHelloMessage::Response helloRes;
    CustomEventMessage::Response eventRes;
    CustomLongMessage::Response longRes;
    std::atomic_bool helloResReceived{false};
    std::atomic_bool eventResReceived{false};
    std::atomic_bool longResReceived{false};
};

TEST_CASE("Connect to the server and send packets", TAG) {
    BackgroundWorker worker;
    CustomServer server;
    CustomClient client;

    IoServiceRunner clientThread(client.getWorker());

    server.bind(12345);
    client.connect("localhost", 12345);

    REQUIRE(waitForCondition([&]() { return server.peers.size() == 1; }));

    CustomHelloMessage::Request req{};
    req.msg = "Hello";
    client.send(req);

    REQUIRE(waitForCondition([&]() { return client.helloResReceived.load() == true; }));
    REQUIRE(client.helloRes.msg == "Hello World!");

    CustomEventMessage::Response res{};
    res.event = 123;
    server.peers.front()->send(res);

    REQUIRE(waitForCondition([&]() { return client.eventResReceived.load() == true; }));
    REQUIRE(client.eventRes.event == 123);

    CustomLongMessage::Request req2{};
    req2.token = "100";
    client.send(req2);

    REQUIRE(waitForCondition([&]() { return client.longResReceived.load() == true; }));
    REQUIRE(client.longRes.data.size() == 2000);
    for (size_t i = 0; i < client.longRes.data.size(); i++) {
        REQUIRE(client.longRes.data.at(i) == i + 100);
    }

    client.stop();
    server.stop();
}

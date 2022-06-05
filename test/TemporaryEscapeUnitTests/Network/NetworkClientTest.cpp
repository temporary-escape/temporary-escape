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

class CustomServer : public NetworkTcpServer<CustomServer, CustomMessageSink> {
public:
    CustomServer() : NetworkTcpServer(*this) {
    }

    void onPeerConnected(std::shared_ptr<NetworkTcpPeer<CustomServer, CustomMessageSink>> peer) override {
        peers.push_back(peer);
    }

    void handle(const PeerPtr& peer, CustomHelloMessage::Request req, CustomHelloMessage::Response& res) {
        res.msg = req.msg + " World!";
    }

    void handle(const PeerPtr& peer, CustomLongMessage::Request req, CustomLongMessage::Response& res) {
        auto idx = std::stoll(req.token);
        while (idx < 2100 && res.data.size() < 64) {
            res.data.push_back(idx++);
        }

        if (res.data.size() == 64) {
            res.token = std::to_string(res.data.back() + 1);
        }
    }

    std::vector<std::shared_ptr<NetworkTcpPeer<CustomServer, CustomMessageSink>>> peers;
};

class CustomClient : public NetworkTcpClient<CustomClient, CustomMessageSink> {
public:
    CustomClient() : NetworkTcpClient(*this) {
    }

    template <typename M, typename Fn> void send(M& message, Fn&& callback) {
        NetworkTcpClient<CustomClient, CustomMessageSink>::template send(message, std::forward<Fn>(callback));
    }

    void handle(CustomEventMessage::Response res) {
        eventResReceived.store(true);
        eventRes = std::move(res);
    }

    CustomEventMessage::Response eventRes;
    std::atomic_bool eventResReceived{false};
};

TEST_CASE("Connect to the server and close", TAG) {
    CustomServer server;
    CustomClient client;

    IoServiceRunner clientThread(client.getWorker());

    server.bind(12345);
    client.connect("localhost", 12345);

    REQUIRE(waitForCondition([&]() { return server.peers.size() == 1; }));

    client.stop();
    server.stop();
}

TEST_CASE("Connect to the server and send packets", TAG) {
    CustomServer server;
    CustomClient client;

    IoServiceRunner clientThread(client.getWorker());

    server.bind(12345);
    client.connect("localhost", 12345);

    REQUIRE(waitForCondition([&]() { return server.peers.size() == 1; }));

    SECTION("Send message and wait for response") {
        CustomHelloMessage::Request req{};
        CustomHelloMessage::Response res;
        std::atomic_bool received{false};

        req.msg = "Hello";

        client.send(req, [&](CustomHelloMessage::Response r) {
            res = std::move(r);
            received.store(true);
        });

        REQUIRE(waitForCondition([&]() { return received.load() == true; }));
        REQUIRE(res.msg == "Hello World!");
    }

    SECTION("Send event message from server") {
        CustomEventMessage::Response event{};
        event.event = 123;
        server.peers.front()->send(event);

        REQUIRE(waitForCondition([&]() { return client.eventResReceived.load() == true; }));
        REQUIRE(client.eventRes.event == 123);
    }

    SECTION("Send message and get paginated response") {
        CustomLongMessage::Request req{};
        CustomLongMessage::Response res{};
        std::atomic_bool received{false};

        req.token = "100";

        client.send(req, [&](CustomLongMessage::Response r) {
            res = std::move(r);
            received.store(true);
        });

        REQUIRE(waitForCondition([&]() { return received.load() == true; }));

        REQUIRE(res.data.size() == 2000);
        for (size_t i = 0; i < res.data.size(); i++) {
            REQUIRE(res.data.at(i) == i + 100);
        }
    }

    client.stop();
    server.stop();
}

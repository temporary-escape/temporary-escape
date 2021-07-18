#include "../Common.hpp"
#include <Network/NetworkClient.hpp>
#include <Network/NetworkServer.hpp>
#include <Network/NetworkTcpAcceptor.hpp>
#include <Network/NetworkTcpConnector.hpp>
#include <Network/NetworkUdpAcceptor.hpp>
#include <Network/NetworkUdpConnector.hpp>

static constexpr int ServerPort = 22443;

class ServerTest : public Network::Server {
public:
    ServerTest() : received(false) {
        addAcceptor<Network::TcpAcceptor>(ServerPort);
        addAcceptor<Network::UdpAcceptor>(ServerPort);
    }

    void dispatch(const Network::SessionPtr& session, Network::Packet packet) override {
        static int counter = 0;
        REQUIRE(++counter == 1);

        REQUIRE(packet.id == 123);
        REQUIRE(packet.data.size() == 10);

        REQUIRE(session == this->session);

        received = true;
    }

    Network::SessionPtr createSession(uint64_t uid, const std::string& name, const std::string& password) override {
        static int counter = 0;
        REQUIRE(++counter == 1);

        REQUIRE(uid == 1234);
        REQUIRE(name == "admin");
        REQUIRE(password == "admin");

        session = std::make_shared<Network::Session>(9876);
        return session;
    }

    void acceptSession(const Network::SessionPtr& session) override {
        static int counter = 0;
        REQUIRE(++counter == 1);

        REQUIRE(session == this->session);
    }

    std::shared_ptr<Network::Session> session;
    bool received;
};

class ClientTest : public Network::Client {
public:
    ClientTest() : Network::Client(0, "localhost"), received(false) {
        initConnection<Network::TcpConnector>(ServerPort, 1234, "admin", "admin");
        addConnection<Network::UdpConnector>(ServerPort);
    }

    void dispatch(Network::Packet packet) override {
        static int counter = 0;
        REQUIRE(++counter == 1);

        REQUIRE(packet.id == 124);
        REQUIRE(packet.data.size() == 12);

        received = true;
    }

    bool received;
};

TEST("Create client-server connection") {
    ServerTest server{};
    ClientTest client{};

    Network::Packet packet{};
    packet.id = 123;
    packet.sessionId = 9876;
    packet.data.write("1234567890", 10);

    client.send(0, packet);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(client.received == false);
    REQUIRE(server.received == true);

    packet = Network::Packet{};
    packet.id = 124;
    packet.sessionId = 9876;
    packet.data.write("123456789012", 12);

    server.session->send(1, packet);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    REQUIRE(client.received == true);
}

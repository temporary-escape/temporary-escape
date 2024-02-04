#pragma once

#include "NetworkStun.hpp"
#include "NetworkUdpConnection.hpp"

namespace Engine {
class ENGINE_API NetworkUdpClient : public std::enable_shared_from_this<NetworkUdpClient>, public NetworkUdpConnection {
public:
    NetworkUdpClient(const Config& config, asio::io_service& service);
    virtual ~NetworkUdpClient();

    void connect(const std::string& address, uint16_t port);
    void start();
    void stop();

    NetworkStunClient& getStunClient() {
        return stun;
    }

private:
    void sendPacket(const PacketBytesPtr& packet) override;
    void onConnected() override;
    std::shared_ptr<NetworkUdpConnection> makeShared() override;

    void receive();

    asio::ip::udp::socket socket;
    NetworkStunClient stun;
    asio::ip::udp::endpoint endpoint;
    asio::ip::udp::endpoint peerEndpoint;

    std::condition_variable connectedCv;
    std::mutex connectedLock;
    bool connected{false};
};
} // namespace Engine

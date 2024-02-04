#pragma once

#include "NetworkUdpConnection.hpp"

namespace Engine {
class ENGINE_API NetworkUdpPeer : public std::enable_shared_from_this<NetworkUdpPeer>, public NetworkUdpConnection {
public:
    NetworkUdpPeer(asio::io_service& service, asio::ip::udp::socket& socket, asio::ip::udp::endpoint endpoint);
    virtual ~NetworkUdpPeer();

    void sendHello();
    void close();
    const asio::ip::udp::endpoint& getEndpoint() const {
        return endpoint;
    }

    void onReceivePeer(const PacketBytesPtr& packet);

private:
    void sendPacket(const PacketBytesPtr& packet) override;
    void onConnected() override;
    std::shared_ptr<NetworkUdpConnection> makeShared() override;

    asio::ip::udp::socket& socket;
    asio::ip::udp::endpoint endpoint;
};
} // namespace Engine

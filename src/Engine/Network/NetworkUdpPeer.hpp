#pragma once

#include "NetworkDispatcher.hpp"
#include "NetworkUdpStream.hpp"

namespace Engine {
class ENGINE_API NetworkUdpPeer : public std::enable_shared_from_this<NetworkUdpPeer>, public NetworkUdpStream {
public:
    NetworkUdpPeer(asio::io_service& service, NetworkDispatcher2& dispatcher, asio::ip::udp::socket& socket,
                   asio::ip::udp::endpoint endpoint);
    virtual ~NetworkUdpPeer();

    void sendPublicKey();
    void close() override;
    const asio::ip::udp::endpoint& getEndpoint() const {
        return endpoint;
    }

    void onReceivePeer(const PacketBytesPtr& packet);

    const std::string& getAddress() const override {
        return address;
    }

private:
    void sendPacket(const PacketBytesPtr& packet) override;
    void onConnected() override;
    void onDisconnected() override;
    std::shared_ptr<NetworkUdpStream> makeShared() override;
    void onObjectReceived(msgpack::object_handle oh) override;

    asio::io_service& service;
    NetworkDispatcher2& dispatcher;
    asio::ip::udp::socket& socket;
    asio::ip::udp::endpoint endpoint;
    std::string address;
};
} // namespace Engine

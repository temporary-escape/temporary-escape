#pragma once

#include "NetworkUtils.hpp"

namespace Engine {
class ENGINE_API NetworkUdpPeer : public std::enable_shared_from_this<NetworkUdpPeer> {
public:
    NetworkUdpPeer(asio::io_service& service, asio::ip::udp::socket& socket, asio::ip::udp::endpoint endpoint);
    virtual ~NetworkUdpPeer();

    void sendHello();
    void close();
    const asio::ip::udp::endpoint& getEndpoint() const {
        return endpoint;
    }

private:
    asio::io_service& service;
    asio::ip::udp::socket& socket;
    asio::ip::udp::endpoint endpoint;
    asio::io_service::strand strand;
    std::array<uint8_t, 1400> buffer{};
    std::string helloMsg;
};
} // namespace Engine

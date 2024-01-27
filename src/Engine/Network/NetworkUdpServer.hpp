#pragma once

#include "NetworkStun.hpp"
#include "NetworkUdpPeer.hpp"
#include <queue>

namespace Engine {
class ENGINE_API NetworkUdpServer {
public:
    explicit NetworkUdpServer(const Config& config, asio::io_service& service);
    virtual ~NetworkUdpServer();
    void receive();
    void stop();
    const asio::ip::udp::endpoint& getEndpoint() const {
        return localEndpoint;
    }

    void stunRequest(NetworkStunClient::Callback callback) {
        stun.send(std::move(callback));
    }

private:
    const Config& config;
    asio::io_service& service;
    asio::io_service::strand strand;
    asio::ip::udp::socket socket;
    NetworkStunClient stun;
    asio::ip::udp::endpoint localEndpoint;
    asio::ip::udp::endpoint peerEndpoint;
    std::array<uint8_t, 1400> buffer{};
    std::mutex mutex;
    std::unordered_map<asio::ip::udp::endpoint, std::shared_ptr<NetworkUdpPeer>> peers{};
};
} // namespace Engine

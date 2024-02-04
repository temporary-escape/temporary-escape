#pragma once

#include "NetworkStun.hpp"
#include "NetworkUdpPeer.hpp"
#include <queue>

namespace Engine {
class ENGINE_API NetworkUdpServer {
public:
    explicit NetworkUdpServer(const Config& config, asio::io_service& service, NetworkDispatcher2& dispatcher);
    virtual ~NetworkUdpServer();

    using NotifyCallback = std::function<void()>;

    void start();
    void stop();

    const asio::ip::udp::endpoint& getEndpoint() const {
        return localEndpoint;
    }

    NetworkStunClient& getStunClient() {
        return stun;
    }

    void notifyClientConnection(const std::string& address, uint16_t port, NotifyCallback callback);

private:
    void receive();
    PacketBytesPtr allocatePacket();

    const Config& config;
    asio::io_service& service;
    NetworkDispatcher2& dispatcher;
    asio::io_service::strand strand;
    asio::ip::udp::socket socket;
    NetworkStunClient stun;
    asio::ip::udp::endpoint localEndpoint;
    asio::ip::udp::endpoint peerEndpoint;

    std::mutex mutex;
    std::unordered_map<asio::ip::udp::endpoint, std::shared_ptr<NetworkUdpPeer>> peers{};

    std::mutex packetPoolMutex;
    MemoryPool<PacketBytes, 64 * 1024> packetPool{};
};
} // namespace Engine

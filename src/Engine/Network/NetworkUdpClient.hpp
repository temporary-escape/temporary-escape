#pragma once

#include "NetworkDispatcher.hpp"
#include "NetworkStun.hpp"
#include "NetworkUdpStream.hpp"

namespace Engine {
class ENGINE_API NetworkUdpClient : public std::enable_shared_from_this<NetworkUdpClient>, public NetworkUdpStream {
public:
    NetworkUdpClient(const Config& config, asio::io_service& service, NetworkDispatcher2& dispatcher);
    virtual ~NetworkUdpClient();

    void connect(const std::string& address, uint16_t port);
    void start();
    void stop();

    NetworkStunClient& getStunClient() {
        return stun;
    }

private:
    void stopInternal();
    void sendPacket(const PacketBytesPtr& packet) override;
    void onConnected() override;
    void onDisconnected() override;
    std::shared_ptr<NetworkUdpStream> makeShared() override;
    void onObjectReceived(msgpack::object_handle oh) override;

    void receive();

    NetworkDispatcher2& dispatcher;
    asio::ip::udp::socket socket;
    NetworkStunClient stun;
    asio::ip::udp::endpoint endpoint;
    asio::ip::udp::endpoint peerEndpoint;

    std::condition_variable connectedCv;
    std::mutex connectedLock;
    bool connected{false};
};
} // namespace Engine

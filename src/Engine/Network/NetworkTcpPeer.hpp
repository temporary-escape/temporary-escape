#pragma once

#include "NetworkDispatcher.hpp"
#include "NetworkPeer.hpp"
#include "NetworkUtils.hpp"

namespace Engine {
using Socket = asio::ip::tcp::socket;

class ENGINE_API NetworkTcpServer;

class ENGINE_API NetworkTcpPeer : public NetworkPeer, public std::enable_shared_from_this<NetworkTcpPeer> {
public:
    explicit NetworkTcpPeer(asio::io_service& service, NetworkTcpServer& server, Socket socket,
                            NetworkDispatcher& dispatcher);
    virtual ~NetworkTcpPeer();

    void receive();
    void close() override;
    bool isConnected() const override;
    const std::string& getAddress() const override {
        return address;
    }

protected:
    void receiveObject(msgpack::object_handle oh) override;
    void writeCompressed(const char* data, size_t length) override;

private:
    asio::io_service& service;
    asio::io_service::strand strand;
    NetworkTcpServer* server;
    Socket socket;
    NetworkDispatcher& dispatcher;
    std::string address;
    bool closed{false};
    std::array<char, 4096> buffer{};
};
} // namespace Engine

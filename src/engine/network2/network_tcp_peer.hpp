#pragma once

#include "../library.hpp"
#include "network_dispatcher.hpp"
#include "network_utils.hpp"

namespace Engine {
using Socket = asio::ssl::stream<asio::ip::tcp::socket>;

class ENGINE_API NetworkTcpPeer : public std::enable_shared_from_this<NetworkTcpPeer> {
public:
    explicit NetworkTcpPeer(asio::io_service& service, Socket socket, NetworkDispatcher& dispatcher);
    virtual ~NetworkTcpPeer();

    void handshake();
    void close();
    bool isConnected() const;
    const std::string& getAddress() const {
        return address;
    }

private:
    void receive();

    asio::io_service& service;
    asio::io_service::strand strand;
    Socket socket;
    NetworkDispatcher& dispatcher;
    std::string address;
    bool closed{false};
    std::array<char, 4096> buffer{};
};
} // namespace Engine

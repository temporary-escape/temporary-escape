#pragma once

#include "network_ssl_context.hpp"
#include "network_tcp_peer.hpp"
#include <asio.hpp>

namespace Engine {
class ENGINE_API NetworkTcpServer {
public:
    explicit NetworkTcpServer(asio::io_service& service, NetworkSslContext& ssl, NetworkDispatcher& dispatcher,
                              uint32_t port, bool ipv6);
    virtual ~NetworkTcpServer();
    void accept();
    void stop();

private:
    static asio::ip::tcp::endpoint endpoint(uint32_t port, bool ipv6);

    asio::io_service& service;
    NetworkSslContext& ssl;
    NetworkDispatcher& dispatcher;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<NetworkTcpPeer>> peers;
};
} // namespace Engine

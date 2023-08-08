#pragma once

#include "network_ssl_context.hpp"
#include "network_tcp_peer.hpp"
#include <asio.hpp>

namespace Engine {
class ENGINE_API NetworkTcpClient {
public:
    explicit NetworkTcpClient(asio::io_service& service, NetworkSslContext& ssl, const std::string& host,
                              uint32_t port);
    virtual ~NetworkTcpClient();

    void close();
    bool isConnected() const;
    const std::string& getAddress() {
        return address;
    }

private:
    void connect(const std::string& host, uint32_t port, std::chrono::milliseconds timeout);
    void receive();

    asio::io_service& service;
    asio::io_service::strand strand;
    std::shared_ptr<Socket> socket;
    std::string address;
    std::array<char, 4096> buffer{};
};
} // namespace Engine

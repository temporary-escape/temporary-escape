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
    
    bool isConnected() const {
        return internal && internal->isConnected();
    }
    const std::string& getAddress() {
        return internal->getAddress();
    }

private:
    class Internal : public std::enable_shared_from_this<Internal> {
    public:
        Internal(asio::io_service& service, NetworkSslContext& ssl);
        void close();
        void connect(const std::string& host, uint32_t port, std::chrono::milliseconds timeout);
        void receive();
        bool isConnected() const;
        const std::string& getAddress() {
            return address;
        }

    private:
        asio::io_service& service;
        asio::io_service::strand strand;
        Socket socket;
        std::string address;
        std::array<char, 4096> buffer{};
    };

    std::shared_ptr<Internal> internal;
};
} // namespace Engine

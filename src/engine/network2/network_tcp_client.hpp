#pragma once

#include "network_ssl_context.hpp"
#include "network_tcp_peer.hpp"
#include <asio.hpp>

namespace Engine {
class ENGINE_API NetworkTcpClient {
public:
    explicit NetworkTcpClient(asio::io_service& service, NetworkDispatcher& dispatcher, const std::string& host,
                              uint32_t port);
    virtual ~NetworkTcpClient();

    void close();

    bool isConnected() const {
        return internal && internal->isConnected();
    }
    const std::string& getAddress() {
        return internal->getAddress();
    }

    template <typename T> void send(const T& msg, const uint64_t xid) {
        internal->send(msg, xid);
    }

private:
    class Internal : public NetworkPeer, public std::enable_shared_from_this<Internal> {
    public:
        Internal(asio::io_service& service, NetworkDispatcher& dispatcher);
        void close();
        void connect(const std::string& host, uint32_t port, std::chrono::milliseconds timeout);
        void receive();
        bool isConnected() const override;
        const std::string& getAddress() const override {
            return address;
        }

    protected:
        void receiveObject(msgpack::object_handle oh) override;
        void writeCompressed(const char* data, size_t length) override;

    private:
        asio::io_service& service;
        NetworkDispatcher& dispatcher;
        asio::io_service::strand strand;
        Socket socket;
        std::string address;
        std::array<char, 4096> buffer{};
    };

    std::shared_ptr<Internal> internal;
};
} // namespace Engine

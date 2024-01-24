#pragma once

#include "NetworkUtils.hpp"

namespace Engine {
class ENGINE_API NetworkUdpClient {
public:
    NetworkUdpClient(asio::io_service& service, const std::string& address, uint16_t port);
    virtual ~NetworkUdpClient();

private:
    void connect(const std::string& address, uint16_t port);

    asio::io_service& service;
    std::unique_ptr<asio::ip::udp::socket> socket;
    asio::io_service::strand strand;
    asio::ip::udp::endpoint endpoint;
    std::array<uint8_t, 1400> buffer{};
};
} // namespace Engine

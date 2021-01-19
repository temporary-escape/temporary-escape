#pragma once

#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include <memory>

namespace Scissio::Network {
class Acceptor;

class SCISSIO_API UdpStream : public Stream, public std::enable_shared_from_this<UdpStream> {
public:
    UdpStream(Acceptor& acceptor, asio::ip::udp::socket& socket, asio::ip::udp::endpoint endpoint);
    virtual ~UdpStream() = default;

    void sendRaw(const Packet& packet) override;

    void disconnect() override;
    void receive(Packet packet);

private:
    Acceptor& acceptor;
    asio::ip::udp::socket& socket;
    asio::ip::udp::endpoint endpoint;
};
} // namespace Scissio::Network

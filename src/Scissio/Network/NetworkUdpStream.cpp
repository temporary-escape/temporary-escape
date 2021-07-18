#include "NetworkUdpStream.hpp"
#include "../Utils/Log.hpp"
#include "NetworkUdpAcceptor.hpp"

static constexpr int WINDOW_SIZE = 1024;

using namespace Scissio;

Network::UdpStream::UdpStream(Acceptor& acceptor, asio::ip::udp::socket& socket, asio::ip::udp::endpoint endpoint)
    : acceptor(acceptor), socket(socket), endpoint(std::move(endpoint)) {
}

void Network::UdpStream::sendRaw(const Packet& packet) {
    auto sbuffer = std::make_shared<msgpack::sbuffer>();
    msgpack::pack(*sbuffer, packet);

    auto self = shared_from_this();
    const auto b = asio::buffer(sbuffer->data(), sbuffer->size());
    self->socket.async_send_to(b, endpoint, [self, sbuffer](const asio::error_code ec, const size_t length) {
        (void)self;

        if (ec) {
            Log::e("Network UDP stream async_send_to error: {}", ec.message());
        } else {
            //Log::d("Network UDP stream sent: {} bytes", length);
        }
    });
}

void Network::UdpStream::disconnect() {
}

void Network::UdpStream::receive(Packet packet) {
    acceptor.receive(shared_from_this(), std::move(packet));
}

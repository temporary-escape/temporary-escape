#include "NetworkUdpConnector.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "NetworkClient.hpp"
#include <future>
#include <iostream>

using namespace Scissio;

static constexpr int WINDOW_SIZE = 1024;

Network::UdpConnector::UdpConnector(Client& client, asio::io_service& service, const std::string& address,
                                    const int port)
    : client(client), socket(service, asio::ip::udp::endpoint(asio::ip::udp::v6(), 0)) {

    const asio::ip::udp::resolver::query query(address, std::to_string(port));
    asio::ip::udp::resolver resolver(service);

    std::future<asio::ip::udp::resolver::results_type> endpoints = resolver.async_resolve(query, asio::use_future);
    if (endpoints.wait_for(std::chrono::milliseconds(2000)) != std::future_status::ready) {
        EXCEPTION("Failed to resolve server address");
    }

    endpoint = *endpoints.get();

    stream = std::make_shared<UdpStream>(*this, socket, endpoint);

    client.addStream(stream);
}

Network::UdpConnector::~UdpConnector() {
    close();
}

void Network::UdpConnector::close() {
    stream.reset();
}

void Network::UdpConnector::start() {
    accept();
}

void Network::UdpConnector::receive(const StreamPtr& stream, const Packet& packet) {
    client.receive(stream, packet);
}

void Network::UdpConnector::accept() {
    unp.reserve_buffer(WINDOW_SIZE);
    const auto b = asio::buffer(unp.buffer(), WINDOW_SIZE);
    auto self = shared_from_this();

    auto endpoint = std::make_shared<asio::ip::udp::endpoint>();

    self->socket.async_receive_from(b, *endpoint, [self, endpoint](const asio::error_code ec, const size_t length) {
        if (ec) {
            Log::e("Network UDP connector async_receive_from error: {}", ec.message());
        } else {
            if (length == WINDOW_SIZE) { // ???
                self->unp.reset();
            } else {
                self->unp.buffer_consumed(length);
                msgpack::object_handle oh;
                while (self->unp.next(oh)) {
                    try {
                        Packet packet;
                        oh.get().convert(packet);
                        self->receive(self->stream, std::move(packet));
                    } catch (std::exception& e) {
                        Log::e("Network UDP connector msgpack error: {}", e.what());
                    }
                }
            }
            self->accept();
        }
    });
}

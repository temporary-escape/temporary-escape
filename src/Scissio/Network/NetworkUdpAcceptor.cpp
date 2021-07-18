#include "NetworkUdpAcceptor.hpp"
#include "../Utils/Log.hpp"
#include "NetworkServer.hpp"
#include "NetworkUdpStream.hpp"
#include "Packet.hpp"

static constexpr int WINDOW_SIZE = 1024;

using namespace Scissio;

Network::UdpAcceptor::UdpAcceptor(Server& server, asio::io_service& service, const int port)
    : server(server), service(service), socket(service, asio::ip::udp::endpoint(asio::ip::udp::v6(), port)) {
}

Network::UdpAcceptor::~UdpAcceptor() {
    close();
}

void Network::UdpAcceptor::start() {
    accept();
    Log::i("Network UDP acceptor started on: {}", endpoint.address().to_string());
}

void Network::UdpAcceptor::close() {
    for (auto& pair : streams) {
        pair.second->disconnect();
    }
}

void Network::UdpAcceptor::accept() {
    unp.reserve_buffer(WINDOW_SIZE);
    const auto b = asio::buffer(unp.buffer(), WINDOW_SIZE);
    auto self = shared_from_this();

    auto endpoint = std::make_shared<asio::ip::udp::endpoint>();

    self->socket.async_receive_from(b, *endpoint, [self, endpoint](const asio::error_code ec, const size_t length) {
        if (ec) {
            Log::e("Network UDP acceptor async_receive_from error: {}", ec.message());
        } else {
            self->unp.buffer_consumed(length);
            msgpack::object_handle oh;
            while (self->unp.next(oh)) {
                try {
                    Packet packet;
                    oh.get().convert(packet);
                    // Log::d("Network UDP stream accepted packet id: {}", packet.id);
                    self->getStream(*endpoint)->receive(std::move(packet));
                } catch (std::exception& e) {
                    Log::e("Network UDP acceptor msgpack error: {}", e.what());
                }
            }
            self->accept();
        }
    });
}

std::shared_ptr<Network::UdpStream> Network::UdpAcceptor::getStream(const asio::ip::udp::endpoint& endpoint) {
    auto found = streams.find(endpoint);
    if (found == streams.end()) {
        Log::v("New UDP connection from: [{}]:{}", endpoint.address().to_string(), endpoint.port());
        found = streams.insert(std::make_pair(endpoint, std::make_shared<UdpStream>(*this, socket, endpoint))).first;
    }

    return found->second;
}

void Network::UdpAcceptor::receive(const StreamPtr& stream, Packet packet) {
    const auto packetId = packet.id;
    const auto sessionId = packet.sessionId;

    try {
        server.receive(stream, std::move(packet));
    } catch (std::exception& e) {
        Log::e("Failed to accept packet id: {} session: {}", packetId, sessionId);
        backtrace(e);
    }
}

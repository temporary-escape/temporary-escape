#include "NetworkTcpAcceptor.hpp"
#include "../Utils/Log.hpp"
#include "NetworkServer.hpp"
#include "NetworkTcpStream.hpp"

#define CMP "NetworkTcpAcceptor"

using namespace Scissio;

Network::TcpAcceptor::TcpAcceptor(EventListener& listener, Crypto::Ecdhe& ecdhe, asio::io_service& service,
                                  const int port)
    : Acceptor(listener), ecdhe(ecdhe), acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port)),
      socket(service), endpoint(acceptor.local_endpoint()) {
}

Network::TcpAcceptor::~TcpAcceptor() {
}

void Network::TcpAcceptor::close() {
    Log::i(CMP, "Closing");
    socket.close();
    std::lock_guard<std::mutex> lock{mutex};
    for (auto& stream : streams) {
        stream->disconnect();
    }
    streams.clear();
}

void Network::TcpAcceptor::start() {
    accept();
    Log::i(CMP, "Started on: [{}]:{}", endpoint.address().to_string(), endpoint.port());
}

void Network::TcpAcceptor::accept() {
    auto self = shared_from_this();
    acceptor.async_accept(socket, [self](const std::error_code ec) {
        if (!ec) {
            if (!self->acceptor.is_open()) {
                return;
            }

            try {
                const auto peer = std::make_shared<TcpStream>(*self, self->ecdhe, std::move(self->socket));
                peer->receive();
                peer->sendPublicKey();

                std::lock_guard<std::mutex> lock{self->mutex};
                self->streams.push_back(peer);
            } catch (std::exception& e) {
                Log::e(CMP, "async_accept error: {}", e.what());
            }
        }

        self->accept();
    });
}

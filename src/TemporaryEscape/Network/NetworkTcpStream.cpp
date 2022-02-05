#include "NetworkTcpStream.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "NetworkAcceptor.hpp"

#define CMP "NetworkTcpStream"

static constexpr int WINDOW_SIZE = 1024;

using namespace Engine;

Network::TcpStream::TcpStream(Acceptor& acceptor, Crypto::Ecdhe& ecdhe, asio::ip::tcp::socket socket)
    : Stream(ecdhe), acceptor(acceptor), flag(true), socket(std::move(socket)), endpoint(this->socket.remote_endpoint()), unp{} {

    Log::i(CMP, "New TCP connection from: [{}]:{}", this->socket.remote_endpoint().address().to_string(),
           this->socket.remote_endpoint().port());
}

Network::TcpStream::~TcpStream() {
    TcpStream::disconnect();
}

void Network::TcpStream::disconnect() {
    if (socket.is_open()) {
        flag.store(false);
        Log::i(CMP, "Disconnecting address: {}", endpoint.address().to_string());
        socket.close();
        // acceptor.removePeer(shared_from_this());
    }
    unp.reset();
}

void Network::TcpStream::receive() {
    if (!flag.load()) {
        return;
    }

    unp.reserve_buffer(WINDOW_SIZE);
    const auto b = asio::buffer(unp.buffer(), WINDOW_SIZE);
    auto self = shared_from_this();
    self->socket.async_read_some(b, [self](const asio::error_code ec, const size_t length) {
        if (ec) {
            Log::e(CMP, "async_read_some error: {}", ec.message());

            if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                if (self->flag.load()) {
                    self->disconnect();
                    self->acceptor.eventDisconnect(self);
                }
            }
        } else {
            self->unp.buffer_consumed(length);
            msgpack::object_handle oh;
            while (self->unp.next(oh)) {
                try {
                    Packet packet;
                    oh.get().convert(packet);
                    // Log::d(CMP, "Network TCP stream accepted packet id: {}", packet.id);

                    if (packet.id == 0) {
                        self->acceptPublicKey(packet);
                        self->acceptor.eventConnect(self);
                    } else {
                        self->decrypt(packet);
                        self->acceptor.eventPacket(self, std::move(packet));
                    }
                } catch (std::exception& e) {
                    BACKTRACE(CMP, e, "error accepting packet");
                }
            }
            self->receive();
        }
    });
}

void Network::TcpStream::sendRaw(const Packet& packet) {
    auto sbuffer = std::make_shared<msgpack::sbuffer>();
    msgpack::pack(*sbuffer, packet);

    auto self = shared_from_this();
    const auto b = asio::buffer(sbuffer->data(), sbuffer->size());
    self->socket.async_write_some(b, [self, sbuffer](const asio::error_code ec, const size_t length) {
        (void)self;

        if (ec) {
            Log::e(CMP, "async_write_some error: {}", ec.message());
        } else {
            // Log::d("Network TCP stream sent: {} bytes", length);
        }
    });
}

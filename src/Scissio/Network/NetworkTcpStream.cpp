#include "NetworkTcpStream.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "NetworkAcceptor.hpp"

static constexpr int WINDOW_SIZE = 1024;

using namespace Scissio;

Network::TcpStream::TcpStream(Acceptor& acceptor, asio::ip::tcp::socket socket)
    : acceptor(acceptor), socket(std::move(socket)), endpoint(this->socket.remote_endpoint()), unp{} {

    Log::v("New TCP connection from: [{}]:{}", this->socket.remote_endpoint().address().to_string(),
           this->socket.remote_endpoint().port());
}

void Network::TcpStream::disconnect() {
    if (socket.is_open()) {
        Log::v("Network TCP stream disconnecting address: {}", endpoint.address().to_string());
        socket.close();
        // acceptor.removePeer(shared_from_this());
    }
}

void Network::TcpStream::receive() {
    unp.reserve_buffer(WINDOW_SIZE);
    const auto b = asio::buffer(unp.buffer(), WINDOW_SIZE);
    auto self = shared_from_this();
    self->socket.async_read_some(b, [self](const asio::error_code ec, const size_t length) {
        if (ec) {
            Log::e("Network TCP stream async_read_some error: {}", ec.message());
        } else {
            self->unp.buffer_consumed(length);
            msgpack::object_handle oh;
            while (self->unp.next(oh)) {
                try {
                    Packet packet;
                    oh.get().convert(packet);
                    //Log::d("Network TCP stream accepted packet id: {}", packet.id);
                    self->acceptor.receive(self, std::move(packet));
                } catch (std::exception& e) {
                    BACKTRACE(e, "Network TCP stream error");
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
            Log::e("Network TCP stream async_write_some error: {}", ec.message());
        } else {
            //Log::d("Network TCP stream sent: {} bytes", length);
        }
    });
}

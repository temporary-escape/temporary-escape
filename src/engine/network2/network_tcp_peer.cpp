#include "network_tcp_peer.hpp"
#include "../utils/exceptions.hpp"
#include "network_tcp_server.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkTcpPeer::NetworkTcpPeer(asio::io_service& service, NetworkTcpServer& server, Socket socket,
                               NetworkDispatcher& dispatcher) :
    service{service}, strand{service}, server{server}, socket{std::move(socket)}, dispatcher{dispatcher} {

    this->socket.lowest_layer().set_option(asio::ip::tcp::no_delay{true});
    address = fmt::format("{}", this->socket.lowest_layer().remote_endpoint());
}

NetworkTcpPeer::~NetworkTcpPeer() {
    close();
}

void NetworkTcpPeer::close() {
    if (!closed && socket.lowest_layer().is_open()) {
        closed = true;
        logger.info("Closing peer endpoint: {}", address);

        auto self = shared_from_this();
        server.disconnect(self);
        self->socket.async_shutdown(self->strand.wrap([self](const std::error_code ec) {
            (void)ec;
            self->socket.lowest_layer().close();
        }));
    }
}

void NetworkTcpPeer::handshake() {
    auto self = shared_from_this();

    socket.async_handshake(asio::ssl::stream_base::server, strand.wrap([self](const std::error_code ec) {
        if (ec) {
            logger.error("Failed to perform handshake from: {} error: {}", self->address, ec.message());
            self->close();
            self->service.post([self]() { self->dispatcher.onDisconnect(self); });
        } else {
            logger.info("Handshake success connection from: {}", self->address);
            self->service.post([self]() { self->dispatcher.onAcceptSuccess(self); });
            self->receive();
        }
    }));
}

void NetworkTcpPeer::receive() {
    const auto b = asio::buffer(buffer.data(), buffer.size());
    auto self = this->shared_from_this();

    socket.async_read_some(b, strand.wrap([self](const asio::error_code ec, const size_t length) {
        if (ec) {
            logger.error("Failed to read data from: {} error: {}", self->address, ec.message());
            self->close();
            self->service.post([self]() { self->dispatcher.onDisconnect(self); });
        } else {
            try {
                self->DecompressionAcceptor::accept(self->buffer.data(), length);
            } catch (std::exception& e) {
                BACKTRACE(e, "Failed to consume data");
            }

            self->receive();
        }
    }));
}

bool NetworkTcpPeer::isConnected() const {
    return socket.lowest_layer().is_open();
}

void NetworkTcpPeer::receiveObject(msgpack::object_handle oh) {
    if (!Detail::validateMessageObject(oh)) {
        logger.error("Received malformed message from: {}", address);
    } else {
        auto o = std::make_shared<decltype(oh)>(std::move(oh));
        auto self = this->shared_from_this();
        service.post([self, o]() { self->dispatcher.onObjectReceived(self, o); });
    }
}

void NetworkTcpPeer::writeCompressed(const char* data, size_t length) {
    if (!isConnected()) {
        return;
    }

    auto self = shared_from_this();
    auto temp = std::make_shared<std::vector<char>>(length);
    std::memcpy(temp->data(), data, length);
    auto b = asio::buffer(temp->data(), temp->size());

    socket.async_write_some(b, strand.wrap([self](const asio::error_code ec, const size_t length) {
        if (ec) {
            logger.error("Failed to write data to: {} error: {}", self->address, ec.message());
            self->close();
            self->service.post([self]() { self->dispatcher.onDisconnect(self); });
        }
    }));
}
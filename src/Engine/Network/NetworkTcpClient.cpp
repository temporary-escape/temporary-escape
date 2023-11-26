#include "NetworkTcpClient.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkTcpClient::NetworkTcpClient(asio::io_service& service, NetworkDispatcher& dispatcher, const std::string& host,
                                   const uint32_t port) :
    internal{std::make_shared<Internal>(service, dispatcher)} {

    logger.info("Connecting to host: {} port: {}", host, port);
    internal->connect(host, port, std::chrono::milliseconds{5000});

    logger.info("Connected to host: {}", internal->getAddress());
}

NetworkTcpClient::~NetworkTcpClient() {
    close();
}

NetworkTcpClient::Internal::Internal(asio::io_service& service, NetworkDispatcher& dispatcher) :
    service{service}, dispatcher{dispatcher}, strand{service}, socket{service} {
}

void NetworkTcpClient::close() {
    if (internal) {
        internal->close();
    }
    internal.reset();
}

void NetworkTcpClient::Internal::close() {
    if (!closing && socket.is_open()) {
        closing = true;
        auto self = shared_from_this();
        strand.post([self] {
            if (self->socket.is_open()) {
                logger.info("Closing connection to: {}", self->address);
                asio::error_code ec;
                (void)self->socket.close(ec);
            }
        });
    }
}

void NetworkTcpClient::Internal::connect(const std::string& host, const uint32_t port,
                                         const std::chrono::milliseconds timeout) {
    const auto tp = std::chrono::system_clock::now() + timeout;

    const asio::ip::tcp::resolver::query query(host, std::to_string(port));
    asio::ip::tcp::resolver resolver{service};

    auto endpointsFuture = resolver.async_resolve(query, asio::use_future);
    if (endpointsFuture.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Failed to resolve address");
    }

    const auto endpoints = endpointsFuture.get();

    auto connect = asio::async_connect(socket, endpoints, asio::use_future);
    if (connect.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Timeout connecting to the address");
    }
    connect.get();

    address = fmt::format("{}", socket.remote_endpoint());
    receive();
}

void NetworkTcpClient::Internal::receive() {
    auto self = shared_from_this();
    const auto b = asio::buffer(buffer.data(), buffer.size());

    socket.async_read_some(b, strand.wrap([self](const asio::error_code ec, const size_t length) {
        if (ec) {
            if (!isAsioEofError(ec)) {
                logger.error("Failed to read data from: {} error: {}", self->address, ec.message());
            }
            self->close();
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

bool NetworkTcpClient::Internal::isConnected() const {
    return socket.is_open();
}

void NetworkTcpClient::Internal::receiveObject(msgpack::object_handle oh) {
    if (!Detail::validateMessageObject(oh)) {
        logger.error("Received malformed message from: {}", address);
    } else {
        auto o = std::make_shared<decltype(oh)>(std::move(oh));

        auto self = this->shared_from_this();
        service.post([self, o]() { self->dispatcher.onObjectReceived(self, o); });
    }
}

void NetworkTcpClient::Internal::writeCompressed(const char* data, size_t length) {
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
        }
    }));
}

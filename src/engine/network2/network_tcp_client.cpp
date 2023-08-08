#include "network_tcp_client.hpp"
#include "../utils/exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkTcpClient::NetworkTcpClient(asio::io_service& service, NetworkSslContext& ssl, const std::string& host,
                                   const uint32_t port) :
    service{service}, strand{service}, socket{std::make_shared<Socket>(service, ssl.get())} {

    logger.info("Connecting to host: {} port: {}", host, port);
    connect(host, port, std::chrono::milliseconds{5000});

    logger.info("Connected to host: {}", address);
}

NetworkTcpClient::~NetworkTcpClient() {
    close();
}

void NetworkTcpClient::close() {
    if (socket && socket->lowest_layer().is_open()) {
        logger.info("Closing connection to: {}", address);
        socket->lowest_layer().close();
    }
    socket.reset();
}

void NetworkTcpClient::connect(const std::string& host, const uint32_t port, const std::chrono::milliseconds timeout) {
    const auto tp = std::chrono::system_clock::now() + timeout;

    const asio::ip::tcp::resolver::query query(host, std::to_string(port));
    asio::ip::tcp::resolver resolver{service};

    auto endpointsFuture = resolver.async_resolve(query, asio::use_future);
    if (endpointsFuture.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Failed to resolve address");
    }

    const auto endpoints = endpointsFuture.get();

    auto connect = asio::async_connect(socket->lowest_layer(), endpoints, asio::use_future);
    if (connect.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Timeout connecting to the address");
    }
    connect.get();

    auto handshake = socket->async_handshake(asio::ssl::stream_base::client, asio::use_future);
    if (handshake.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Timeout TLS handshake");
    }
    handshake.get();

    address = fmt::format("{}", socket->lowest_layer().remote_endpoint());
    receive();
}

void NetworkTcpClient::receive() {
    const auto b = asio::buffer(buffer.data(), buffer.size());

    socket->async_read_some(b, strand.wrap([=](const asio::error_code ec, const size_t length) {
        (void)socket;

        if (ec) {
            logger.error("Failed to read data from: {} error: {}", address, ec.message());
        } else {
            try {
                // Receive
            } catch (std::exception& e) {
                BACKTRACE(e, "Failed to consume data");
            }

            receive();
        }
    }));
}

bool NetworkTcpClient::isConnected() const {
    return socket && socket->lowest_layer().is_open();
}

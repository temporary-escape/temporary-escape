#include "NetworkUdpPeer.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpPeer::NetworkUdpPeer(asio::io_service& service, asio::ip::udp::socket& socket,
                               asio::ip::udp::endpoint endpoint) :
    service{service}, strand{service}, socket{socket}, endpoint{endpoint} {

    logger.info("UDP peer created on address: {} to remote: {}", socket.local_endpoint(), endpoint);
    helloMsg = fmt::format("{}", endpoint);
}

NetworkUdpPeer::~NetworkUdpPeer() {
}

void NetworkUdpPeer::close() {
    auto self = shared_from_this();
    strand.post([self]() {
        asio::error_code ec;
        (void)self->socket.close(ec);
        if (ec) {
            logger.error("UDP peer close error: {}", ec.message());
        }
    });
}

void NetworkUdpPeer::sendHello() {
    auto self = shared_from_this();
    auto buff = asio::buffer(helloMsg.data(), helloMsg.size());
    socket.async_send_to(buff, endpoint, strand.wrap([self](const asio::error_code ec, const size_t sent) {
        if (ec) {
            logger.error("UDP peer failed to send to remote: {} error: {}", self->endpoint, ec.message());
            self->close();
        } else {
            logger.info("UDP peer sent hello to remote: {}", self->endpoint);
        }
    }));
}

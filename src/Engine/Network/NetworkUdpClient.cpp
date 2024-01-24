#include "NetworkUdpClient.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpClient::NetworkUdpClient(asio::io_service& service, const std::string& address, const uint16_t port) :
    service{service}, strand{service} {
    connect(address, port);
}

NetworkUdpClient::~NetworkUdpClient() {
}

void NetworkUdpClient::connect(const std::string& address, const uint16_t port) {
    logger.info("Connecting to address: {} port: {}", address, port);

    const auto tp = std::chrono::system_clock::now() + std::chrono::seconds{5};

    const asio::ip::udp::resolver::query query(address, std::to_string(port));
    asio::ip::udp::resolver resolver{service};

    auto future = resolver.async_resolve(query, asio::use_future);
    if (future.wait_until(tp) != std::future_status::ready) {
        EXCEPTION("Failed to resolve address: {}", address);
    }

    const auto endpoints = future.get();

    static const std::string_view msg = "Hello World!";

    for (const auto& e : endpoints) {
        socket = std::make_unique<asio::ip::udp::socket>(service, asio::ip::udp::endpoint{asio::ip::udp::v6(), 0});

        logger.debug("Trying remote address: {}", e.endpoint());
        {
            const auto buff = asio::buffer(msg.data(), msg.size());
            auto f = socket->async_send_to(buff, e.endpoint(), asio::use_future);
            if (f.wait_until(tp) != std::future_status::ready) {
                EXCEPTION("Timeout connecting to address: {}", address);
            }

            (void)f.get();
        }

        {
            const auto buff = asio::buffer(buffer.data(), buffer.size());
            asio::ip::udp::endpoint test;
            auto f = socket->async_receive_from(buff, test, asio::use_future);

            // Timeout?
            if (f.wait_for(std::chrono::seconds{1}) != std::future_status::ready) {
                socket->close();
                socket.reset();
                continue;
            }

            const auto received = f.get();
            endpoint = test;
            std::string resp{reinterpret_cast<const char*>(buffer.data()), received};
            logger.info("Received reply from the server: {}", resp);
            break;
        }
    }

    if (socket) {
        logger.info("Connected UDP client to address: \"{}\"", endpoint);
    } else {
        EXCEPTION("Unable to connect to address: {}", address);
    }
}

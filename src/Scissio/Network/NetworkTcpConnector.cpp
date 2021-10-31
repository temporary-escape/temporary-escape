#include "NetworkTcpConnector.hpp"
#include "../Utils/Exceptions.hpp"
#include "NetworkClient.hpp"
#include <future>

#define CMP "NetworkTcpConnector"

using namespace Scissio;

Network::TcpConnector::TcpConnector(Client& client, asio::io_service& service, const std::string& address,
                                    const int port)
    : client(client) {

    const asio::ip::tcp::resolver::query query(address, std::to_string(port));
    asio::ip::tcp::resolver resolver(service);

    std::future<asio::ip::tcp::resolver::results_type> endpoints = resolver.async_resolve(query, asio::use_future);
    if (endpoints.wait_for(std::chrono::milliseconds(2000)) != std::future_status::ready) {
        EXCEPTION("Failed to resolve server address");
    }

    asio::ip::tcp::socket socket(service);

    const auto connect = socket.async_connect(*endpoints.get(), asio::use_future);
    if (connect.wait_for(std::chrono::milliseconds(2000)) != std::future_status::ready) {
        EXCEPTION("Failed to connect to the server");
    }

    endpoint = socket.remote_endpoint();
    stream = std::make_shared<TcpStream>(*this, std::move(socket));
    stream->receive();

    client.eventConnect(stream);
}

Network::TcpConnector::~TcpConnector() {
    close();
}

void Network::TcpConnector::close() {
    stream.reset();
}

void Network::TcpConnector::start() {
}

void Network::TcpConnector::eventPacket(const StreamPtr& stream, Packet packet) {
    const auto packetId = packet.id;

    try {
        client.eventPacket(stream, std::move(packet));
    } catch (std::exception& e) {
        Log::e(CMP, "Network TCP connector failed to accept packet id: {}", packetId);
        backtrace(e);
    }
}

void Network::TcpConnector::eventDisconnect(const StreamPtr& stream) {
    try {
        client.eventDisconnect(stream);
    } catch (std::exception& e) {
        Log::e(CMP, "Network TCP connector failed to disconnect stream");
        backtrace(e);
    }
}

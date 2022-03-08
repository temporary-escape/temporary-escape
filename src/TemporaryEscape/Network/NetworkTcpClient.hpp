#pragma once

#include "../Future.hpp"
#include "NetworkMessage.hpp"
#include "NetworkTcpConnector.hpp"

namespace Engine {
template <typename Sink> class ENGINE_API NetworkTcpClient : public Sink::template Client<NetworkTcpConnector<Sink>> {
public:
    using Connector = NetworkTcpConnector<Sink>;
    using ConnectorPtr = std::shared_ptr<Connector>;

    NetworkTcpClient() = default;
    virtual ~NetworkTcpClient() = default;

    void connect(const std::string& address, uint16_t port) {
        Log::i(CMP, "asio service started!");
        work = std::make_unique<asio::io_service::work>(service);
        thread = std::thread([this]() {
            service.run();
            Log::i(CMP, "asio service stopped!");
        });

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

        connector = std::make_shared<Connector>(*this, ecdhe, std::move(socket));
        connector->start();
        connector->sendPublicKey();

        auto future = connected.future();
        if (future.waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
            EXCEPTION("Failed to connect to server {}:{}", address, port);
        }

        future.get();
    }

    void stop() {
        Log::i(CMP, "asio service stopping!");
        if (connector) {
            connector->close();
        }
        connector.reset();
        if (thread.joinable()) {
            service.post([this]() { work.reset(); });
            service.stop();
            thread.join();
        }
    }

    void onReceive(Packet packet) {
        try {
            Sink::template Client<Connector>::dispatch(worker, connector, std::move(packet));
        } catch (...) {
            EXCEPTION_NESTED("Failed to dispatch packet id: {}", packet.id);
        }
    }

    template <typename M> void send(M& message) {
        message.id = Sink::template Client<Connector>::nextRequestId();
        connector->send(message);
    }

    void onConnected() {
        connected.resolve();
    }

    asio::io_service& getWorker() {
        return worker;
    }

private:
    static inline const char* CMP = "NetworkTcpClient";

    asio::io_service worker;
    Crypto::Ecdhe ecdhe;
    std::thread thread;
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    ConnectorPtr connector;
    Promise<void> connected;
};

template <typename Sink> void NetworkTcpConnector<Sink>::onConnected() {
    client.onConnected();
}

template <typename Sink> void NetworkTcpConnector<Sink>::onReceive(Packet packet) {
    client.onReceive(std::move(packet));
}
} // namespace Engine

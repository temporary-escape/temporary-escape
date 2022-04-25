#pragma once

#include "../Future.hpp"
#include "NetworkMessage.hpp"
#include "NetworkTcpConnector.hpp"

namespace Engine {
template <typename Handler, typename Sink> class ENGINE_API NetworkTcpClient : public Sink::Client {
public:
    using Connector = NetworkTcpConnector<Handler, Sink>;
    using ConnectorPtr = std::shared_ptr<Connector>;

    explicit NetworkTcpClient(Handler& handler) : handler(handler) {
    }
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
            Sink::Client::dispatch(worker, handler, connector, std::move(packet));
        } catch (...) {
            EXCEPTION_NESTED("Failed to dispatch packet id: {}", packet.id);
        }
    }

    template <typename M, typename Fn> void send(M& message, Fn&& callback) {
        Sink::Client::allocateRequestData(std::forward<Fn>(callback), message.id);
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

    Handler& handler;
    asio::io_service worker;
    Crypto::Ecdhe ecdhe;
    std::thread thread;
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    ConnectorPtr connector;
    Promise<void> connected;
};

template <typename Handler, typename Sink> void NetworkTcpConnector<Handler, Sink>::onConnected() {
    client.onConnected();
}

template <typename Handler, typename Sink> void NetworkTcpConnector<Handler, Sink>::onReceive(Packet packet) {
    client.onReceive(std::move(packet));
}
} // namespace Engine

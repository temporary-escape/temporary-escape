#pragma once

#include "../Utils/Worker.hpp"
#include "NetworkMessage.hpp"
#include "NetworkTcpPeer.hpp"

namespace Engine {
template <typename Sink> class ENGINE_API NetworkTcpServer : public Sink::template Server<NetworkTcpPeer<Sink>> {
public:
    using Peer = NetworkTcpPeer<Sink>;
    using PeerPtr = std::shared_ptr<Peer>;
    using PeerWeakPtr = std::weak_ptr<Peer>;

    NetworkTcpServer() = default;
    virtual ~NetworkTcpServer() = default;

    void stop() {
        acceptor.reset();
        if (thread.joinable()) {
            service.stop();
            thread.join();
        }
    }

    void bind(uint16_t port) {
        acceptor =
            std::make_unique<asio::ip::tcp::acceptor>(service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
        accept();

        Log::i(CMP, "asio service started!");
        thread = std::thread([this]() {
            service.run();
            Log::i(CMP, "asio service stopped!");
        });
    }

    virtual void onPeerConnected(PeerPtr peer) = 0;

    void onPeerReceive(PeerPtr peer, const Packet& packet) {
        try {
            Sink::template Server<Peer>::dispatch(worker.getService(), peer, packet);
        } catch (...) {
            EXCEPTION_NESTED("Failed to dispatch packet id: {}", packet.id);
        }
    }

    BackgroundWorker& getWorker() {
        return worker;
    }

private:
    static inline const char* CMP = "NetworkTcpServer";

    void accept() {
        socket = std::make_unique<asio::ip::tcp::socket>(service);
        acceptor->async_accept(*socket, std::bind(&NetworkTcpServer::onAccept, this, std::placeholders::_1));
    }

    void onAccept(std::error_code ec) {
        if (ec) {
            Log::e(CMP, "async_accept error: {}", ec.message());
        } else {
            if (!acceptor->is_open()) {
                return;
            }

            try {
                auto peer = std::make_shared<Peer>(*this, ecdhe, std::move(*socket));
                peer->start();
                peer->sendPublicKey();
            } catch (std::exception& e) {
                Log::e(CMP, "async_accept error: {}", e.what());
            }

            socket = std::make_unique<asio::ip::tcp::socket>(service);
            accept();
        }
    }

    BackgroundWorker worker;
    Crypto::Ecdhe ecdhe;
    std::thread thread;
    asio::io_service service;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
    std::unique_ptr<asio::ip::tcp::socket> socket;
};

template <typename Sink> void NetworkTcpPeer<Sink>::onConnected() {
    server.onPeerConnected(std::dynamic_pointer_cast<NetworkTcpPeer<Sink>>(NetworkTcpStream<Sink>::shared_from_this()));
}

template <typename Sink> void NetworkTcpPeer<Sink>::onReceive(Packet packet) {
    server.onPeerReceive(std::dynamic_pointer_cast<NetworkTcpPeer<Sink>>(NetworkTcpStream<Sink>::shared_from_this()),
                         std::move(packet));
}

} // namespace Engine

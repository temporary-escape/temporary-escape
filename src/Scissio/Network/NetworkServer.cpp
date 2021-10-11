#include "NetworkServer.hpp"
#include "../Utils/Log.hpp"

using namespace Scissio;

Network::Server::Server() {
    startIoService();
}

Network::Server::~Server() {
    try {
        stopIoService();
    } catch (std::exception& e) {
        Log::e("Network server stop error: {}", e.what());
    }
}

void Network::Server::startIoService() {
    work = std::make_unique<asio::io_service::work>(service);
    Log::i("Network server asio service started!");
    thread = std::thread([this]() {
        service.run();
        Log::w("Network server asio service stopped!");
    });
}

void Network::Server::stopIoService() {
    acceptor.reset();
    if (thread.joinable()) {
        work.reset();
        service.stop();
        thread.join();
    }
}

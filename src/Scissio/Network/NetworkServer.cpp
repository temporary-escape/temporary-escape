#include "NetworkServer.hpp"
#include "../Utils/Log.hpp"

#define CMP "NetworkServer"

using namespace Scissio;

Network::Server::Server(EventListener& listener) : listener(listener) {
    startIoService();
}

Network::Server::~Server() {
    stopIoService();
}

void Network::Server::startIoService() {
    work = std::make_unique<asio::io_service::work>(service);
    Log::i(CMP, "asio service started!");
    thread = std::thread([this]() {
        service.run();
        Log::w(CMP, "asio service stopped!");
    });
}

void Network::Server::stopIoService() {
    if (thread.joinable()) {
        service.post([this]() { work.reset(); });
        service.stop();
        thread.join();
    }
}

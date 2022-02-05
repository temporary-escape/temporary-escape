#include "NetworkClient.hpp"
#include "../Utils/Log.hpp"

#define CMP "NetworkClient"

using namespace Engine;

Network::Client::Client(EventListener& listener) : listener(listener) {
    startIoService();
}

Network::Client::~Client() {
    stopIoService();
}

void Network::Client::startIoService() {
    Log::i(CMP, "asio service started!");
    work = std::make_unique<asio::io_service::work>(service);
    thread = std::thread([this]() {
        service.run();
        Log::w(CMP, "asio service stopped!");
    });
}

void Network::Client::stopIoService() {
    Log::i(CMP, "asio service stopping!");
    if (thread.joinable()) {
        service.post([this]() { work.reset(); });
        service.stop();
        thread.join();
    }
}

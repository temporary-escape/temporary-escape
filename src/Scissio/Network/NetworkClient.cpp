#include "NetworkClient.hpp"
#include "../Utils/Log.hpp"

#define CMP "NetworkClient"

using namespace Scissio;

Network::Client::Client() {
    startIoService();
}

Network::Client::~Client() {
    try {
        stopIoService();
    } catch (std::exception& e) {
        Log::e("Network client stop error: {}", e.what());
    }
}

void Network::Client::startIoService() {
    Log::i(CMP, "AbstractClient network asio service started!");
    work = std::make_unique<asio::io_service::work>(service);
    thread = std::thread([this]() {
        service.run();
        Log::w(CMP, "AbstractClient network asio service stopped!");
    });
}

void Network::Client::stopIoService() {
    stream.reset();
    acceptor.reset();
    if (thread.joinable()) {
        work.reset();
        service.stop();
        thread.join();
    }
}

void Network::Client::eventConnect(const StreamPtr& stream) {
    this->stream = stream;
}

void Network::Client::eventDisconnect(const StreamPtr& stream) {
    (void)stream;
    this->stream.reset();
}

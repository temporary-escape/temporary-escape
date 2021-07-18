#include "NetworkClient.hpp"
#include "../Utils/Log.hpp"

using namespace Scissio;

Network::Client::Client(const uint64_t sessionId, const std::string& address) : Session(sessionId), address(address) {
    startIoService();
}

Network::Client::~Client() {
    try {
        stopIoService();
    } catch (std::exception& e) {
        Log::e("Network client stop error: {}", e.what());
    }
}

void Network::Client::close() {
    disconnect();
    stopIoService();
}

void Network::Client::startIoService() {
    Log::i("AbstractClient network asio service started!");
    work = std::make_unique<asio::io_service::work>(service);
    thread = std::thread([this]() {
        service.run();
        Log::w("AbstractClient network asio service stopped!");
    });
}

void Network::Client::stopIoService() {
    if (thread.joinable()) {
        work.reset();
        Session::disconnect();
        service.stop();
        thread.join();
    }
}

void Network::Client::receive(const StreamPtr& stream, Packet packet) {
    (void)stream;

    // Utils::Log::d("AbstractClient received packet: {}", packet.id);

    switch (packet.id) {
    case getMessageId<MessageSessionInitResponse>(): {
        {
            std::lock_guard<std::mutex> lock{mutex};
            auto message = Details::MessageUnpacker<MessageSessionInitResponse>::unpack(packet);

            if (!message.error.empty()) {
                error = message.error;
            } else {
                Session::setSessionId(message.sessionId);
                Log::d("Setting session id to: {}", message.sessionId);
            }
        }

        cvConnection.notify_one();
        break;
    }
    case getMessageId<MessageSessionConnect>(): {
        {
            std::lock_guard<std::mutex> lock{mutex};
            auto message = Details::MessageUnpacker<MessageSessionConnect>::unpack(packet);
            state = message.state;
            Log::d("Setting state to: {}", state);
        }

        cvConnection.notify_one();
        break;
    }
    default: {
        dispatch(std::move(packet));
        break;
    }
    }
}

void Network::Client::waitForSession() {
    Log::d("Waiting for session response...");

    std::unique_lock<std::mutex> lock{mutex};
    if (cvConnection.wait_for(lock, std::chrono::milliseconds(2000)) == std::cv_status::timeout) {
        EXCEPTION("Failed to connect server: response timeout");
    }

    if (!error.empty()) {
        EXCEPTION("Failed to connect server responded with: {}", error);
    }
}

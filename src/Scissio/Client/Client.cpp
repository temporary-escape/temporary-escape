#include "Client.hpp"
#include "../Network/NetworkTcpClient.hpp"
#include "../Server/Messages.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

#define DISPATCH_FUNC(M, T, F) std::bind(static_cast<void (T::*)(M)>(&T::F), this, std::placeholders::_1)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Client, handle));

using namespace Scissio;

uint64_t generatePlayerUid(const Config& config) {
    const auto path = config.userdataPath / Path("uid");
    if (Fs::exists(path)) {
        std::ifstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for reading", path.string());
        }

        const std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        return std::stoull(str);
    } else {
        const auto uid = randomId();

        std::ofstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for writing", path.string());
        }

        f << uid;

        return uid;
    }
}

Client::Client(Config& config, const std::string& address, const int port)
    : listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpClient>(*listener, address, port)), secret(generatePlayerUid(config)) {

    MESSAGE_DISPATCH(MessageLoginResponse);
}

Client::~Client() {
}

void Client::eventPacket(Network::Packet packet) {
    try {
        dispatcher.dispatch(packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void Client::eventConnect() {
}

void Client::eventDisconnect() {
}

void Client::login(const std::string& password) {
    MessageLoginRequest req{};
    req.secret = secret;
    req.name = "Some Player";
    req.password = password;

    network->send(req);

    auto future = loginPromise.future();
    future.get(std::chrono::milliseconds(1000));
}

void Client::handle(MessageLoginResponse req) {
    if (!req.error.empty()) {
        loginPromise.reject<std::runtime_error>(req.error);
    } else {
        loginPromise.resolve();
    }
}

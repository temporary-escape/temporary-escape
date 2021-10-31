#include "Client.hpp"
#include "../Server/Messages.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

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
    : Network::TcpClient(address, port), uid(generatePlayerUid(config)) {

    MessageLoginRequest req{};
    req.uid = uid;
    req.name = "Some Player";
    req.password = "password";
    send(req);
}

Client::~Client() {
}

void Client::eventPacket(const Network::StreamPtr& stream, Network::Packet packet) {
}

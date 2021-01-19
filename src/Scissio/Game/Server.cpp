#include "Server.hpp"

#include "../Network/NetworkTcpAcceptor.hpp"
#include "../Network/NetworkUdpAcceptor.hpp"

#include <random>

using namespace Scissio;

Server::Server(const Config& config, AssetManager& assetManager) : config(config), assetManager(assetManager) {
    addAcceptor<Network::TcpAcceptor>(config.serverPort);
    addAcceptor<Network::UdpAcceptor>(config.serverPort);
}

Server::~Server() {
}

void Server::handle(const PlayerPtr& player, MessageClientHello message) {
    MessageServerHello hello;
    hello.name = "Server Name";
    hello.version = {1, 0, 0};
    player->send(0, hello);
}

#define MESSAGE_DISPATCH(T)                                                                                            \
    case T::KIND: {                                                                                                    \
        this->handle(std::dynamic_pointer_cast<Player>(session), Scissio::Network::Details::unpack<T>(packet.data));   \
        break;                                                                                                         \
    }

void Server::dispatch(const Network::SessionPtr& session, const Network::Packet& packet) {
    switch (packet.id) {
        MESSAGE_DISPATCH(MessageClientHello)
    default: {
        Log::e("Client cannot accept unknown packet id: {}", packet.id);
    }
    }
}

Network::SessionPtr Server::createSession(int64_t playerId, const std::string& password) {
    std::unique_lock<std::shared_mutex> lock{playerMutex};

    union SessionId {
        uint8_t bytes[8];
        int64_t value;
    };

    std::mt19937_64 rng;
    std::uniform_int_distribution<int> dist(0, 0xFF);

    SessionId id;

    while (true) {
        for (auto i = 0; i < 8; i++) {
            id.bytes[i] = dist(rng);
        }

        const auto pred = [&](PlayerPtr& p) { return p->getSessionId() == id.value; };
        if (std::find_if(players.begin(), players.end(), pred) == players.end()) {
            break;
        }
    }

    auto player = std::make_shared<Player>(id.value, playerId);

    Log::i("Creating network session for player uid: {}", player->getPlaterId());
    playersLobby.push_back(std::move(player));

    return playersLobby.back();
}

void Server::acceptSession(const Network::SessionPtr& session) {
    std::unique_lock<std::shared_mutex> lock{playerMutex};

    auto player = std::dynamic_pointer_cast<Player>(session);

    const auto it = std::find(playersLobby.begin(), playersLobby.end(), player);
    if (it != playersLobby.end()) {
        playersLobby.erase(it);

        Log::i("Accepting network session for player uid: {}", player->getPlaterId());
        players.push_back(std::move(player));
    }
}

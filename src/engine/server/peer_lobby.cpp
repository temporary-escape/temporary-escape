#include "peer_lobby.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PeerLobby::PeerLobby(const Config& config) : config{config} {
}

void PeerLobby::addPeer(const NetworkPeerPtr& peer) {
    logger.info("Adding to lobby peer: {}", peer->getAddress());
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};
    lobby.map.insert(peer);
    logger.info("New peer in lobby: {}", peer->getAddress());
}

void PeerLobby::removePeer(const NetworkPeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};

    const auto it = lobby.map.find(peer);
    if (it != lobby.map.end()) {
        logger.info("Removing from lobby peer: {}", peer->getAddress());
        lobby.map.erase(it);
    }
}

void PeerLobby::disconnectPeer(const NetworkPeerPtr& peer) {
    removePeer(peer);
    logger.info("Disconnecting peer: {}", peer->getAddress());
    peer->close();
}

void PeerLobby::clear() {
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};
    for (const auto& peer : lobby.map) {
        peer->close();
    }
    lobby.map.clear();
}

std::vector<NetworkPeerPtr> PeerLobby::getAllPeers() {
    std::shared_lock<std::shared_mutex> lock{lobby.mutex};
    std::vector<NetworkPeerPtr> peers;
    peers.resize(lobby.map.size());
    for (auto& peer : lobby.map) {
        peers.push_back(peer);
    }
    return peers;
}

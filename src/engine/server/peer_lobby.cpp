#include "peer_lobby.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PeerLobby::PeerLobby(const Config& config) : config{config} {
}

void PeerLobby::addPeer(const NetworkPeerPtr& peer) {
    logger.info("Adding to lobby peer: {}", peer->getAddress());
    std::unique_lock<std::shared_mutex> lock{mutex};
    map.insert(peer);
    logger.info("New peer in lobby: {}", peer->getAddress());
}

void PeerLobby::removePeer(const NetworkPeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{mutex};

    const auto it = map.find(peer);
    if (it != map.end()) {
        logger.info("Removing from lobby peer: {}", peer->getAddress());
        map.erase(it);
    }
}

void PeerLobby::disconnectPeer(const NetworkPeerPtr& peer) {
    removePeer(peer);
    logger.info("Disconnecting peer: {}", peer->getAddress());
    peer->close();
}

void PeerLobby::clear() {
    std::unique_lock<std::shared_mutex> lock{mutex};
    for (const auto& peer : map) {
        peer->close();
    }
    map.clear();
}

std::vector<NetworkPeerPtr> PeerLobby::getAllPeers() {
    std::shared_lock<std::shared_mutex> lock{mutex};
    std::vector<NetworkPeerPtr> peers;
    peers.resize(map.size());
    for (auto& peer : map) {
        peers.push_back(peer);
    }
    return peers;
}

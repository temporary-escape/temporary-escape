#include "peer_lobby.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PeerLobby::PeerLobby(const Config& config) : config{config} {
}

void PeerLobby::addPeer(const NetworkPeerPtr& peer) {
    removePeer(peer);

    logger.info("Adding to lobby peer: {}", peer->getAddress());
    std::unique_lock<std::shared_mutex> lock{mutex};
    peers.push_back(peer);
    logger.info("New peer in lobby: {}", peer->getAddress());
}

void PeerLobby::removePeer(const NetworkPeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{mutex};

    const auto it = std::find_if(peers.begin(), peers.end(), [&](NetworkPeerPtr& p) { return p.get() == peer.get(); });

    if (it != peers.end()) {
        logger.info("Removing from lobby peer: {}", peer->getAddress());
        peers.erase(it);
    }
}

void PeerLobby::disconnectPeer(const NetworkPeerPtr& peer) {
    removePeer(peer);
    logger.info("Disconnecting peer: {}", peer->getAddress());
    peer->close();
}

void PeerLobby::clear() {
    std::unique_lock<std::shared_mutex> lock{mutex};
    for (const auto& peer : peers) {
        peer->close();
    }
    peers.clear();
}

std::vector<NetworkPeerPtr> PeerLobby::getAllPeers() {
    std::shared_lock<std::shared_mutex> lock{mutex};
    return peers;
}

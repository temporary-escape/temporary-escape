#include "peer_lobby.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PeerLobby::PeerLobby(const Config& config) : config{config} {
}

void PeerLobby::addPeerToLobby(const PeerPtr& peer) {
    logger.info("Adding to lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};
    lobby.map.insert(peer.get());
}

void PeerLobby::removePeerFromLobby(const PeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};

    const auto it = lobby.map.find(peer.get());
    if (it != lobby.map.end()) {
        logger.info("Removing from lobby peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
        lobby.map.erase(it);
    }
}

void PeerLobby::disconnectPeer(const PeerPtr& peer) {
    removePeerFromLobby(peer);
    logger.info("Disconnecting peer id: {}", reinterpret_cast<uint64_t>(peer.get()));
    peer->close();
}

void PeerLobby::clear() {
    std::unique_lock<std::shared_mutex> lock{lobby.mutex};
    for (const auto& peer : lobby.map) {
        peer->close();
    }
    lobby.map.clear();
}

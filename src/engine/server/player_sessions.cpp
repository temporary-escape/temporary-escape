#include "player_sessions.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PlayerSessions::PlayerSessions(const Config& config, Database& db) : config{config}, db{db} {
}

SessionPtr PlayerSessions::createSession(const NetworkPeerPtr& peer, const std::string& playerId) {
    logger.info("Creating session peer: {} player: {}", peer->getAddress(), playerId);
    std::unique_lock<std::shared_mutex> lock{mutex};
    auto session = std::make_shared<Session>(playerId, peer);
    map.insert(std::make_pair(peer.get(), session));
    return session;
}

SessionPtr PlayerSessions::getSession(const std::string& playerId) {
    std::shared_lock<std::shared_mutex> lock{mutex};
    for (const auto& pair : map) {
        if (pair.second->getPlayerId() == playerId) {
            return pair.second;
        }
    }
    return nullptr;
}

SessionPtr PlayerSessions::getSession(const NetworkPeerPtr& peer) {
    std::shared_lock<std::shared_mutex> lock{mutex};
    const auto it = map.find(peer.get());
    if (it == map.end()) {
        EXCEPTION("Player session not found for peer: {}", peer->getAddress());
    }
    return it->second;
}

std::vector<SessionPtr> PlayerSessions::getAllSessions() {
    std::shared_lock<std::shared_mutex> lock{mutex};
    std::vector<SessionPtr> res;
    res.reserve(map.size());
    for (const auto& [_, peer] : map) {
        res.push_back(peer);
    }
    return res;
}

/*void PlayerSessions::updateSessionsPing() {
    std::shared_lock<std::shared_mutex> lock{sessions.mutex};
    const auto now = std::chrono::steady_clock::now();
    for (const auto& [_, session] : sessions.map) {
        if (!session->hasFlag(Session::Flags::PingSent)) {
            session->setFlag(Session::Flags::PingSent);

            MessagePingRequest req{};
            req.time = now;
            session->send(req);
        }
    }
}*/

void PlayerSessions::removeSession(const NetworkPeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{mutex};
    if (const auto it = map.find(peer.get()); it != map.end()) {
        logger.info("Removing session peer: {} player: {}", peer->getAddress(), it->second->getPlayerId());
        map.erase(it);
    }
    if (const auto it = locations.find(peer.get()); it != locations.end()) {
        locations.erase(it);
    }
}

bool PlayerSessions::isLoggedIn(const std::string& playerId) {
    std::shared_lock<std::shared_mutex> lock{mutex};
    for (const auto& [_, session] : map) {
        if (session->getPlayerId() == playerId) {
            return true;
        }
    }

    return false;
}

void PlayerSessions::clear() {
    std::unique_lock<std::shared_mutex> lock{mutex};
    for (const auto& [_, session] : map) {
        session->close();
    }
    map.clear();
    locations.clear();
}

std::optional<std::string> PlayerSessions::getLocation(const SessionPtr& session) {
    std::shared_lock<std::shared_mutex> lock{mutex};
    if (const auto it = locations.find(session->getStream().get()); it != locations.end()) {
        return it->second;
    }
    return std::nullopt;
}

void PlayerSessions::setLocation(const SessionPtr& session, const std::optional<std::string>& sectorId) {
    std::unique_lock<std::shared_mutex> lock{mutex};
    if (sectorId) {
        locations.emplace(session->getStream().get(), *sectorId);
    } else if (const auto it = locations.find(session->getStream().get()); it != locations.end()) {
        locations.erase(it);
    }
}

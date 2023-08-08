#include "player_sessions.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

PlayerSessions::PlayerSessions(const Config& config, Database& db) : config{config}, db{db} {
}

std::optional<std::string> PlayerSessions::secretToId(const uint64_t secret) {
    auto player = db.getByIndex<&PlayerData::secret>(secret);
    if (!player.empty()) {
        return player.front().id;
    }

    return std::nullopt;
}

PlayerData PlayerSessions::login(const uint64_t secret, const std::string& name) {
    // Find the player in the database using its secret
    // to get us a primary key (id)
    const auto found = db.getByIndex<&PlayerData::secret>(secret);
    std::string playerId;
    if (found.empty()) {
        // No such player found, generate one
        playerId = uuid();
    } else {
        playerId = found.front().id;
    }

    // Update the player data or create a new player
    auto result = db.update<PlayerData>(playerId, [&](std::optional<PlayerData> player) {
        if (!player) {
            logger.info("Registering new player: '{}'", name);
            player = PlayerData{};
            player->id = playerId;
            player->secret = secret;
            player->admin = true;
        }

        player->name = name;
        return player.value();
    });

    return result;
}

void PlayerSessions::createSession(const PeerPtr& peer, const std::string& playerId) {
    logger.info("Creating session peer id: {} player: {}", reinterpret_cast<uint64_t>(peer.get()), playerId);
    std::unique_lock<std::shared_mutex> lock{sessions.mutex};
    auto session = std::make_shared<Session>(playerId, peer);
    sessions.map.insert(std::make_pair(peer.get(), session));
}

SessionPtr PlayerSessions::getSession(const std::string& playerId) {
    std::shared_lock<std::shared_mutex> lock{sessions.mutex};
    for (const auto& pair : sessions.map) {
        if (pair.second->getPlayerId() == playerId) {
            return pair.second;
        }
    }
    return nullptr;
}

SessionPtr PlayerSessions::getSession(const PeerPtr& peer) {
    std::shared_lock<std::shared_mutex> lock{sessions.mutex};
    const auto it = sessions.map.find(peer.get());
    if (it == sessions.map.end()) {
        EXCEPTION("Player session not found for peer: {}", reinterpret_cast<uint64_t>(peer.get()));
    }
    return it->second;
}

std::vector<SessionPtr> PlayerSessions::getAllSessions() {
    std::shared_lock<std::shared_mutex> lock{sessions.mutex};
    std::vector<SessionPtr> res;
    res.reserve(sessions.map.size());
    for (const auto& [_, peer] : sessions.map) {
        res.push_back(peer);
    }
    return res;
}

void PlayerSessions::updateSessionsPing() {
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
}

void PlayerSessions::removeSession(const PeerPtr& peer) {
    std::unique_lock<std::shared_mutex> lock{sessions.mutex};
    const auto it = sessions.map.find(peer.get());
    if (it != sessions.map.end()) {
        logger.info("Removing session peer id: {} player: {}",
                    reinterpret_cast<uint64_t>(peer.get()),
                    it->second->getPlayerId());
        sessions.map.erase(it);
    }
}

bool PlayerSessions::isLoggedIn(const std::string& playerId) {
    std::shared_lock<std::shared_mutex> lock{sessions.mutex};
    for (const auto& [_, session] : sessions.map) {
        if (session->getPlayerId() == playerId) {
            return true;
        }
    }

    return false;
}

void PlayerSessions::clear() {
    std::unique_lock<std::shared_mutex> lock{sessions.mutex};
    for (const auto& [_, session] : sessions.map) {
        session->close();
    }
    sessions.map.clear();
}

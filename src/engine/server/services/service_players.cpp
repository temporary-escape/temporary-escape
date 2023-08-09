#include "service_players.hpp"
#include "../../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServicePlayers::ServicePlayers(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {
}

ServicePlayers::~ServicePlayers() = default;

std::optional<std::string> ServicePlayers::secretToId(const uint64_t secret) {
    auto player = db.getByIndex<&PlayerData::secret>(secret);
    if (!player.empty()) {
        return player.front().id;
    }

    return std::nullopt;
}

PlayerData ServicePlayers::login(const uint64_t secret, const std::string& name) {
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

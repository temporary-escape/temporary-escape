#include "ServicePlayers.hpp"
#include "../../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServicePlayers::ServicePlayers(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST2(MessagePlayerLocationRequest);
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

PlayerLocationData ServicePlayers::getSpawnLocation(const std::string& id) {
    const auto found = db.find<PlayerData>(id);
    if (!found) {
        EXCEPTION("No such player id: {}", id);
    }

    // Does the player already have a spawn location?
    auto location = db.find<PlayerLocationData>(id);
    if (location) {
        return *location;
    }

    // Find the available spawn locations
    // TODO: Choose the right location for the player
    const auto startingLocations = db.seekAll<StartingLocationData>("", 1);
    if (startingLocations.empty()) {
        EXCEPTION("No starting locations found for player id: {}", id);
    }

    location = PlayerLocationData{};
    location->galaxyId = startingLocations.front().galaxyId;
    location->systemId = startingLocations.front().systemId;
    location->sectorId = startingLocations.front().sectorId;

    db.put<PlayerLocationData>(id, *location);

    return *location;
}

void ServicePlayers::handle(Request2<MessagePlayerLocationRequest> req) {
    MessagePlayerLocationResponse res{};

    const auto session = sessions.getSession(req.peer);
    const auto location = db.find<PlayerLocationData>(session->getPlayerId());
    if (location) {
        res.location = *location;
    } else {
        logger.warn("Player location requested for: '{}' but player has no location", session->getPlayerId());
    }

    req.respond(res);
}

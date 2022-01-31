#include "ServicePlayers.hpp"
#include "../../Utils/Random.hpp"

#define CMP "ServicePlayers"

using namespace Scissio;

ServicePlayers::ServicePlayers(const Config& config, AssetManager& assetManager, Scissio::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServicePlayers::tick() {
}

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
    auto [_, result] = db.update<PlayerData>(playerId, [&](std::optional<PlayerData>& player) {
        if (!player.has_value()) {
            Log::i(CMP, "Registering new player: '{}'", name);
            player = PlayerData{};
            player.value().id = playerId;
            player.value().secret = secret;
            player.value().admin = true;
        }

        player.value().name = name;

        return true;
    });

    return result.value();
}

PlayerLocationData ServicePlayers::findStartingLocation(const PlayerData& player) {
    auto [_, result] = db.update<PlayerLocationData>(player.id, [&](std::optional<PlayerLocationData>& location) {
        if (!location.has_value()) {
            Log::i(CMP, "Choosing starting position for player: '{}'", player.name);

            const auto choices = db.seek<SectorData>("", 1);
            if (choices.empty()) {
                EXCEPTION("No choices for starting sector");
            }
            const auto& choice = choices.back();

            location = PlayerLocationData{};
            location.value().galaxyId = choice.galaxyId;
            location.value().systemId = choice.systemId;
            location.value().sectorId = choice.id;

            return true;
        }

        return false;
    });

    return result.value();
}

PlayerLocationData ServicePlayers::getLocation(const std::string& playerId) {
    auto location = db.get<PlayerLocationData>(playerId);
    if (!location) {
        EXCEPTION("Player {} has no location", playerId);
    }
    return location.value();
}

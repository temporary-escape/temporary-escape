#include "service_players.hpp"
#include "../utils/random.hpp"
#include "service_sectors.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

ServicePlayers::ServicePlayers(const Config& config, Registry& registry, TransactionalDatabase& db,
                               Network::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {

    HANDLE_REQUEST(MessagePlayerLocationRequest, MessagePlayerLocationResponse);
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
        if (!player) {
            logger.info("Registering new player: '{}'", name);
            player = PlayerData{};
            player->id = playerId;
            player->secret = secret;
            player->admin = true;
        }

        player->name = name;

        return true;
    });

    return result.value();
}

PlayerLocationData ServicePlayers::findStartingLocation(const std::string& playerId) {
    auto [_, result] = db.update<PlayerLocationData>(playerId, [&](std::optional<PlayerLocationData>& location) {
        if (!location) {
            logger.info("Choosing starting position for player: '{}'", playerId);

            const auto choices = db.seekAll<SectorData>("", 1);
            if (choices.empty()) {
                EXCEPTION("No choices for starting sector");
            }
            const auto& choice = choices.back();

            location = PlayerLocationData{};
            location->galaxyId = choice.galaxyId;
            location->systemId = choice.systemId;
            location->sectorId = choice.id;

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
    return *location;
}

std::vector<BlockPtr> ServicePlayers::getUnlockedBlocks(const std::string& playerId) {
    const auto found = db.get<PlayerUnlockedBlocks>(playerId);

    if (!found) {
        const auto allBlocks = registry.getBlocks().findAll();

        PlayerUnlockedBlocks data{};
        data.id = playerId;

        // TODO!
        data.blocks.reserve(allBlocks.size());
        for (const auto& block : allBlocks) {
            data.blocks.push_back(block);
        }

        db.put(playerId, data);
        return data.blocks;
    }

    return found->blocks;
}

void ServicePlayers::handle(const Service::PeerPtr& peer, MessagePlayerLocationRequest req,
                            MessagePlayerLocationResponse& res) {

    const auto session = sessionValidator.find(peer);
    const auto location = getLocation(session->getPlayerId());

    res.galaxyId = location.galaxyId;
    res.systemId = location.systemId;
    res.sectorId = location.sectorId;
}

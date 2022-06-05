#include "World.hpp"
#include "../Utils/Random.hpp"

#define CMP "World"

using namespace Engine;

World::World(const Config& config, AssetManager& assetManager, Engine::TransactionalDatabase& db)
    : config(config), assetManager(assetManager), db(db) {
}

void World::tick() {
}

std::optional<std::string> World::playerSecretToId(const uint64_t secret) {
    auto player = db.getByIndex<&PlayerData::secret>(secret);
    if (!player.empty()) {
        return player.front().id;
    }

    return std::nullopt;
}

PlayerData World::loginPlayer(const uint64_t secret, const std::string& name) {
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

PlayerLocationData World::findPlayerStartingLocation(const std::string& playerId) {
    auto [_, result] = db.update<PlayerLocationData>(playerId, [&](std::optional<PlayerLocationData>& location) {
        if (!location.has_value()) {
            Log::i(CMP, "Choosing starting position for player: '{}'", playerId);

            const auto choices = db.seekAll<SectorData>("", 1);
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

PlayerLocationData World::getPlayerLocation(const std::string& playerId) {
    auto location = db.get<PlayerLocationData>(playerId);
    if (!location) {
        EXCEPTION("Player {} has no location", playerId);
    }
    return location.value();
}

std::vector<AssetBlockPtr> World::getPlayerUnlockedBlocks(const std::string& playerId) {
    const auto found = db.get<PlayerUnlockedBlocks>(playerId);

    if (!found) {
        const auto allBlocks = assetManager.findAll<AssetBlock>();

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

    return found.value().blocks;
}

GalaxyData World::getGalaxyForPlayer(const std::string& playerId, const std::string& galaxyId) {
    auto galaxy = db.get<GalaxyData>(galaxyId);
    if (!galaxy) {
        EXCEPTION("Galaxy {} does not exist", galaxyId);
    }
    return galaxy.value();
}

void World::create(const GalaxyData& galaxy) {
    db.put(galaxy.id, galaxy);
}

std::vector<RegionData> World::getRegionsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                   const std::string& start, std::string& next) {
    return db.next<RegionData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

void World::create(const RegionData& region) {
    db.put(fmt::format("{}/{}", region.galaxyId, region.id), region);
}

void World::create(const FactionData& faction) {
    db.put(fmt::format("{}", faction.id), faction);
}

std::vector<FactionData> World::getFactionsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                     const std::string& start, std::string& next) {
    // TODO: Return factions only that are relevant to the galaxy
    return db.next<FactionData>(fmt::format(""), start, 64, &next);
}

std::vector<SystemData> World::getSystemsForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                   const std::string& start, std::string& next) {
    return db.next<SystemData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

std::vector<SectorPlanetData> World::getSystemPlanets(const std::string& galaxyId, const std::string& systemId) {
    return db.seekAll<SectorPlanetData>(fmt::format("{}/{}/", galaxyId, systemId));
}

void World::create(const SystemData& system) {
    db.put(fmt::format("{}/{}", system.galaxyId, system.id), system);
}

std::optional<SectorData> World::findSector(const std::string& galaxyId, const std::string& systemId,
                                            const std::string& sectorId) {
    return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
}

void World::create(const SectorPlanetData& planet) {
    db.put(fmt::format("{}/{}/{}", planet.galaxyId, planet.systemId, planet.id), planet);
}

void World::create(const SectorData& sector) {
    db.put(fmt::format("{}/{}/{}", sector.galaxyId, sector.systemId, sector.id), sector);
}

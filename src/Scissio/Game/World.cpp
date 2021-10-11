#include "World.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"

using namespace Scissio;

#define GET_PLAYER(id)                                                                                                 \
    const auto playerOpt = db.get<Player>("id", id);                                                                   \
    if (!playerOpt.has_value()) {                                                                                      \
        EXCEPTION("Player with id: '{}' not found", id);                                                               \
    }                                                                                                                  \
    const auto player = std::move(playerOpt.value());

template <typename D, typename T> static std::vector<D> transform(const std::vector<T>& items) {
    std::vector<D> dtos;
    dtos.reserve(items.size());

    std::transform(items.begin(), items.end(), std::back_inserter(dtos), [](const T& item) { return D::from(item); });

    return dtos;
}

template <typename D, typename T> static Page<D> transform(const Page<T>& page) {
    const auto& items = page.results;

    std::vector<D> dtos;
    dtos.reserve(items.size());

    std::transform(items.begin(), items.end(), std::back_inserter(dtos), [](const T& item) { return D::from(item); });

    return {dtos, page.cont};
}

template <typename D, typename A, typename B>
static std::vector<D> transform(const std::vector<Database::InnerJoin<A, B>>& pairs) {
    std::vector<D> dtos;
    dtos.reserve(pairs.size());

    std::transform(pairs.begin(), pairs.end(), std::back_inserter(dtos),
                   [](const Database::InnerJoin<A, B>& pair) { return D::from(pair.right); });

    return dtos;
}

template <typename D, typename A, typename B> static Page<D> transform(const Page<Database::InnerJoin<A, B>>& page) {
    const auto& pairs = page.results;

    std::vector<D> dtos;
    dtos.reserve(pairs.size());

    std::transform(pairs.begin(), pairs.end(), std::back_inserter(dtos),
                   [](const Database::InnerJoin<A, B>& pair) { return D::from(pair.right); });

    return {dtos, page.cont};
}

void World::Players::updateLocation(const uint64_t playerId, const uint64_t sectorId) {
    auto locationOpt = db.get<PlayerLocation>("playerId", playerId);
    if (!locationOpt.has_value()) {
        locationOpt = {PlayerLocation{0, playerId, 0}};
    }

    locationOpt.value().sectorId = sectorId;
    db.update(locationOpt.value());
}

std::optional<PlayerLocation> World::Players::getLocation(const uint64_t playerId) {
    return db.get<PlayerLocation>("playerId", playerId);
}

std::vector<Asteroid> World::Asteroids::findMany(const float min, const float max) {
    return db.select<Asteroid>("WHERE distMin >= ? AND distMax <= ?", min, max);
}

Page<BlockDto> World::Blocks::findForPlayer(const uint64_t playerId, const uint64_t cont) {
    GET_PLAYER(playerId);

    const auto results = joinPaged<PlayerBlock>("blockId", "PlayerBlock.playerId = ?", cont, playerId);

    return transform<BlockDto>(results);
}

void World::Blocks::addPlayerBlock(const uint64_t playerId, const uint64_t blockId) {
    db.transaction([&]() {
        if (db.count<PlayerBlock>("WHERE playerId = ? AND blockId = ?", playerId, blockId) == 0) {
            PlayerBlock pb{0, playerId, blockId};
            db.insert<PlayerBlock>(pb);
        }
    });
}

Page<SystemDto> World::Systems::findForPlayer(const uint64_t playerId, const uint64_t cont) {
    GET_PLAYER(playerId);

    auto systems = db.select<System>("WHERE id > ? ORDER BY id ASC LIMIT ?", cont, pageLimit);
    const auto links = db.select<SystemLink>(
        "WHERE sourceId IN (SELECT System.id FROM System WHERE System.id > ? ORDER BY System.id ASC LIMIT ?)", cont,
        pageLimit);

    Page<SystemDto> page;

    page.results.reserve(systems.size());
    page.cont = systems.empty() ? cont : systems.back().id;

    std::unordered_map<uint64_t, std::vector<uint64_t>> destinations;
    for (const auto& link : links) {
        destinations[link.sourceId].push_back(link.destinationId);
    }

    for (auto& system : systems) {
        auto it = destinations.find(system.id);
        if (it != destinations.end()) {
            page.results.push_back(SystemDto::from(system, it->second));
        }
    }

    return page;
}

std::optional<Sector> World::Sectors::chooseStartingSector(const uint64_t playerId) {
    GET_PLAYER(playerId);

    const auto results = db.select<Sector>("LIMIT ?", 1);
    if (results.empty()) {
        return std::nullopt;
    }

    return {results.front()};
}

/*Page<SectorDto> World::Sectors::findForPlayer(const uint64_t playerId, const uint64_t cont) {
    GET_PLAYER(playerId);
}*/

Page<RegionDto> World::Regions::findForPlayer(const uint64_t playerId, const uint64_t cont) {
    GET_PLAYER(playerId);

    const auto page = findPaged("", cont);

    return transform<RegionDto>(page);
}

World::World(const Config& config, Database& db)
    : config(config), players(db), asteroids(db), blocks(db), galaxies(db), systems(db), systemLinks(db), sectors(db),
      regions(db), db(db) {
}

std::optional<Player> World::playerLogin(const uint64_t playerId, const std::string& name) {
    Log::v("Logging in player id: {} name: '{}'", playerId, name);

    // Check if player already exists in the database
    auto data = players.get(playerId);
    if (data.has_value()) {
        // Player has changed their name
        if (data.value().name != name) {
            Log::v("Changing player's original uid: {} name: '{}' into: '{}'", playerId, data.value().name, name);
            data.value().name = name;
        }

        data.value().lastLogin = 1;
        players.update(data.value());
    }

    return data;
}

bool World::playerNameTaken(const std::string& name) {
    const auto found = players.find("WHERE name = ?", name);
    return found.has_value();
}

Player World::playerRegister(const uint64_t playerId, const std::string& name) {
    // Do not allow multiple players with the same name
    const auto found = players.find("WHERE name = ?", name);
    if (found.has_value()) {
        EXCEPTION("Player with name '{}' already exists", name);
    }

    // Create a new player
    Log::v("Creating player id: {} name: '{}'", playerId, name);
    Player player{playerId, name, false, std::nullopt};
    players.insert(player);

    // Choose starting location for the new player
    const auto locationOpt = players.getLocation(playerId);
    if (!locationOpt.has_value()) {
        const auto sectorOpt = sectors.chooseStartingSector(playerId);

        if (!sectorOpt.has_value()) {
            EXCEPTION("Unable to choose starting sector for player '{}'", name);
        }

        players.updateLocation(playerId, sectorOpt.value().id);
    }

    return player;
}

void World::playerInit(const uint64_t playerId) {
    Log::v("Initializing player id: {}", playerId);

    for (const auto& block : blocks.findMany("")) {
        this->blocks.addPlayerBlock(playerId, block.id);
    }
}

/*std::optional<PlayerLocation> World::playerGetLocation(const uint64_t id) {
    (void)getPlayer(id);
    return db.get<PlayerLocation>("playerId", id);
}

Player World::getPlayer(const uint64_t id) {
    const auto found = playerFind(id);
    if (!found.has_value()) {
        EXCEPTION("Player with id: '{}' does not exist", id);
    }

    return found.value();
}

std::vector<Asteroid> World::asteroidsFind(float min, float max) {
    return db.select<Asteroid>("WHERE distMin >= ? AND distMax <= ?", min, max);
}

std::vector<BlockDto> World::blocksGetForPlayer(const uint64_t playerId, const size_t offset) {
    auto player = getPlayer(playerId);
    std::vector<BlockDto> results;
    const auto join = db.join<PlayerBlock, Block>("blockId", "id", "WHERE PlayerBlock.playerId = ? LIMIT 128 OFFSET ?",
                                                  player.id, offset);

    if (join.empty() && !offset) {
        auto blocks = db.select<Block>();
        for (const auto& b : blocks) {
            db.transaction([&]() {
                if (db.count<PlayerBlock>("WHERE playerId = ? AND blockId = ?", player.id, b.id) == 0) {
                    PlayerBlock pb{0, player.id, b.id};
                    db.insert<PlayerBlock>(pb);
                }
            });
        }

        for (auto& block : blocks) {
            results.push_back(BlockDto::from(block));
        }

        return results;
    }

    results.reserve(join.size());

    for (auto& pair : join) {
        results.push_back(BlockDto::from(pair.right));
    }

    return results;
}

bool World::galaxyExists() {
    return db.count<Galaxy>() > 0;
}

std::vector<SystemDto> World::systemsGetPaged(const size_t offset) {
    auto systems = db.select<System>("ORDER BY id ASC LIMIT 128 OFFSET ?", offset);
    const auto links = db.select<SystemLink>(
        "WHERE sourceId IN (SELECT System.id FROM System ORDER BY System.id ASC LIMIT 128 OFFSET ?)", offset);

    std::vector<SystemDto> result;
    result.reserve(systems.size());

    std::unordered_map<uint64_t, std::vector<uint64_t>> destinations;
    for (const auto& link : links) {
        destinations[link.sourceId].push_back(link.destinationId);
    }

    for (auto& system : systems) {
        auto it = destinations.find(system.id);
        if (it != destinations.end()) {
            result.push_back(SystemDto::from(system, it->second));
        }
    }

    return result;
}

std::vector<RegionDto> World::regionsGetPages(const size_t offset) {
    auto regions = db.select<Region>("ORDER BY id ASC LIMIT 128 OFFSET ?", offset);

    std::vector<RegionDto> result;
    result.reserve(regions.size());

    for (auto& region : regions) {
        result.push_back(RegionDto::from(region));
    }

    return result;
}

System World::systemGet(const uint64_t id) {
    const auto found = db.get<System>(id);
    if (!found.has_value()) {
        EXCEPTION("System with id: '{}' does not exist", id);
    }

    return found.value();
}

Sector World::sectorGet(const uint64_t id) {
    const auto found = db.get<Sector>(id);
    if (!found.has_value()) {
        EXCEPTION("Sector with id: '{}' does not exist", id);
    }

    return found.value();
}*/

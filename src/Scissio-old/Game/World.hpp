#pragma once

#include "../Utils/Repository.hpp"
#include "Schemas.hpp"

namespace Scissio {
class World {
public:
    class Players : public Repository<Player> {
    public:
        using Repository<Player>::Repository;

        void updateLocation(uint64_t playerId, uint64_t sectorId);
        std::optional<PlayerLocation> getLocation(uint64_t playerId);
    };

    class Asteroids : public Repository<Asteroid> {
    public:
        using Repository<Asteroid>::Repository;

        std::vector<Asteroid> findMany(float min, float max);
    };

    class Blocks : public Repository<Block> {
    public:
        using Repository<Block>::Repository;

        Page<BlockDto> findForPlayer(uint64_t playerId, uint64_t cont = 0);
        void addPlayerBlock(uint64_t playerId, uint64_t blockId);
    };

    class Galaxies : public Repository<Galaxy> {
    public:
        using Repository<Galaxy>::Repository;
    };

    class Systems : public Repository<System> {
    public:
        using Repository<System>::Repository;

        Page<SystemDto> findForPlayer(uint64_t playerId, uint64_t cont = 0);
    };

    class SystemLinks : public Repository<SystemLink> {
    public:
        using Repository<SystemLink>::Repository;
    };

    class Sectors : public Repository<Sector> {
    public:
        using Repository<Sector>::Repository;

        std::optional<Sector> chooseStartingSector(uint64_t playerId);
        // Page<SectorDto> findForPlayer(uint64_t playerId, uint64_t cont = 0);
    };

    class Regions : public Repository<Region> {
    public:
        using Repository<Region>::Repository;

        Page<RegionDto> findForPlayer(uint64_t playerId, uint64_t cont = 0);
    };

    explicit World(const Config& config, Database& db);

    void playerInit(uint64_t playerId);
    std::optional<Player> playerLogin(uint64_t playerId, const std::string& name);
    bool playerNameTaken(const std::string& name);
    Player playerRegister(uint64_t playerId, const std::string& name);

    template <typename Fn> void transaction(Fn&& fn) {
        db.transaction(std::forward<Fn>(fn));
    }

    /*// Player
    void playerInit(uint64_t id);
    Player playerLogin(uint64_t id, const std::string& name);
    std::optional<Player> playerFind(uint64_t id);
    std::optional<PlayerLocation> playerGetLocation(uint64_t id);

    // Asteroids
    std::vector<Asteroid> asteroidsFind(float min, float max);

    // Blocks
    std::vector<BlockDto> blocksGetForPlayer(uint64_t playerId, const size_t offset);

    // Galaxy
    bool galaxyExists();

    std::vector<SystemDto> systemsGetPaged(size_t offset);
    std::vector<RegionDto> regionsGetPages(size_t offset);
    System systemGet(uint64_t id);
    Sector sectorGet(uint64_t id);*/

    /*Database& getDb() const {
        return db;
    }*/

    Players players;
    Asteroids asteroids;
    Blocks blocks;
    Galaxies galaxies;
    Systems systems;
    SystemLinks systemLinks;
    Sectors sectors;
    Regions regions;

private:
    const Config& config;
    Database& db;
};
} // namespace Scissio

#pragma once

#include "Database.hpp"
#include "Schemas.hpp"

namespace Scissio {
class Generator {
public:
    struct Options {
        int totalSystems = 2000;
        float galaxyWidth = 300.0f;
        float regionDistance = 50.0f;
        int systemSectorsMin = 2;
        int systemSectorsMax = 7;
    };

    explicit Generator(Database& db, Options options);

    void generateWorld(uint64_t seed);
    void generateGalaxies(uint64_t seed);
    void generateRegions(uint64_t seed, const std::string& galaxyId);
    void generateSystems(uint64_t seed, const std::string& galaxyId);
    void generateSectors(uint64_t seed, const std::string& galaxyId, const std::string& systemId);

private:
    const Options options;
    Database& db;
};
} // namespace Scissio

#pragma once

#include "../database/database.hpp"
#include "schemas.hpp"

namespace Engine {
class Generator {
public:
    struct Options {
        float galaxyWidthMax{250.0f};
        int galaxyRegionsMax{20};
        int galaxySystemsMax{2000};
        float galaxyRegionDistanceMin{35.0f};
        float galaxyFactionDistanceMin{25.0f};
        float connectionDistMax{40.0f};
        int systemPlanetsMin{3};
        int systemPlanetsMax{6};
        int planetMoonsMin{0};
        int planetMoonsMax{2};
        float planetDistanceMin{8.0f};
        float planetDistanceMax{12.0f};
        float moonDistanceMin{1.0f};
        float moonDistanceMax{2.5f};
    };

    explicit Generator(const Options& options, AssetsManager& assetsManager, Database& db);
    virtual ~Generator() = default;

    void generate(uint64_t seed);

private:
    void createMainGalaxy(const std::string& galaxyId, uint64_t seed);
    void createGalaxyRegions(const std::string& galaxyId);
    void createGalaxySystems(const std::string& galaxyId);
    void createGalaxySectors(const std::string& galaxyId);
    size_t createGalaxySectors(const GalaxyData& galaxy, const SystemData& system);
    void fillGalaxyRegions(const std::string& galaxyId, const std::vector<Vector2>& positions,
                           const std::unordered_map<size_t, std::vector<size_t>>& connections,
                           std::vector<SystemData>& systems);
    void findFactionHomes(const std::string& galaxyId, std::vector<SystemData>& systems);
    void fillGalaxyFactions(const std::string& galaxyId, const std::vector<Vector2>& positions,
                            const std::unordered_map<size_t, std::vector<size_t>>& connections,
                            std::vector<SystemData>& systems);
    void fillSystemPlanets(const std::string& galaxyId, const SystemData& system);

    const Options options;
    AssetsManager& assetsManager;
    Database& db;
};
} // namespace Engine

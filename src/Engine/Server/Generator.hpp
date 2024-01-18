#pragma once

#include "../Database/Database.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/Random.hpp"
#include "Schemas.hpp"

namespace Engine {
class ENGINE_API NameGenerator;

class ENGINE_API SystemHeuristics {
public:
    SystemHeuristics(const GalaxyData& galaxy, const SystemData& system, const std::vector<SystemData>& systems,
                     const std::vector<SectorData>& sectors, const std::vector<PlanetData>& planets,
                     const std::optional<FactionData>& faction);

    const GalaxyData& getGalaxy() const {
        return galaxy;
    }

    const SystemData& getSystem() const {
        return system;
    }

    const std::vector<SystemData>& getSystems() const {
        return systems;
    }

    const std::vector<SectorData>& getSectors() const {
        return sectors;
    }

    const std::vector<PlanetData>& getPlanets() const {
        return planets;
    }

    const std::optional<FactionData>& getFaction() const {
        return faction;
    }

    Vector2 findEmptyPosition(Rng& rng) const;
    float getSystemRadius() const;

private:
    const GalaxyData& galaxy;
    const SystemData& system;
    const std::vector<SystemData>& systems;
    const std::vector<SectorData>& sectors;
    const std::vector<PlanetData>& planets;
    const std::optional<FactionData>& faction;
};

class ENGINE_API Generator {
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

    using OnGeneratorStartCallback = std::function<void(uint64_t)>;
    using OnGalaxyCreatedCallback = std::function<void(const GalaxyData&)>;
    using OnRegionCreatedCallback = std::function<void(const GalaxyData&, const RegionData&)>;
    using OnSystemCreatedCallback =
        std::function<void(const GalaxyData&, const SystemData& system, const std::optional<FactionData>& faction)>;
    using OnSectorCreatedCallback =
        std::function<void(const GalaxyData&, const SystemData& system, const std::optional<FactionData>& faction,
                           const SectorData& sector)>;
    using OnGeneratorEndCallback = std::function<void(const GalaxyData&)>;
    using OnGeneratorSkippedCallback = std::function<void(const GalaxyData&)>;
    using SystemHeuristicsCallback = std::function<float(SystemHeuristics&)>;
    using OnCreateSystemCallback = std::function<SectorData(SystemHeuristics&, uint64_t)>;

    explicit Generator(const Options& options, AssetsManager& assetsManager, Database& db);
    virtual ~Generator() = default;

    void generate(uint64_t seed);
    // void populate(const SectorData& sector, Scene& scene);

    void addSectorType(const std::string& name, SystemHeuristicsCallback heuristics, OnCreateSystemCallback onCreate);

    void addOnStart(OnGeneratorStartCallback fn) {
        callbacks.onStart.push_back(std::move(fn));
    }

    void addOnGalaxyCreated(OnGalaxyCreatedCallback fn) {
        callbacks.onGalaxyCreated.push_back(std::move(fn));
    }

    void addOnRegionCreated(OnRegionCreatedCallback fn) {
        callbacks.onRegionCreated.push_back(std::move(fn));
    }

    void addOnSystemCreated(OnSystemCreatedCallback fn) {
        callbacks.onSystemCreated.push_back(std::move(fn));
    }

    void addOnSectorCreated(OnSectorCreatedCallback fn) {
        callbacks.onSectorCreated.push_back(std::move(fn));
    }

    void addOnEnd(OnGeneratorEndCallback fn) {
        callbacks.onEnd.push_back(std::move(fn));
    }

    void addOnSkipped(OnGeneratorSkippedCallback fn) {
        callbacks.onSkipped.push_back(std::move(fn));
    }

    std::string getRandomName(Rng& rng) const;
    NameGenerator& getNameGenerator();

private:
    struct SystemType {
        SystemHeuristicsCallback heuristics;
        OnCreateSystemCallback onCreate;
    };

    GalaxyData createMainGalaxy(const std::string& galaxyId, uint64_t seed);
    void createGalaxyRegions(const std::string& galaxyId);
    void createGalaxySystems(const std::string& galaxyId);
    void createGalaxySectors(const std::string& galaxyId);
    size_t createGalaxySectors(const GalaxyData& galaxy, const std::vector<SystemData>& systems,
                               const SystemData& system);
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

    std::unordered_map<std::string, SystemType> systemTypes;

    struct {
        std::vector<OnGeneratorStartCallback> onStart;
        std::vector<OnGalaxyCreatedCallback> onGalaxyCreated;
        std::vector<OnRegionCreatedCallback> onRegionCreated;
        std::vector<OnSystemCreatedCallback> onSystemCreated;
        std::vector<OnSectorCreatedCallback> onSectorCreated;
        std::vector<OnGeneratorEndCallback> onEnd;
        std::vector<OnGeneratorSkippedCallback> onSkipped;
    } callbacks;
};
} // namespace Engine

#pragma once

#include "../Scene/Scene.hpp"
#include "Database.hpp"
#include "Schemas.hpp"

namespace Scissio {
class GeneratorStep;

class Generator {
public:
    template <typename T> struct OptionType {
        T* value{nullptr};
        std::string name;
        std::string description;
    };

    using Option = std::variant<OptionType<bool>, OptionType<int>, OptionType<float>>;
    using OptionMap = std::unordered_map<std::string, Option>;

    template <typename T> void addStep(AssetManager& assetManager) {
        steps.push_back(std::make_shared<T>(assetManager));
    }

    void generate(Database& db, uint64_t seed);
    void populate(Database& db, uint64_t seed, Scene& scene, const std::string& galaxyId, const std::string& systemId,
                  const std::string& sectorId);

private:
    std::vector<std::shared_ptr<GeneratorStep>> steps;
};

class GeneratorStep {
public:
    virtual ~GeneratorStep() = default;

    virtual Generator::OptionMap getOptions() = 0;
    virtual void generate(Database& db, uint64_t seed) = 0;
    virtual void populate(Database& db, uint64_t seed, Scene& scene, const std::string& galaxyId,
                          const std::string& systemId, const std::string& sectorId) = 0;
};

class GeneratorStepCoreGalaxy : public GeneratorStep {
public:
    explicit GeneratorStepCoreGalaxy(AssetManager& assetManager);

    Generator::OptionMap getOptions() override;
    void generate(Database& db, uint64_t seed) override;
    void populate(Database& db, uint64_t seed, Scene& scene, const std::string& galaxyId, const std::string& systemId,
                  const std::string& sectorId) override;
    void generateGalaxies(Database& db, uint64_t seed);
    void generateRegions(Database& db, uint64_t seed, const std::string& galaxyId);
    void generateSystems(Database& db, uint64_t seed, const std::string& galaxyId);
    void generateSectors(Database& db, uint64_t seed, const std::string& galaxyId, const std::string& systemId);

private:
    struct Options {
        int totalSystems{2000};
        float galaxyWidth{300.0f};
        float regionDistance{50.0f};
        int systemSectorsMin{2};
        int systemSectorsMax{7};
    };

    AssetManager& assetManager;
    Options options;
};
} // namespace Scissio

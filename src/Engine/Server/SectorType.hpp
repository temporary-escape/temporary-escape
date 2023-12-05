#pragma once

#include "../Utils/Random.hpp"
#include "Schemas.hpp"

namespace Engine {
struct ENGINE_API Spawner {
    Spawner() = default;
    explicit Spawner(std::string entity, float weight, int count) :
        entity{std::move(entity)}, weight{weight}, count{count} {
    }

    std::string entity;
    float weight{1.0f};
    int count;
};

struct ENGINE_API SectorCondition {
    using Value = std::variant<bool, float, int>;

    SectorCondition() = default;
    explicit SectorCondition(std::string name, Value value) : name{std::move(name)}, value{value} {
    }

    std::string name;
    Value value;
};

struct ENGINE_API SectorType {
    ImagePtr mapIcon;
    float weight{1.0f};
    int minCount{1};
    int maxCount{1};
    std::vector<SectorCondition> conditions;
    std::vector<Spawner> entities;

    bool checkConditions(Rng& rng, const GalaxyData& galaxy, const SystemData& system,
                         const std::vector<PlanetData>& planets) const;
};
} // namespace Engine

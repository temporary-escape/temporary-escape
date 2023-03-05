#pragma once

#include "../scene/scene.hpp"
#include "world.hpp"

namespace Engine {
class ENGINE_API Generator {
public:
    using SectorGenerator = std::function<void(const std::string&, Scene&)>;

    virtual ~Generator() = default;

    virtual void generate(uint64_t seed) = 0;

private:
    std::unordered_map<std::string, SectorGenerator> sectorGenerators;
};
} // namespace Engine

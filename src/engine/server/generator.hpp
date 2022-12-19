#pragma once

#include "world.hpp"

namespace Engine {
class ENGINE_API Generator {
public:
    virtual ~Generator() = default;

    virtual void generate(uint64_t seed) = 0;
};
} // namespace Engine

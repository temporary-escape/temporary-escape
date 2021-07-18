#pragma once

#include "Schemas.hpp"

namespace Scissio {
class World;
}

namespace Scissio::Generator {
Galaxy generateGalaxy(const Config& config, World& world, uint64_t seed);
} // namespace Scissio::Generator

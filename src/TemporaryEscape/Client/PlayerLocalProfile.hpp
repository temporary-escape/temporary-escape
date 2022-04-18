#pragma once

#include "../Utils/Yaml.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;

    YAML_DEFINE(name, secret);
};
} // namespace Engine

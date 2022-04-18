#pragma once
#include "../Utils/Yaml.hpp"
#include <string>

namespace Engine {
struct Manifest {
    Path path;
    std::string name;
    std::string description;
    std::string author;

    YAML_DEFINE(name, description, author);
};
} // namespace Engine

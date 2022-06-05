#pragma once
#include "../Utils/Msgpack.hpp"
#include "../Utils/Yaml.hpp"
#include <string>

namespace Engine {
struct Manifest {
    Path path;
    std::string name;
    std::string description;
    std::string author;
    std::string version;

    YAML_DEFINE(name, description, author, version);
    MSGPACK_DEFINE_MAP(name, description, author, version);
};
} // namespace Engine

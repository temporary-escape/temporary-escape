#pragma once
#include "../utils/yaml.hpp"
#include <msgpack.hpp>

namespace Engine {
struct ENGINE_API ModManifest {
    Path path;
    std::string name;
    std::string description;
    std::string author;
    std::string version;

    YAML_DEFINE(name, description, author, version);
    MSGPACK_DEFINE_MAP(name, description, author, version);
};
} // namespace Engine

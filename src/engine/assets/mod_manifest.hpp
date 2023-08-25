#pragma once
#include "../utils/xml.hpp"
#include <msgpack.hpp>

namespace Engine {
struct ENGINE_API ModManifest {
    Path path;
    std::string name;
    std::string description;
    std::string author;
    std::string version;

    MSGPACK_DEFINE_MAP(name, description, author, version);

    void convert(const Xml::Node& xml) {
        xml.convert("name", name);
        xml.convert("description", description);
        xml.convert("author", author);
        xml.convert("version", version);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("name", name);
        xml.pack("description", description);
        xml.pack("author", author);
        xml.pack("version", version);
    }
};

XML_DEFINE(ModManifest, "manifest");
} // namespace Engine

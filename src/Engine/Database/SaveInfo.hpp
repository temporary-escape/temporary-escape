#pragma once

#include "../Utils/Chrono.hpp"

namespace Engine {
struct SaveInfo {
    std::string version;
    std::chrono::system_clock::time_point timestamp;
    Path path;              // Filled via loadSaveInfoDir()
    bool compatible{false}; // Filled via loadSaveInfoDir()

    void convert(const Xml::Node& xml) {
        xml.convert("version", version);
        xml.convert("timestamp", timestamp);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("version", version);
        xml.pack("timestamp", timestamp);
    }
};

XML_DEFINE(SaveInfo, "save-info");

ENGINE_API std::vector<SaveInfo> loadSaveInfoDir(const Path& path);
} // namespace Engine

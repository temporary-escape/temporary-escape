#pragma once
#include "Chrono.hpp"
#include <nlohmann/json.hpp>

namespace Engine {
using Json = nlohmann::json;
}

namespace nlohmann {
template <> struct adl_serializer<std::chrono::system_clock::time_point> {
    static void to_json(json& j, const std::chrono::system_clock::time_point& tp) {
        j = Engine::timePointToIso(tp);
    }

    static void from_json(const json& j, std::chrono::system_clock::time_point& tp) {
        tp = Engine::isoToTimePoint(j.get<std::string>());
    }
};
} // namespace nlohmann

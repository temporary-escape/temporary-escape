#pragma once

#include "../library.hpp"
#include "exceptions.hpp"
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Engine {
extern std::vector<std::string> ENGINE_API splitLast(const std::string_view& str, const std::string_view& delim);
extern std::vector<std::string> ENGINE_API split(const std::string_view& str, const std::string_view& delim);
extern std::string ENGINE_API intToRomanNumeral(const int value);
extern std::string ENGINE_API toLower(const std::string_view& str);
extern bool ENGINE_API endsWith(const std::string_view& str, const std::string_view& ending);
extern bool ENGINE_API startsWith(const std::string_view& str, const std::string_view& start);

template <typename T> extern std::string join(const std::string& joiner, const std::vector<T>& items) {
    std::stringstream ss;
    auto first = true;
    for (const auto& item : items) {
        if (!first) {
            ss << joiner;
        }
        ss << item;
        first = false;
    }
    return ss.str();
}

template <typename E> struct EnumMapper {
    static const std::unordered_map<E, std::string>& getToStrMap() {
        EXCEPTION("Default template case not initialized");
    }
    static const std::unordered_map<std::string, E>& getFromStrMap() {
        EXCEPTION("Default template case not initialized");
    }
};

template <typename E> inline const std::string& enumToStr(const E value) {
    const auto& map = EnumMapper<E>::getToStrMap();
    const auto it = map.find(value);
    if (it == map.end()) {
        EXCEPTION("Unknown enum of type: {} value: {}", typeid(E).name(), int(value));
    }
    return it->second;
}

template <typename E> inline E strToEnum(const std::string& value) {
    const auto& map = EnumMapper<E>::getFromStrMap();
    const auto it = map.find(value);
    if (it == map.end()) {
        EXCEPTION("Unknown enum of type: {} value: {}", typeid(E).name(), value);
    }
    return it->second;
}

template <typename E>
static std::unordered_map<E, std::string> buildEnumToStrMap(const std::vector<std::pair<E, std::string>>& items) {
    std::unordered_map<E, std::string> map;
    for (const auto& pair : items) {
        map.insert(pair);
    }
    return map;
}

template <typename E>
static std::unordered_map<std::string, E> buildEnumFromStrMap(const std::vector<std::pair<E, std::string>>& items) {
    std::unordered_map<std::string, E> map;
    for (const auto& pair : items) {
        map.insert(std::make_pair(pair.second, pair.first));
    }
    return map;
}

template <typename E> class EnumPairs {
public:
    explicit EnumPairs(std::vector<std::pair<E, std::string>> pairs) : pairs(std::move(pairs)) {
    }

    using Type = E;

    std::vector<std::pair<E, std::string>> pairs;
};

#define DEFINE_ENUM_TOKEN(x, y) x##y
#define DEFINE_ENUM_TOKEN2(x, y) DEFINE_ENUM_TOKEN(x, y)
#define DEFINE_ENUM_STR(E)                                                                                             \
    static const inline auto DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__) = E;                                              \
    template <> struct EnumMapper<decltype(DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__))::Type> {                           \
        static const std::unordered_map<decltype(DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__))::Type, std::string>&         \
        getToStrMap() {                                                                                                \
            static const auto map = buildEnumToStrMap<decltype(DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__))::Type>(        \
                DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__).pairs);                                                        \
            return map;                                                                                                \
        }                                                                                                              \
        static const std::unordered_map<std::string, decltype(DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__))::Type>&         \
        getFromStrMap() {                                                                                              \
            static const auto map = buildEnumFromStrMap<decltype(DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__))::Type>(      \
                DEFINE_ENUM_TOKEN2(EnumPairs, __LINE__).pairs);                                                        \
            return map;                                                                                                \
        }                                                                                                              \
    };
} // namespace Engine

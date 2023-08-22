#pragma once

#include "../library.hpp"
#include "../math/vector.hpp"
#include "exceptions.hpp"
#include "macros.hpp"
#include "path.hpp"
#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

// Forward declaration
namespace YAML {
class Node;
}

namespace Engine::Yaml {
class ENGINE_API Node {
public:
    explicit Node(std::unique_ptr<YAML::Node> yaml);
    Node(const Node& other) = delete;
    Node(Node&& other) noexcept;
    ~Node();

    static Node map();
    static Node array();

    Node& operator=(const Node& other) = delete;
    Node& operator=(Node&& other) noexcept;

    [[nodiscard]] int8_t asInt8() const;
    [[nodiscard]] int16_t asInt16() const;
    [[nodiscard]] int32_t asInt32() const;
    [[nodiscard]] int64_t asInt64() const;
    [[nodiscard]] uint8_t asUint8() const;
    [[nodiscard]] uint16_t asUint16() const;
    [[nodiscard]] uint32_t asUint32() const;
    [[nodiscard]] uint64_t asUint64() const;
    [[nodiscard]] float asFloat() const;
    [[nodiscard]] double asDouble() const;
    [[nodiscard]] bool asBool() const;
    [[nodiscard]] std::string asString() const;
    void packInt8(int8_t value);
    void packInt16(int16_t value);
    void packInt32(int32_t value);
    void packInt64(int64_t value);
    void packUint8(uint8_t value);
    void packUint16(uint16_t value);
    void packUint32(uint32_t value);
    void packUint64(uint64_t value);
    void packFloat(float value);
    void packDouble(double value);
    void packBool(bool value);
    void packString(const std::string& value);
    void packNull();
    [[nodiscard]] bool contains(const std::string& key) const;
    [[nodiscard]] Node child(const std::string& key) const;
    [[nodiscard]] Node append();
    [[nodiscard]] Node insert(const std::string& key);
    [[nodiscard]] bool isMap() const;
    [[nodiscard]] bool isNull() const;
    [[nodiscard]] bool isSequence() const;
    [[nodiscard]] size_t size() const;
    void forEach(const std::function<void(const Node&)>& fn) const;
    void forEach(const std::function<void(const std::string&, const Node&)>& fn) const;

    operator bool() const {
        return bool(yaml);
    }

    friend ENGINE_API void save(const Node& node, const Path& path);

private:
    std::unique_ptr<YAML::Node> yaml;
};

extern ENGINE_API Node load(const Path& path);
extern ENGINE_API void save(const Node& node, const Path& path);

template <typename T> struct Adaptor {
    static void convert(const Yaml::Node& node, T& value) {
        if (!node)
            throw std::bad_cast();
        value.convert(node);
    }

    static void pack(Yaml::Node& node, const T& value) {
        if (!node)
            throw std::bad_cast();
        value.pack(node);
    }
};

template <> struct Adaptor<int8_t> {
    static void convert(const Yaml::Node& node, int8_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asInt8();
    }

    static void pack(Yaml::Node& node, const int8_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packInt8(value);
    }
};

template <> struct Adaptor<int16_t> {
    static void convert(const Yaml::Node& node, int16_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asInt16();
    }

    static void pack(Yaml::Node& node, const int16_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packInt16(value);
    }
};

template <> struct Adaptor<int32_t> {
    static void convert(const Yaml::Node& node, int32_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asInt32();
    }

    static void pack(Yaml::Node& node, const int32_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packInt32(value);
    }
};

template <> struct Adaptor<int64_t> {
    static void convert(const Yaml::Node& node, int64_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asInt64();
    }

    static void pack(Yaml::Node& node, const int64_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packInt64(value);
    }
};

template <> struct Adaptor<uint8_t> {
    static void convert(const Yaml::Node& node, uint8_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asUint8();
    }

    static void pack(Yaml::Node& node, const uint8_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packUint8(value);
    }
};

template <> struct Adaptor<uint16_t> {
    static void convert(const Yaml::Node& node, uint16_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asUint16();
    }

    static void pack(Yaml::Node& node, const uint16_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packUint16(value);
    }
};

template <> struct Adaptor<uint32_t> {
    static void convert(const Yaml::Node& node, uint32_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asUint32();
    }

    static void pack(Yaml::Node& node, const uint32_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packUint32(value);
    }
};

template <> struct Adaptor<uint64_t> {
    static void convert(const Yaml::Node& node, uint64_t& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asUint64();
    }

    static void pack(Yaml::Node& node, const uint64_t& value) {
        if (!node)
            throw std::bad_cast();
        node.packUint64(value);
    }
};

template <> struct Adaptor<float> {
    static void convert(const Yaml::Node& node, float& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asFloat();
    }

    static void pack(Yaml::Node& node, const float& value) {
        if (!node)
            throw std::bad_cast();
        node.packFloat(value);
    }
};

template <> struct Adaptor<double> {
    static void convert(const Yaml::Node& node, double& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asDouble();
    }

    static void pack(Yaml::Node& node, const double& value) {
        if (!node)
            throw std::bad_cast();
        node.packDouble(value);
    }
};

template <> struct Adaptor<bool> {
    static void convert(const Yaml::Node& node, bool& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asBool();
    }

    static void pack(Yaml::Node& node, const bool& value) {
        if (!node)
            throw std::bad_cast();
        node.packBool(value);
    }
};

template <> struct Adaptor<std::string> {
    static void convert(const Yaml::Node& node, std::string& value) {
        if (!node)
            throw std::bad_cast();
        value = node.asString();
    }

    static void pack(Yaml::Node& node, const std::string& value) {
        if (!node)
            throw std::bad_cast();
        node.packString(value);
    }
};

template <typename T> struct Adaptor<std::optional<T>> {
    static void convert(const Yaml::Node& node, std::optional<T>& value) {
        if (!node || node.isNull()) {
            value = std::nullopt;
        } else {
            value = T{};
            Adaptor<T>::convert(node, value.value());
        }
    }

    static void pack(Yaml::Node& node, const std::optional<T>& value) {
        if (value.has_value()) {
            Adaptor<T>::pack(node, value.value());
        } else {
            node.packNull();
        }
    }
};

template <typename T> struct Adaptor<std::vector<T>> {
    static void convert(const Yaml::Node& node, std::vector<T>& value) {
        if (!node || !node.isSequence())
            throw std::bad_cast();

        value.reserve(node.size());
        node.forEach([&](const Node& child) {
            value.emplace_back();
            Adaptor<T>::convert(child, value.back());
        });
    }

    static void pack(Yaml::Node& node, const std::vector<T>& value) {
        for (const auto& item : value) {
            auto child = node.append();
            Adaptor<T>::pack(child, item);
        }
    }
};

template <typename T, size_t N> struct Adaptor<std::array<T, N>> {
    static void convert(const Yaml::Node& node, std::array<T, N>& value) {
        if (!node || !node.isSequence())
            throw std::bad_cast();

        if (node.size() != value.size()) {
            EXCEPTION("Too many items to unpack into array of size {}", N);
        }

        size_t i{0};
        node.forEach([&](const Node& child) { Adaptor<T>::convert(child, value[i++]); });
    }

    static void pack(Yaml::Node& node, const std::array<T, N>& value) {
        for (const auto& item : value) {
            auto child = node.append();
            Adaptor<T>::pack(child, item);
        }
    }
};

template <glm::length_t L, typename T> struct Adaptor<glm::vec<L, T, glm::defaultp>> {
    static void convert(const Yaml::Node& node, glm::vec<L, T, glm::defaultp>& value) {
        if (!node || !node.isSequence() || node.size() != L)
            throw std::bad_cast();

        T* ptr = &value.x;
        node.forEach([&](const Node& child) { Adaptor<T>::convert(child, *ptr++); });
    }

    static void pack(Yaml::Node& node, const glm::vec<L, T, glm::defaultp>& value) {
        const T* src = &value.x;
        for (auto i = 0; i < L; i++) {
            auto child = node.append();
            Adaptor<T>::pack(child, *src++);
        }
    }
};

template <typename T> struct Adaptor<std::unordered_map<std::string, T>> {
    static void convert(const Yaml::Node& node, std::unordered_map<std::string, T>& value) {
        if (!node || !node.isMap())
            throw std::bad_cast();

        value.reserve(node.size());
        node.forEach([&](const std::string& key, const Node& child) {
            auto& ref = value[key];
            Adaptor<T>::convert(child, ref);
        });
    }

    static void pack(Yaml::Node& node, const std::unordered_map<std::string, T>& value) {
        for (const auto& [key, item] : value) {
            auto child = node.insert(key);
            Adaptor<T>::pack(child, item);
        }
    }
};

template <typename T> inline void convert(const Yaml::Node& node, const std::string& key, T& value) {
    try {
        Adaptor<T>::convert(node.child(key), value);
    } catch (...) {
        EXCEPTION_NESTED("Failed to convert yaml map key: '{}'", key);
    }
}

template <typename T> inline void pack(Yaml::Node& node, const std::string& key, const T& value) {
    try {
        auto child = node.insert(key);
        Adaptor<T>::pack(child, value);
    } catch (...) {
        EXCEPTION_NESTED("Failed to pack yaml map key: '{}'", key);
    }
}
} // namespace Engine::Yaml

#define YAML_EXPAND(x) x
#define YAML_FOR_EACH_1(what, x, ...) what(x)
#define YAML_FOR_EACH_2(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_1(what, __VA_ARGS__))
#define YAML_FOR_EACH_3(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_2(what, __VA_ARGS__))
#define YAML_FOR_EACH_4(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_3(what, __VA_ARGS__))
#define YAML_FOR_EACH_5(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_4(what, __VA_ARGS__))
#define YAML_FOR_EACH_6(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_5(what, __VA_ARGS__))
#define YAML_FOR_EACH_7(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_6(what, __VA_ARGS__))
#define YAML_FOR_EACH_8(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_7(what, __VA_ARGS__))
#define YAML_FOR_EACH_9(what, x, ...)                                                                                  \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_8(what, __VA_ARGS__))
#define YAML_FOR_EACH_10(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_9(what, __VA_ARGS__))
#define YAML_FOR_EACH_11(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_10(what, __VA_ARGS__))
#define YAML_FOR_EACH_12(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_11(what, __VA_ARGS__))
#define YAML_FOR_EACH_13(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_12(what, __VA_ARGS__))
#define YAML_FOR_EACH_14(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_13(what, __VA_ARGS__))
#define YAML_FOR_EACH_15(what, x, ...)                                                                                 \
    what(x);                                                                                                           \
    YAML_EXPAND(YAML_FOR_EACH_14(what, __VA_ARGS__))
#define YAML_FOR_EACH_NARG(...) YAML_FOR_EACH_NARG_(__VA_ARGS__, YAML_FOR_EACH_RSEQ_N())
#define YAML_FOR_EACH_NARG_(...) YAML_EXPAND(YAML_FOR_EACH_ARG_N(__VA_ARGS__))
#define YAML_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, N, ...) N
#define YAML_FOR_EACH_RSEQ_N() 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define YAML_CONCATENATE(x, y) x##y
#define YAML_FOR_EACH_(N, what, ...) YAML_EXPAND(YAML_CONCATENATE(YAML_FOR_EACH_, N)(what, __VA_ARGS__))
#define YAML_FOR_EACH(what, ...) YAML_FOR_EACH_(YAML_FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)
#define YAML_CALL_CONVERT(f) Engine::Yaml::convert(node, #f, f);
#define YAML_CALL_PACK(f) Engine::Yaml::pack(node, #f, f);
#define YAML_CALL_INSERT_TO_STRING(f) map.insert(std::make_pair(Type::f, #f))
#define YAML_CALL_INSERT_FROM_STRING(f) map.insert(std::make_pair(#f, Type::f))

#define YAML_DEFINE(...)                                                                                               \
    void convert(const Yaml::Node& node) {                                                                             \
        YAML_FOR_EACH(YAML_CALL_CONVERT, __VA_ARGS__)                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    void pack(Yaml::Node& node) const {                                                                                \
        YAML_FOR_EACH(YAML_CALL_PACK, __VA_ARGS__)                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    void fromYaml(const Path& p) {                                                                                     \
        try {                                                                                                          \
            auto node = Yaml::load(p);                                                                                 \
            if (!node || !node.isMap()) {                                                                              \
                EXCEPTION("Root is not a map");                                                                        \
            }                                                                                                          \
            Engine::Yaml::Adaptor<decltype(*this)>::convert(node, *this);                                              \
        } catch (...) {                                                                                                \
            EXCEPTION_NESTED("Failed to parse yaml document: '{}'", p.string());                                       \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void toYaml(const Path& p) const {                                                                                 \
        try {                                                                                                          \
            auto node = Yaml::Node::map();                                                                             \
            Engine::Yaml::Adaptor<decltype(*this)>::pack(node, *this);                                                 \
            Yaml::save(node, p);                                                                                       \
        } catch (...) {                                                                                                \
            EXCEPTION_NESTED("Failed to create yaml document: '{}'", p.string());                                      \
        }                                                                                                              \
    }

#define YAML_DEFINE_ENUM(Enum, ...)                                                                                    \
    template <> struct Engine::Yaml::Adaptor<Enum> {                                                                   \
        using Type = Enum;                                                                                             \
        static std::unordered_map<Enum, std::string> generateToStringMap() {                                           \
            std::unordered_map<Enum, std::string> map;                                                                 \
            YAML_FOR_EACH(YAML_CALL_INSERT_TO_STRING, __VA_ARGS__);                                                    \
            return map;                                                                                                \
        }                                                                                                              \
        static std::unordered_map<std::string, Enum> generateFromStringMap() {                                         \
            std::unordered_map<std::string, Enum> map;                                                                 \
            YAML_FOR_EACH(YAML_CALL_INSERT_FROM_STRING, __VA_ARGS__);                                                  \
            return map;                                                                                                \
        }                                                                                                              \
        static void convert(const Yaml::Node& node, Enum& value) {                                                     \
            static const auto map = generateFromStringMap();                                                           \
                                                                                                                       \
            if (!node)                                                                                                 \
                throw std::bad_cast();                                                                                 \
                                                                                                                       \
            const auto it = map.find(node.asString());                                                                 \
            if (it == map.end()) {                                                                                     \
                EXCEPTION(                                                                                             \
                    "Can not convert string: '{}' into enum of type: '{}'", node.asString(), typeid(Enum).name());     \
            }                                                                                                          \
            value = it->second;                                                                                        \
        }                                                                                                              \
        static void pack(Yaml::Node& node, const Enum& value) {                                                        \
            static const auto map = generateToStringMap();                                                             \
            const auto it = map.find(value);                                                                           \
            if (it == map.end()) {                                                                                     \
                EXCEPTION("Can not convert enum value: '{}' of type: '{}' into string, missing lookup pair",           \
                          int(value),                                                                                  \
                          typeid(Enum).name());                                                                        \
            }                                                                                                          \
            node.packString(it->second);                                                                               \
        }                                                                                                              \
    }

#define YAML_CALL_INSERT_TO_VARIANT(f)                                                                                 \
    if (type == #f) {                                                                                                  \
        value = f{};                                                                                                   \
        Yaml::Adaptor<f>::convert(data, std::get<f>(value));                                                           \
        return;                                                                                                        \
    }

#define YAML_CALL_INSERT_FROM_VARIANT(f)                                                                               \
    if (std::holds_alternative<f>(value)) {                                                                            \
        node.insert("type").packString(#f);                                                                            \
        auto data = node.insert("data");                                                                               \
        Engine::Yaml::Adaptor<f>::pack(data, std::get<f>(value));                                                      \
        return;                                                                                                        \
    }

#define YAML_DEFINE_VARIANT(Variant, ...)                                                                              \
    template <> struct Engine::Yaml::Adaptor<Variant> {                                                                \
        static void convert(const Yaml::Node& node, Variant& value) {                                                  \
            if (!node || !node.isMap()) {                                                                              \
                throw std::runtime_error("variant field must be a map");                                               \
            }                                                                                                          \
            if (!node.contains("type")) {                                                                              \
                throw std::runtime_error("variant field must have 'type' key");                                        \
            }                                                                                                          \
            if (!node.contains("data")) {                                                                              \
                throw std::runtime_error("variant field must have 'data' key");                                        \
            }                                                                                                          \
            const auto type = node.child("type").asString();                                                           \
            const auto& data = node.child("data");                                                                     \
            YAML_FOR_EACH(YAML_CALL_INSERT_TO_VARIANT, __VA_ARGS__);                                                   \
        }                                                                                                              \
        static void pack(Yaml::Node& node, const Variant& value) {                                                     \
            YAML_FOR_EACH(YAML_CALL_INSERT_FROM_VARIANT, __VA_ARGS__);                                                 \
        }                                                                                                              \
    }

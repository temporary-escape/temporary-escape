#pragma once

#include "../library.hpp"
#include "../math/quaternion.hpp"
#include "../math/vector.hpp"
#include "exceptions.hpp"
#include "macros.hpp"
#include "moveable_copyable.hpp"
#include "path.hpp"
#include <charconv>
#include <fstream>
#include <list>
#include <unordered_map>
#include <vector>

// Forward declaration
typedef struct _xmlDoc xmlDoc;
typedef xmlDoc* xmlDocPtr;
typedef struct _xmlNode xmlNode;
typedef xmlNode* xmlNodePtr;

namespace Engine::Xml {

class Node;

template <typename T> void convert(const Node& node, const std::string& key, T& value, bool required = true);
template <typename T> void pack(Node& node, const std::string& key, const T& value);

class ENGINE_API Node {
public:
    Node() = default;
    explicit Node(const Node* parent, const std::string& name);
    explicit Node(const Node* parent, xmlNodePtr node);
    ~Node();
    Node(const Node& other) = delete;
    Node(Node&& other) noexcept;
    Node& operator=(const Node& other) = delete;
    Node& operator=(Node&& other) noexcept;
    void swap(Node& other);

    [[nodiscard]] xmlNodePtr get() const {
        return node;
    }

    template <typename T> void pack(const std::string& key, const T& value) {
        Xml::pack(*this, key, value);
    }

    template <typename T> void convert(const std::string& key, T& value, bool required = true) const {
        Xml::convert(*this, key, value, required);
    }

    Node insert(const std::string& key);
    void setText(const std::string& text);
    [[nodiscard]] std::string_view name() const;
    [[nodiscard]] std::string_view getText() const;
    [[nodiscard]] Node next() const;
    [[nodiscard]] Node next(const std::string& name) const;
    [[nodiscard]] Node child() const;
    [[nodiscard]] Node child(const std::string& name) const;
    [[nodiscard]] bool empty() const;

    operator bool() const {
        return parent && node;
    }

    [[nodiscard]] std::string breadcrumbs() const;
    void throwWith(const std::string_view& msg) const {
        throw std::runtime_error(fmt::format("Unable to parse property: {} error: {}", breadcrumbs(), msg));
    }

private:
    const Node* parent{nullptr};
    xmlNodePtr node{nullptr};
    mutable unsigned char* contents{nullptr};
};

class ENGINE_API Document {
public:
    explicit Document(const std::string& raw);
    explicit Document(const std::string& rootName, const std::string& version);
    ~Document();
    NON_COPYABLE(Document);
    MOVEABLE(Document);

    [[nodiscard]] std::string toString() const;
    Node& getRoot() {
        return root;
    }
    [[nodiscard]] const Node& getRoot() const {
        return root;
    }

private:
    xmlDocPtr doc{nullptr};
    Node root;
};

namespace Detail {
template <typename T> struct DocumentDefinition;
} // namespace Detail

template <typename T> struct Adaptor {
    // Pack adaptor for all user defined types
    static void pack(Node& node, const T& value) {
        value.pack(node);
    }
    // Convert adaptor for all user defined types
    static void convert(const Node& node, T& value) {
        value.convert(node);
    }
};

template <> struct Adaptor<std::string> {
    static void pack(Node& node, const std::string& value) {
        node.setText(value);
    }
    static void convert(const Node& node, std::string& value) {
        value = node.getText();
    }
};

template <> struct Adaptor<bool> {
    static void pack(Node& node, const bool& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, bool& value) {
        const auto text = node.getText();
        if (text.empty()) {
            node.throwWith("empty bool value");
        } else if (text != "true" && text != "false") {
            node.throwWith(R"(invalid bool value, expected "true" or "false")");
        }
        value = text == "true";
    }
};

template <> struct Adaptor<float> {
    static void pack(Node& node, const float& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, float& value) {
        const auto text = node.getText();
        try {
            value = std::stof(std::string{text});
        } catch (std::exception&) {
            node.throwWith("invalid float value");
        }
    }
};

template <> struct Adaptor<double> {
    static void pack(Node& node, const double& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, double& value) {
        const auto text = node.getText();
        try {
            value = std::stod(std::string{text});
        } catch (std::exception&) {
            node.throwWith("invalid float value");
        }
    }
};

template <> struct Adaptor<int32_t> {
    static void pack(Node& node, const int32_t& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, int32_t& value) {
        const auto text = node.getText();
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            node.throwWith("invalid int32_t value");
        }
    }
};

template <> struct Adaptor<uint32_t> {
    static void pack(Node& node, const uint32_t& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, uint32_t& value) {
        const auto text = node.getText();
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            node.throwWith("invalid uint32_t value");
        }
    }
};

template <> struct Adaptor<int64_t> {
    static void pack(Node& node, const int64_t& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, int64_t& value) {
        const auto text = node.getText();
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            node.throwWith("invalid int64_t value");
        }
    }
};

template <> struct Adaptor<uint64_t> {
    static void pack(Node& node, const uint64_t& value) {
        node.setText(fmt::format("{}", value));
    }
    static void convert(const Node& node, uint64_t& value) {
        const auto text = node.getText();
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            node.throwWith("invalid uint64_t value");
        }
    }
};

template <> struct Adaptor<Vector2> {
    static void pack(Node& node, const Vector2& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
    }
    static void convert(const Node& node, Vector2& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
    }
};

template <> struct Adaptor<Vector2i> {
    static void pack(Node& node, const Vector2i& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
    }
    static void convert(const Node& node, Vector2i& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
    }
};

template <> struct Adaptor<Vector3> {
    static void pack(Node& node, const Vector3& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
        node.pack("z", value.z);
    }
    static void convert(const Node& node, Vector3& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
        node.convert("z", value.z);
    }
};

template <> struct Adaptor<Vector3i> {
    static void pack(Node& node, const Vector3i& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
        node.pack("z", value.z);
    }
    static void convert(const Node& node, Vector3i& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
        node.convert("z", value.z);
    }
};

template <> struct Adaptor<Vector4> {
    static void pack(Node& node, const Vector4& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
        node.pack("z", value.z);
        node.pack("w", value.w);
    }
    static void convert(const Node& node, Vector4& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
        node.convert("z", value.z);
        node.convert("w", value.w);
    }
};

template <> struct Adaptor<Quaternion> {
    static void pack(Node& node, const Quaternion& value) {
        node.pack("x", value.x);
        node.pack("y", value.y);
        node.pack("z", value.z);
        node.pack("w", value.w);
    }
    static void convert(const Node& node, Quaternion& value) {
        node.convert("x", value.x);
        node.convert("y", value.y);
        node.convert("z", value.z);
        node.convert("w", value.w);
    }
};

template <typename T> struct Container {
    static void pack(Node& node, const std::string& key, const T& value) {
        auto child = node.insert(key);
        Adaptor<T>::pack(child, value);
    }
    static void convert(const Node& node, const std::string& key, T& value, bool required) {
        auto child = node.child(key);
        if (!child) {
            if (!required) {
                return;
            }
            throw std::invalid_argument(fmt::format("Missing property: {}/{}", node.breadcrumbs(), key));
        } else {
            Adaptor<T>::convert(child, value);
        }
    }
};

template <typename T> struct Container<std::vector<T>> {
    static void pack(Node& node, const std::string& key, const std::vector<T>& value) {
        for (const auto& item : value) {
            auto child = node.insert(key);
            Adaptor<T>::pack(child, item);
        }
    }
    static void convert(const Node& node, const std::string& key, std::vector<T>& value, bool required) {
        (void)required;
        auto child = node.child(key);
        while (child) {
            Adaptor<T>::convert(child, value.emplace_back());
            child = child.next(key);
        }
    }
};

template <typename T> struct Container<std::list<T>> {
    static void pack(Node& node, const std::string& key, const std::list<T>& value) {
        for (const auto& item : value) {
            auto child = node.insert(key);
            Adaptor<T>::pack(child, item);
        }
    }
    static void convert(const Node& node, const std::string& key, std::list<T>& value, bool required) {
        (void)required;
        auto child = node.child(key);
        while (child) {
            Adaptor<T>::convert(child, value.emplace_back());
            child = child.next(key);
        }
    }
};

template <typename T> struct Container<std::optional<T>> {
    static void pack(Node& node, const std::string& key, const std::optional<T>& value) {
        if (value) {
            auto child = node.insert(key);
            Adaptor<T>::pack(child, *value);
        }
    }
    static void convert(const Node& node, const std::string& key, std::optional<T>& value, bool required) {
        (void)required;
        auto child = node.child(key);
        if (child && !child.empty()) {
            value = T{};
            Adaptor<T>::convert(child, *value);
        }
    }
};

template <typename T> struct Container<std::unordered_map<std::string, T>> {
    static void pack(Node& node, const std::string& key, const std::unordered_map<std::string, T>& value) {
        auto child = node.insert(key);
        for (const auto& pair : value) {
            auto grandChild = child.insert(pair.first);
            Adaptor<T>::pack(grandChild, pair.second);
        }
    }
    static void convert(const Node& node, const std::string& key, std::unordered_map<std::string, T>& value,
                        bool required) {
        (void)required;
        auto child = node.child(key);
        if (child && !child.empty()) {
            auto grandChild = child.child();
            while (grandChild) {
                auto& item = value[std::string{grandChild.name()}];
                Adaptor<T>::convert(grandChild, item);
                grandChild = grandChild.next();
            }
        }
    }
};

template <typename T> inline void pack(Node& node, const std::string& key, const T& value) {
    Container<T>::pack(node, key, value);
}

template <typename T> inline void convert(const Node& node, const std::string& key, T& value, bool required) {
    Container<T>::convert(node, key, value, required);
}

template <typename T> inline std::string dump(const T& value) {
    Document doc{Detail::DocumentDefinition<T>::getName(), Detail::DocumentDefinition<T>::getVersion()};
    value.pack(doc.getRoot());
    return doc.toString();
}

template <typename T> inline void load(T& value, const std::string& raw) {
    Document doc{raw};
    value.convert(doc.getRoot());
}

template <typename T> inline void toFile(const Path& path, const T& value) {
    std::fstream file{path, std::ios::out};
    if (!file) {
        EXCEPTION("Failed to open file for writing: {}", path);
    }
    file << dump(value);
}

template <typename T> inline void fromFile(const Path& path, T& value) {
    std::fstream file{path, std::ios::in};
    if (!file) {
        EXCEPTION("Failed to open file for reading: {}", path);
    }
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    load(value, str);
}
} // namespace Engine::Xml

#define XML_DEFINE(Type, RootName)                                                                                     \
    template <> struct Xml::Detail::DocumentDefinition<Type> {                                                         \
        static const char* getName() {                                                                                 \
            return RootName;                                                                                           \
        }                                                                                                              \
        static const char* getVersion() {                                                                              \
            return "1.0";                                                                                              \
        }                                                                                                              \
    }

#define XML_DEFINE_ENUM_PACK(Field)                                                                                    \
    if (value == T::Field) {                                                                                           \
        node.setText(#Field);                                                                                          \
        return;                                                                                                        \
    }

#define XML_DEFINE_ENUM_CONVERT(Field)                                                                                 \
    if (text == #Field) {                                                                                              \
        value = T::Field;                                                                                              \
        return;                                                                                                        \
    }

#define XML_DEFINE_ENUM(Type, ...)                                                                                     \
    template <> struct Xml::Adaptor<Type> {                                                                            \
        using T = Type;                                                                                                \
        static void pack(Node& node, const Type& value) {                                                              \
            FOR_EACH(XML_DEFINE_ENUM_PACK, __VA_ARGS__)                                                                \
        }                                                                                                              \
        static void convert(const Node& node, Type& value) {                                                           \
            const auto text = node.getText();                                                                          \
            FOR_EACH(XML_DEFINE_ENUM_CONVERT, __VA_ARGS__)                                                             \
            node.throwWith(fmt::format("unknown enum value: \"{}\"", text));                                           \
        }                                                                                                              \
    }

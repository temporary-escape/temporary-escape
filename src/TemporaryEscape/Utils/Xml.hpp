#pragma once
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

struct _xmlDoc;
struct _xmlSchema;
struct _xmlSchemaParserCtxt;
struct _xmlSchemaValidCtxt;
struct _xmlNode;
struct _xmlAttr;

namespace Engine {
namespace Xml {
class ENGINE_API Schema {
public:
    explicit Schema(const std::string& src);
    explicit Schema(const Path& path);

    [[nodiscard]] _xmlSchemaValidCtxt* getValidCtx() const {
        return valid.get();
    }

private:
    void load();

    std::shared_ptr<_xmlSchema> schema;
    std::shared_ptr<_xmlSchemaParserCtxt> ctx;
    std::shared_ptr<_xmlSchemaValidCtxt> valid;
};

class ENGINE_API Attribute {
public:
    Attribute() = default;
    Attribute(std::shared_ptr<_xmlDoc> doc, const _xmlAttr* attr);
    ~Attribute() = default;

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] std::string asString() const;
    [[nodiscard]] int64_t asLong() const;
    [[nodiscard]] double asDouble() const;
    [[nodiscard]] bool asBool() const;
    [[nodiscard]] float asFloat() const {
        return static_cast<float>(asDouble());
    }

private:
    std::shared_ptr<_xmlDoc> doc;
    const _xmlAttr* attr = nullptr;
    std::string name;
};

class Node;

template <typename T> class Iterator {
public:
    Iterator() = default;
    explicit Iterator(T node) : node(std::move(node)) {
    }
    ~Iterator() = default;

    T operator*() const {
        return node;
    }
    T operator->() const {
        return node;
    }

    Iterator<T>& operator++() {
        if (node.hasNext()) {
            node = node.next();
        } else {
            node = T{};
        }
        return *this;
    }

    friend bool operator==(const Iterator& a, const Iterator& b) {
        return a.node == b.node;
    }
    friend bool operator!=(const Iterator& a, const Iterator& b) {
        return a.node != b.node;
    }

private:
    T node;
};

class Node;
class NewNode;

template <typename T> struct Adaptor { static void convert(const Xml::Node& node, T& value); };

template <typename T> struct Writer { static void write(Xml::NewNode& node, const T& value); };

class ENGINE_API Node {
public:
    Node() = default;
    Node(std::shared_ptr<_xmlDoc> doc, const _xmlNode* node);
    ~Node() = default;

    [[nodiscard]] bool hasNext() const;
    [[nodiscard]] bool hasNext(const std::string& name) const;
    [[nodiscard]] Node next() const;
    [[nodiscard]] Node next(const std::string& name) const;
    [[nodiscard]] Node child() const;
    [[nodiscard]] Node child(const std::string& name) const;
    [[nodiscard]] bool hasChild(const std::string& name) const;
    [[nodiscard]] std::optional<Attribute> attribute(const std::string& name) const;
    [[nodiscard]] std::string asString() const;
    [[nodiscard]] int64_t asLong() const;
    [[nodiscard]] double asDouble() const;
    [[nodiscard]] bool asBool() const;
    [[nodiscard]] float asFloat() const {
        return static_cast<float>(asDouble());
    }

    Iterator<Node> begin() const;
    Iterator<Node> end() const;

    [[nodiscard]] const std::string& getName() const;

    template <typename T> void convert(T& value) const {
        Adaptor<T>::convert(*this, value);
    }

    friend bool operator==(const Node& a, const Node& b) {
        return a.node == b.node;
    }
    friend bool operator!=(const Node& a, const Node& b) {
        return a.node != b.node;
    }

private:
    std::shared_ptr<_xmlDoc> doc;
    const _xmlNode* node = nullptr;
    std::string name;
};

class ENGINE_API Document {
public:
    explicit Document(const Path& path);
    void validate(const Schema& schema) const;
    [[nodiscard]] Node getRoot() const;

private:
    std::shared_ptr<_xmlDoc> doc;
};

template <typename T> inline void loadAsXml(const Path& path, T& value) {
    Document(path).getRoot().convert(value);
}

template <typename T> struct Adaptor<std::vector<T>> {
    static void convert(const Xml::Node& node, std::vector<T>& value) {
        for (const auto& child : node) {
            T v{};
            child.template convert<T>(v);
            value.push_back(std::move(v));
        }
    }
};

template <> struct Adaptor<std::string> {
    static void convert(const Xml::Node& node, std::string& value) {
        value = node.asString();
    }
};

template <> struct Adaptor<int32_t> {
    static void convert(const Xml::Node& node, int32_t& value) {
        value = static_cast<int32_t>(node.asLong());
    }
};

template <> struct Adaptor<int64_t> {
    static void convert(const Xml::Node& node, int64_t& value) {
        value = static_cast<int64_t>(node.asLong());
    }
};

template <> struct Adaptor<uint32_t> {
    static void convert(const Xml::Node& node, uint32_t& value) {
        value = static_cast<uint32_t>(node.asLong());
    }
};

template <> struct Adaptor<uint64_t> {
    static void convert(const Xml::Node& node, uint64_t& value) {
        value = static_cast<uint64_t>(node.asLong());
    }
};

template <> struct Adaptor<float> {
    static void convert(const Xml::Node& node, float& value) {
        value = static_cast<float>(node.asDouble());
    }
};

template <> struct Adaptor<double> {
    static void convert(const Xml::Node& node, double& value) {
        value = node.asDouble();
    }
};

template <> struct Adaptor<bool> {
    static void convert(const Xml::Node& node, bool& value) {
        value = node.asBool();
    }
};

class ENGINE_API NewNode {
public:
    explicit NewNode(std::shared_ptr<_xmlDoc> doc, _xmlNode* node);
    NewNode child(const std::string& name);
    void asString(const std::string& value);
    void asBool(bool value);
    void asLong(int64_t value);
    void asFloat(float value);
    void asDouble(double value);
    template <typename T> void write(const T& value) {
        Writer<T>::read(*this, value);
    }

private:
    std::shared_ptr<_xmlDoc> doc;
    _xmlNode* node = nullptr;
};

class ENGINE_API NewDocument {
public:
    explicit NewDocument(const Path& path);
    NewNode getRoot(const std::string& name);
    void save();

private:
    Path path;
    std::shared_ptr<_xmlDoc> doc;
};

template <> struct Writer<std::string> {
    static void read(Xml::NewNode& node, const std::string& value) {
        node.asString(value);
    }
};

template <> struct Writer<int32_t> {
    static void read(Xml::NewNode& node, const int32_t value) {
        node.asLong(static_cast<int64_t>(value));
    }
};

template <> struct Writer<int64_t> {
    static void read(Xml::NewNode& node, const int64_t value) {
        node.asLong(value);
    }
};

template <> struct Writer<uint32_t> {
    static void read(Xml::NewNode& node, const uint32_t value) {
        node.asLong(static_cast<int64_t>(value));
    }
};

template <> struct Writer<uint64_t> {
    static void read(Xml::NewNode& node, const uint64_t value) {
        node.asLong(static_cast<int64_t>(value));
    }
};

template <> struct Writer<bool> {
    static void read(Xml::NewNode& node, const bool value) {
        node.asBool(value);
    }
};

template <> struct Writer<float> {
    static void read(Xml::NewNode& node, const float value) {
        node.asFloat(value);
    }
};

template <> struct Writer<double> {
    static void read(Xml::NewNode& node, const double value) {
        node.asDouble(value);
    }
};

template <typename T> inline void saveAsXml(const Path& path, const std::string& root, const T& value) {
    NewDocument doc(path);
    doc.getRoot(root).write(value);
    doc.save();
}

template <> struct Adaptor<Vector2> {
    static void convert(const Xml::Node& n, Vector2& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
    }
};

template <> struct Adaptor<Vector2i> {
    static void convert(const Xml::Node& n, Vector2i& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
    }
};

template <> struct Adaptor<Vector3> {
    static void convert(const Xml::Node& n, Vector3& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
        n.child("z").convert(v.z);
    }
};

template <> struct Adaptor<Vector3i> {
    static void convert(const Xml::Node& n, Vector3i& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
        n.child("z").convert(v.z);
    }
};

template <> struct Adaptor<Vector4> {
    static void convert(const Xml::Node& n, Vector4& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
        n.child("z").convert(v.z);
        n.child("w").convert(v.w);
    }
};

template <> struct Adaptor<Vector4i> {
    static void convert(const Xml::Node& n, Vector4i& v) {
        n.child("x").convert(v.x);
        n.child("y").convert(v.y);
        n.child("z").convert(v.z);
        n.child("w").convert(v.w);
    }
};
}; // namespace Xml

} // namespace Engine

#pragma once
#include "../Library.hpp"
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

namespace Scissio {
namespace Xml {
class SCISSIO_API Schema {
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

class SCISSIO_API Attribute {
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

template <typename T> struct Adaptor { static void convert(const Xml::Node& node, T& value); };

class SCISSIO_API Node {
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

class SCISSIO_API Document {
public:
    explicit Document(const Path& path);
    void validate(const Schema& schema) const;
    [[nodiscard]] Node getRoot() const;

private:
    std::shared_ptr<_xmlDoc> doc;
};

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
}; // namespace Xml

} // namespace Scissio

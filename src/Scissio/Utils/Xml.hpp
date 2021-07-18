#pragma once
#include "../Library.hpp"
#include "Path.hpp"
#include <optional>
#include <unordered_map>

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
    Schema(const std::string& src);
    Schema(const Path& path);

    _xmlSchemaValidCtxt* getValidCtx() const {
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

    const std::string& getName() const;
    std::string asString() const;
    int64_t asLong() const;
    double asDouble() const;
    bool asBool() const;

private:
    std::shared_ptr<_xmlDoc> doc;
    const _xmlAttr* attr = nullptr;
    std::string name;
};

class Node;

template <typename T> class Iterator {
public:
    Iterator() = default;
    Iterator(T node) : node(std::move(node)) {
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

class SCISSIO_API Node {
public:
    Node() = default;
    Node(std::shared_ptr<_xmlDoc> doc, const _xmlNode* node);
    ~Node() = default;

    bool hasNext() const;
    bool hasNext(const std::string& name) const;
    Node next() const;
    Node next(const std::string& name) const;
    Node child() const;
    Node child(const std::string& name) const;
    Attribute attribute(const std::string& name) const;
    std::string asString() const;
    int64_t asLong() const;
    double asDouble() const;
    bool asBool() const;

    Iterator<Node> begin();
    Iterator<Node> end();

    const std::string& getName() const;

    template <typename T> void convert(T& value) const {
        T::convert(*this, value);
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
    Node getRoot() const;

private:
    std::shared_ptr<_xmlDoc> doc;
};

template <> inline void Node::convert<std::string>(std::string& value) const {
    value = this->asString();
}

template <> inline void Node::convert<int64_t>(int64_t& value) const {
    value = this->asLong();
}

template <> inline void Node::convert<int>(int& value) const {
    value = static_cast<int>(this->asLong());
}

template <> inline void Node::convert<float>(float& value) const {
    value = static_cast<float>(this->asDouble());
}

template <> inline void Node::convert<double>(double& value) const {
    value = this->asDouble();
}

template <> inline void Node::convert<bool>(bool& value) const {
    value = this->asBool();
}
}; // namespace Xml

} // namespace Scissio

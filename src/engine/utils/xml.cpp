#include "xml.hpp"
#include "exceptions.hpp"
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

using namespace Engine;

class XmlParserSingleton {
public:
    XmlParserSingleton() {
        xmlInitParser();
    }
    ~XmlParserSingleton() {
        xmlCleanupParser();
    }
};

[[maybe_unused]] static XmlParserSingleton xmlParser{};

Xml::Node::Node(const Node* parent, const std::string& name) : parent{parent} {
    node = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>(name.c_str()));
}

Xml::Node::Node(const Node* parent, xmlNodePtr node) : parent{parent}, node{node} {
}

Xml::Node::~Node() {
    if (contents) {
        xmlFree(contents);
    }
}

Xml::Node::Node(Xml::Node&& other) noexcept {
    swap(other);
}

Xml::Node& Xml::Node::operator=(Xml::Node&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Xml::Node::swap(Xml::Node& other) {
    std::swap(parent, other.parent);
    std::swap(node, other.node);
    std::swap(contents, other.contents);
}

Xml::Node Xml::Node::insert(const std::string& key) {
    Node child{this, key};
    xmlAddChild(node, child.get());
    return child;
}

void Xml::Node::setText(const std::string& text) {
    auto child = xmlNewText(reinterpret_cast<const xmlChar*>(text.c_str()));
    xmlAddChild(node, child);
}

std::string_view Xml::Node::getText() const {
    if (node) {
        if (!contents) {
            contents = xmlNodeGetContent(node);
        }
        return reinterpret_cast<const char*>(contents);
    }

    return "";
}

std::string_view Xml::Node::name() const {
    return node ? reinterpret_cast<const char*>(node->name) : "";
}

Xml::Node Xml::Node::next() const {
    auto cur = node ? node->next : nullptr;
    while (cur) {
        if (cur->type == XML_ELEMENT_NODE) {
            return Node{this, cur};
        }

        cur = cur->next;
    }

    return Node{};
}

Xml::Node Xml::Node::next(const std::string& name) const {
    auto cur = node ? node->next : nullptr;
    while (cur) {
        if (cur->type == XML_ELEMENT_NODE && name == reinterpret_cast<const char*>(cur->name)) {
            return Node{this, cur};
        }

        cur = cur->next;
    }

    return Node{};
}

Xml::Node Xml::Node::child() const {
    auto cur = node ? node->children : nullptr;
    while (cur) {
        if (cur->type == XML_ELEMENT_NODE) {
            return Node{this, cur};
        }

        cur = cur->next;
    }

    return Node{};
}

Xml::Node Xml::Node::child(const std::string& name) const {
    auto cur = node ? node->children : nullptr;
    while (cur) {
        if (cur->type == XML_ELEMENT_NODE && name == reinterpret_cast<const char*>(cur->name)) {
            return Node{this, cur};
        }

        cur = cur->next;
    }

    return Node{};
}

bool Xml::Node::empty() const {
    return node == nullptr || node->children == nullptr;
}

std::string Xml::Node::breadcrumbs() const {
    if (parent) {
        return fmt::format("{}/{}", parent->breadcrumbs(), name());
    }
    return fmt::format("{}", name());
}

Xml::Document::Document(const std::string& raw) {
    doc = xmlParseDoc(reinterpret_cast<const xmlChar*>(raw.c_str()));
    root = Node{nullptr, xmlDocGetRootElement(doc)};
    if (!root.get()) {
        throw std::runtime_error("XML document has no root node");
    }
}

Xml::Document::Document(const std::string& rootName, const std::string& version) {
    doc = xmlNewDoc(reinterpret_cast<const xmlChar*>(version.c_str()));
    root = Node{nullptr, rootName};
    xmlDocSetRootElement(doc, root.get());

    // xmlCreateIntSubset(doc, reinterpret_cast<const xmlChar*>(version.c_str()), nullptr, BAD_CAST "tree2.dtd");
}

Xml::Document::~Document() {
    if (doc) {
        xmlFreeDoc(doc);
    }
}

std::string Xml::Document::toString() const {
    // auto writerOutput = xmlBufferCreate();
    // auto writer = xmlNewTextWriterMemory(writerOutput, 0);

    const auto deleter = [](xmlBuffer* p) { xmlBufferFree(p); };
    auto buffer = std::unique_ptr<xmlBuffer, decltype(deleter)>(xmlBufferCreate(), deleter);
    auto* outputBuffer = xmlOutputBufferCreateBuffer(buffer.get(), nullptr);

    const auto res = xmlSaveFormatFileTo(outputBuffer, doc, "utf-8", 1);
    if (res <= 0) {
        EXCEPTION("Failed to print XML document");
    }

    return reinterpret_cast<const char*>(buffer->content);
}

#include "Xml.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <mutex>
#include <sstream>

using namespace Scissio;

static std::mutex mutex;

static std::string format(const char* message) {
    std::string s(message);
    while (s.back() == '\n') {
        s.pop_back();
    }
    return s;
}

static void defaultHandler(void* userData, const xmlErrorPtr error) {
    auto& ss = *reinterpret_cast<std::stringstream*>(userData);
    ss << "\n" << format(error->message) << " at line " << error->line;
}

Xml::Document::Document(const Path& path) {
    std::lock_guard<std::mutex> lock{mutex};

    const auto options = XML_PARSE_NONET;
    const auto pathStr = path.string();
    std::stringstream ss;

    xmlSetStructuredErrorFunc(&ss, defaultHandler);

    _xmlDoc* ptr = xmlReadFile(pathStr.c_str(), "UTF-8", options);
    if (!ptr) {
        EXCEPTION("Failed to parse xml file: {} error:{}", path.string(), ss.str());
    }

    doc = std::shared_ptr<_xmlDoc>(ptr, [](_xmlDoc* doc) { xmlFreeDoc(doc); });
}

void Xml::Document::validate(const Schema& schema) const {
    std::lock_guard<std::mutex> lock{mutex};
    std::stringstream ss;

    xmlSetStructuredErrorFunc(&ss, defaultHandler);
    xmlSchemaSetValidStructuredErrors(schema.getValidCtx(), defaultHandler, &ss);

    if (xmlSchemaValidateDoc(schema.getValidCtx(), doc.get()) != 0) {
        EXCEPTION("Validation error:{}", ss.str());
    }
}

Xml::Schema::Schema(const std::string& src) {
    std::lock_guard<std::mutex> lock{mutex};

    const auto ctxPtr = xmlSchemaNewMemParserCtxt(src.data(), static_cast<int>(src.size()));
    if (!ctxPtr) {
        EXCEPTION("Failed to create schema parsing context");
    }

    ctx =
        std::shared_ptr<_xmlSchemaParserCtxt>(ctxPtr, [](_xmlSchemaParserCtxt* ctx) { xmlSchemaFreeParserCtxt(ctx); });

    load();
}

Xml::Schema::Schema(const Path& path) {
    std::lock_guard<std::mutex> lock{mutex};

    const auto pathStr = path.string();
    const auto ctxPtr = xmlSchemaNewParserCtxt(pathStr.c_str());
    if (!ctxPtr) {
        EXCEPTION("Failed to create schema parsing context");
    }

    ctx =
        std::shared_ptr<_xmlSchemaParserCtxt>(ctxPtr, [](_xmlSchemaParserCtxt* ctx) { xmlSchemaFreeParserCtxt(ctx); });

    load();
}

void Xml::Schema::load() {
    std::stringstream ss;

    xmlSetStructuredErrorFunc(&ss, defaultHandler);
    xmlSchemaSetParserStructuredErrors(ctx.get(), defaultHandler, &ss);

    const auto schemaPtr = xmlSchemaParse(ctx.get());
    if (!schemaPtr) {
        EXCEPTION("Failed to parse schema error:{}", ss.str());
    }

    ss.clear();

    schema = std::shared_ptr<_xmlSchema>(schemaPtr, [](_xmlSchema* schema) { xmlSchemaFree(schema); });

    const auto validPtr = xmlSchemaNewValidCtxt(schema.get());
    if (!validPtr) {
        EXCEPTION("Failed to create validation context");
    }

    valid = std::shared_ptr<_xmlSchemaValidCtxt>(validPtr,
                                                 [](_xmlSchemaValidCtxt* valid) { xmlSchemaFreeValidCtxt(valid); });

    xmlSchemaSetValidOptions(valid.get(), XML_SCHEMA_VAL_VC_I_CREATE);

    // xmlSchemaSetValidStructuredErrors(valid.get(), defaultHandler, &ss);

    // xmlSchemaSetValidErrors(valid.get(), (xmlSchemaValidityErrorFunc)err, (xmlSchemaValidityWarningFunc)err, NULL);
}

Xml::Attribute::Attribute(std::shared_ptr<_xmlDoc> doc, const _xmlAttr* attr) : doc(std::move(doc)), attr(attr) {
    if (attr) {
        name = reinterpret_cast<const char*>(attr->name);
    }
}

const std::string& Xml::Attribute::getName() const {
    return name;
}

std::string Xml::Attribute::asString() const {
    if (attr->children && attr->children->type == XML_TEXT_NODE) {
        return reinterpret_cast<const char*>(attr->children->content);
    }
    throw std::out_of_range(fmt::format("Xml node '{}' has no value", name));
}

int64_t Xml::Attribute::asLong() const {
    const auto s = asString();
    return std::stoll(s, nullptr, 10);
}

double Xml::Attribute::asDouble() const {
    const auto s = asString();
    return std::stod(s, nullptr);
}

bool Xml::Attribute::asBool() const {
    const auto s = asString();
    return s == "true" || s == "True" || s == "TRUE";
}

Xml::Node::Node(std::shared_ptr<_xmlDoc> doc, const _xmlNode* node) : doc(std::move(doc)), node(node) {
    if (node) {
        name = reinterpret_cast<const char*>(node->name);
    }
}

const std::string& Xml::Node::getName() const {
    return name;
}

std::string Xml::Node::asString() const {
    if (node->children && node->children->type == XML_TEXT_NODE) {
        return reinterpret_cast<const char*>(node->children->content);
    }
    throw std::out_of_range(fmt::format("Xml attribute '{}' has no value", name));
}

int64_t Xml::Node::asLong() const {
    const auto s = asString();
    return std::stoll(s, nullptr, 10);
}

double Xml::Node::asDouble() const {
    const auto s = asString();
    return std::stod(s, nullptr);
}

bool Xml::Node::asBool() const {
    const auto s = asString();
    return s == "true" || s == "True" || s == "TRUE";
}

bool Xml::Node::hasNext() const {
    for (const xmlNode* cur = node->next; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            return true;
        }
    }
    return false;
}

bool Xml::Node::hasNext(const std::string& name) const {
    for (const xmlNode* cur = node->next; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (std::strcmp(name.c_str(), reinterpret_cast<const char*>(cur->name)) == 0) {
                return true;
            }
        }
    }
    return false;
}

Xml::Iterator<Xml::Node> Xml::Node::begin() {
    return Iterator<Node>(child());
}

Xml::Iterator<Xml::Node> Xml::Node::end() {
    return Iterator<Node>(Node{});
}

Xml::Node Xml::Node::next() const {
    for (const xmlNode* cur = node->next; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            return Node(doc, cur);
        }
    }

    throw std::out_of_range("Xml node has no neighbor node");
}

Xml::Node Xml::Node::next(const std::string& name) const {
    for (const xmlNode* cur = node->next; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (std::strcmp(name.c_str(), reinterpret_cast<const char*>(cur->name)) == 0) {
                return Node(doc, cur);
            }
        }
    }

    throw std::out_of_range("Xml node has no neighbor node");
}

Xml::Node Xml::Node::child() const {
    for (const xmlNode* cur = node->children; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            return Node(doc, cur);
        }
    }

    throw std::out_of_range("Xml node has no child node");
}

Xml::Node Xml::Node::child(const std::string& name) const {
    for (const xmlNode* cur = node->children; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (std::strcmp(name.c_str(), reinterpret_cast<const char*>(cur->name)) == 0) {
                return Node(doc, cur);
            }
        }
    }

    throw std::out_of_range(fmt::format("Xml node has no child node '{}'", name));
}

Xml::Attribute Xml::Node::attribute(const std::string& name) const {
    for (const xmlAttr* cur = node->properties; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ATTRIBUTE_NODE) {
            if (std::strcmp(name.c_str(), reinterpret_cast<const char*>(cur->name)) == 0) {
                return Attribute(doc, cur);
            }
        }
    }

    throw std::out_of_range(fmt::format("Xml node has no attribute '{}'", name));
}

Xml::Node Xml::Document::getRoot() const {
    return {doc, xmlDocGetRootElement(doc.get())};
}

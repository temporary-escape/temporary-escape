#include "yaml.hpp"
#include <fstream>
#include <yaml-cpp/yaml.h>

using namespace Engine;

Yaml::Node::Node(std::unique_ptr<YAML::Node> yaml) : yaml(std::move(yaml)) {
}

Yaml::Node::~Node() = default;

Yaml::Node::Node(Yaml::Node&& other) noexcept : yaml(std::move(other.yaml)) {
}

Yaml::Node& Yaml::Node::operator=(Yaml::Node&& other) noexcept {
    if (this != &other) {
        std::swap(yaml, other.yaml);
    }
    return *this;
}

Yaml::Node Yaml::Node::map() {
    return Yaml::Node(std::make_unique<YAML::Node>());
}

Yaml::Node Yaml::Node::array() {
    return Yaml::Node(std::make_unique<YAML::Node>());
}

int8_t Yaml::Node::asInt8() const {
    return yaml->as<int8_t>();
}

int16_t Yaml::Node::asInt16() const {
    return yaml->as<int16_t>();
}

int32_t Yaml::Node::asInt32() const {
    return yaml->as<int32_t>();
}

int64_t Yaml::Node::asInt64() const {
    return yaml->as<int64_t>();
}

uint8_t Yaml::Node::asUint8() const {
    return yaml->as<uint8_t>();
}

uint16_t Yaml::Node::asUint16() const {
    return yaml->as<uint16_t>();
}

uint32_t Yaml::Node::asUint32() const {
    return yaml->as<uint32_t>();
}

uint64_t Yaml::Node::asUint64() const {
    return yaml->as<uint64_t>();
}

double Yaml::Node::asDouble() const {
    return yaml->as<double>();
}

float Yaml::Node::asFloat() const {
    return yaml->as<float>();
}

bool Yaml::Node::asBool() const {
    return yaml->as<bool>();
}

std::string Yaml::Node::asString() const {
    return yaml->as<std::string>();
}

void Yaml::Node::packInt8(int8_t value) {
    *yaml = value;
}

void Yaml::Node::packInt16(int16_t value) {
    *yaml = value;
}

void Yaml::Node::packInt32(int32_t value) {
    *yaml = value;
}

void Yaml::Node::packInt64(int64_t value) {
    *yaml = value;
}

void Yaml::Node::packUint8(uint8_t value) {
    *yaml = value;
}

void Yaml::Node::packUint16(uint16_t value) {
    *yaml = value;
}

void Yaml::Node::packUint32(uint32_t value) {
    *yaml = value;
}

void Yaml::Node::packUint64(uint64_t value) {
    *yaml = value;
}

void Yaml::Node::packFloat(float value) {
    *yaml = value;
}

void Yaml::Node::packDouble(double value) {
    *yaml = value;
}

void Yaml::Node::packBool(bool value) {
    *yaml = value;
}

void Yaml::Node::packString(const std::string& value) {
    *yaml = value;
}

void Yaml::Node::packNull() {
    *yaml = YAML::Node{};
}

bool Yaml::Node::contains(const std::string& key) const {
    return bool(yaml->operator[](key));
}

Yaml::Node Yaml::Node::child(const std::string& key) const {
    if (!yaml->IsMap()) {
        throw std::runtime_error("Node is not a map");
    }
    if (!yaml->operator[](key)) {
        return Yaml::Node(nullptr);
        // throw std::runtime_error(fmt::format("Key: '{}' does not exist in the map", key));
    }
    return Yaml::Node(std::make_unique<YAML::Node>(yaml->operator[](key)));
}

Yaml::Node Yaml::Node::append() {
    yaml->push_back(YAML::Node{});
    return Yaml::Node(std::make_unique<YAML::Node>(yaml->operator[](yaml->size() - 1)));
}

Yaml::Node Yaml::Node::insert(const std::string& key) {
    auto empty = std::make_unique<YAML::Node>();
    yaml->operator[](key) = *empty;
    return Yaml::Node(std::move(empty));
}

bool Yaml::Node::isMap() const {
    return yaml->IsMap();
}

bool Yaml::Node::isNull() const {
    return yaml->IsNull();
}

bool Yaml::Node::isSequence() const {
    return yaml->IsSequence();
}

size_t Yaml::Node::size() const {
    return yaml->size();
}

void Yaml::Node::forEach(const std::function<void(const Node&)>& fn) const {
    for (std::size_t i = 0; i < yaml->size(); i++) {
        fn(Yaml::Node(std::make_unique<YAML::Node>(yaml->operator[](i))));
    }
}

void Yaml::Node::forEach(const std::function<void(const std::string&, const Node&)>& fn) const {
    for (YAML::const_iterator it = yaml->begin(); it != yaml->end(); ++it) {
        const auto key = it->first.as<std::string>();
        fn(key, Yaml::Node(std::make_unique<YAML::Node>(it->second)));
    }
}

/*Yaml::Node Yaml::load(const std::string& document) {
    return Yaml::Node(std::make_unique<YAML::Node>(YAML::Load(document)));
}*/

Yaml::Node Yaml::load(const Path& path) {
    try {
        return Yaml::Node(std::make_unique<YAML::Node>(YAML::LoadFile(path.string())));
    } catch (std::exception& e) {
        EXCEPTION("Failed to load yaml document: '{}' error: {}", path.string(), e.what());
    }
}

void Yaml::save(const Node& node, const Path& path) {
    try {
        std::fstream file(path, std::ios::out);
        file << YAML::Dump(*node.yaml);
    } catch (std::exception& e) {
        EXCEPTION("Failed to save yaml document: '{}' error: {}", path.string(), e.what());
    }
}

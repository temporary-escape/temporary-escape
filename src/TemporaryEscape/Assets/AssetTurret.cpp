#include "AssetTurret.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetTurret::AssetTurret(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetTurret::load(AssetManager& assetManager) {
    try {
        Xml::Document(path).getRoot().convert(definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load turret: '{}'", getName());
    }
}

std::shared_ptr<AssetTurret> AssetTurret::from(const std::string& name) {
    return AssetManager::singleton().find<AssetTurret>(name);
}

void Xml::Adaptor<AssetTurret::Definition>::convert(const Xml::Node& n, AssetTurret::Definition& v) {
    n.child("title").convert(v.title);
    n.child("components").convert(v.components);
}

void Xml::Adaptor<AssetTurret::Definition::Component>::convert(const Xml::Node& n,
                                                               AssetTurret::Definition::Component& v) {
    n.child("model").convert(v.model);
    n.child("offset").convert(v.offset);
}

void Xml::Adaptor<AssetTurret::Definition::Components>::convert(const Xml::Node& n,
                                                                AssetTurret::Definition::Components& v) {
    n.child("base").convert(v.base);
    n.child("arm").convert(v.arm);
    n.child("cannon").convert(v.cannon);
}

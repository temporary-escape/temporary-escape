#include "AssetParticles.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetParticles::AssetParticles(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetParticles::load(AssetManager& assetManager) {
    try {
        Xml::Document(path).getRoot().convert(definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load particles: '{}'", getName());
    }
}

std::shared_ptr<AssetParticles> AssetParticles::from(const std::string& name) {
    return AssetManager::singleton().find<AssetParticles>(name);
}

void Xml::Adaptor<AssetParticlesPtr>::convert(const Xml::Node& n, AssetParticlesPtr& v) {
    v = AssetParticles::from(n.asString());
}

void Xml::Adaptor<AssetParticles::Definition>::convert(const Xml::Node& n, AssetParticles::Definition& v) {
    n.child("texture").convert(v.texture);
    n.child("color").child("start").convert(v.startColor);
    n.child("color").child("end").convert(v.endColor);
    n.child("size").child("start").convert(v.startSize);
    n.child("size").child("end").convert(v.endSize);
    n.child("radius").child("start").convert(v.startRadius);
    n.child("radius").child("end").convert(v.endRadius);
    n.child("force").convert(v.force);
    n.child("duration").convert(v.duration);
    n.child("count").convert(v.count);
}

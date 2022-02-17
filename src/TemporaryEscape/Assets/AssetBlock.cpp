#include "AssetBlock.hpp"
#include "AssetManager.hpp"

using namespace Engine;

AssetBlock::AssetBlock(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetBlock::load(AssetManager& assetManager) {
    try {
        Xml::Document(path).getRoot().convert(definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load block: '{}'", getName());
    }
}

std::shared_ptr<AssetBlock> AssetBlock::from(const std::string& name) {
    return AssetManager::singleton().find<AssetBlock>(name);
}

void Xml::Adaptor<AssetBlock::Definition>::convert(const Xml::Node& n, AssetBlock::Definition& v) {
    n.child("title").convert(v.title);
    n.child("model").convert(v.model);
    if (n.hasChild("particles")) {
        v.particles = AssetBlock::Definition::Particles{};
        n.child("particles").convert(v.particles.value());
    }
}

void Xml::Adaptor<AssetBlock::Definition::Particles>::convert(const Xml::Node& n,
                                                              AssetBlock::Definition::Particles& v) {
    n.child("asset").convert(v.asset);
    n.child("offset").convert(v.offset);
}

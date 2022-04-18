#include "AssetBlock.hpp"
#include "AssetManager.hpp"

#define CMP "AssetBlock"

using namespace Engine;

AssetBlock::AssetBlock(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetBlock::load(AssetManager& assetManager) {
    try {
        definition.fromYaml(path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load block: '{}'", getName());
    }

    for (auto& ptr : shapeSideToMaterial) {
        ptr = nullptr;
    }

    if (definition.shapes.empty()) {
        EXCEPTION_NESTED("Block must allow at least one type of shape, the property 'shapes: []' can not be empty");
    }

    std::unordered_map<const Definition::Material*, size_t> map;

    for (const auto& def : definition.materials) {
        materials.emplace_back();
        auto& material = materials.back();

        material.baseColorTexture = def.baseColorTexture;
        material.uniform.baseColorFactor = def.baseColorFactor.has_value() ? def.baseColorFactor.value() : Color4{1.0f};
        material.uniform.emissiveFactor = def.emissiveFactor.has_value() ? def.emissiveFactor.value() : Color4{1.0f};
        material.uniform.metallicRoughnessFactor =
            def.metallicRoughnessFactor.has_value() ? def.metallicRoughnessFactor.value() : Color4{1.0f};
        material.metallicRoughnessTexture =
            def.metallicRoughnessTexture.has_value() ? def.metallicRoughnessTexture.value() : nullptr;
        material.emissiveTexture = def.emissiveTexture.has_value() ? def.emissiveTexture.value() : nullptr;
        material.normalTexture = def.normalTexture.has_value() ? def.normalTexture.value() : nullptr;
        material.ambientOcclusionTexture =
            def.ambientOcclusionTexture.has_value() ? def.ambientOcclusionTexture.value() : nullptr;

        map.insert(std::make_pair(&def, materials.size() - 1));
    }

    for (const auto& def : definition.materials) {
        for (const auto& side : def.faces) {
            shapeSideToMaterial[side] = &materials.at(map[&def]);
        }
    }
}

std::shared_ptr<AssetBlock> AssetBlock::from(const std::string& name) {
    return AssetManager::singleton().find<AssetBlock>(name);
}

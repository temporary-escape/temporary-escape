#include "AssetBlock.hpp"
#include "AssetManager.hpp"

#define CMP "AssetBlock"

using namespace Engine;

AssetBlock::AssetBlock(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetBlock::load(AssetManager& assetManager, bool noGraphics) {
    (void)noGraphics;

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

        material.baseColorTexture = def.baseColor.texture;
        material.uniform.baseColorFactor = def.baseColor.factor.has_value() ? *def.baseColor.factor : Color4{1.0f};
        material.uniform.emissiveFactor = def.emissive && def.emissive->factor ? *def.emissive->factor : Color4{1.0f};
        material.uniform.metallicRoughnessFactor =
            def.metallicRoughness && def.metallicRoughness->factor ? *def.metallicRoughness->factor : Color4{1.0f};
        material.metallicRoughnessTexture = def.metallicRoughness ? def.metallicRoughness->texture : nullptr;
        material.emissiveTexture = def.emissive ? def.emissive->texture : nullptr;
        material.normalTexture = def.normal ? def.normal->texture : nullptr;
        material.ambientOcclusionTexture = def.ambientOcclusion ? def.ambientOcclusion->texture : nullptr;

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

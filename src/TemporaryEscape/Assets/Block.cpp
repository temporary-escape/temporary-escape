#include "Block.hpp"
#include "Registry.hpp"

#define CMP "Block"

using namespace Engine;

Block::Block(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
    try {
        definition.fromYaml(this->path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load block: '{}'", getName());
    }

    for (auto& ptr : shapeSideToMaterial) {
        ptr = nullptr;
    }

    if (definition.shapes.empty()) {
        EXCEPTION_NESTED("Block must allow at least one type of shape, the property 'shapes: []' can not be empty");
    }
}

void Block::load(Registry& registry, VulkanDevice& vulkan) {
    auto& defaults = registry.getDefaultTextures();

    std::unordered_map<const Definition::MaterialDefinition*, size_t> map;

    for (const auto& def : definition.materials) {
        materials.emplace_back();
        auto& material = materials.back();

        material.baseColorTexture = def.baseColor ? def.baseColor->texture : defaults.baseColor;
        material.uniform.baseColorFactor = def.baseColor->factor.has_value() ? *def.baseColor->factor : Color4{1.0f};

        material.uniform.metallicRoughnessFactor =
            def.metallicRoughness && def.metallicRoughness->factor ? *def.metallicRoughness->factor : Color4{1.0f};
        material.metallicRoughnessTexture =
            def.metallicRoughness ? def.metallicRoughness->texture : defaults.metallicRoughness;

        material.emissiveTexture = def.emissive ? def.emissive->texture : defaults.emissive;
        material.uniform.emissiveFactor = def.emissive && def.emissive->factor ? *def.emissive->factor : Color4{1.0f};

        material.normalTexture = def.normal ? def.normal->texture : defaults.normal;
        material.uniform.normalFactor = Color4{1.0f};

        material.ambientOcclusionTexture = def.ambientOcclusion ? def.ambientOcclusion->texture : defaults.ambient;
        material.uniform.ambientOcclusionFactor = Color4{1.0f};

        map.insert(std::make_pair(&def, materials.size() - 1));
    }

    for (const auto& def : definition.materials) {
        for (const auto& side : def.faces) {
            shapeSideToMaterial[side] = &materials.at(map[&def]);
        }
    }

    for (auto& material : materials) {
        material.ubo =
            vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(Material::Uniform));
        material.ubo.subData(&material.uniform, 0, sizeof(Material::Uniform));
    }
}

BlockPtr Block::from(const std::string& name) {
    return Registry::getInstance().getBlocks().find(name);
}

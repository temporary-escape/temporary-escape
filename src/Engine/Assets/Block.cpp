#include "Block.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Block::Block(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Block::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)audio;

    try {
        Xml::fromFile(this->path, definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load block: '{}'", getName());
    }

    if (definition.shapes.empty()) {
        EXCEPTION_NESTED("Block must allow at least one type of shape, the property 'shapes: []' can not be empty");
    }

    // Do not load unless Vulkan is present (client mode)
    if (!vulkan) {
        return;
    }

    auto& defaults = assetsManager.getDefaultTextures();

    materials.reserve(definition.materials.size());

    for (const auto& def : definition.materials) {
        if (def.baseColor) {
            def.baseColor->texture->setUsage(TextureUsage::Diffuse);
        }
        if (def.metallicRoughness) {
            def.metallicRoughness->texture->setUsage(TextureUsage::MetallicRoughness);
        }
        if (def.normal) {
            def.normal->texture->setUsage(TextureUsage::Normal);
        }
        if (def.emissive) {
            def.emissive->texture->setUsage(TextureUsage::Emissive);
        }
        if (def.ambientOcclusion) {
            def.ambientOcclusion->texture->setUsage(TextureUsage::MetallicRoughness);
        }
        if (def.mask) {
            def.mask->texture->setUsage(TextureUsage::Mask);
        }

        // const auto [index, material] = assetsManager.addBlockMaterial();
        auto& material = materials.emplace_back();

        material.baseColorTexture = def.baseColor
                                        ? def.baseColor->texture
                                        : assetsManager.getTextures().find(AssetsManager::defaultTextureDiffName);
        material.baseColorFactor = def.baseColor->factor.has_value() ? *def.baseColor->factor : Color4{1.0f};

        material.metallicRoughnessTexture =
            def.metallicRoughness ? def.metallicRoughness->texture
                                  : assetsManager.getTextures().find(AssetsManager::defaultTextureMetaName);
        material.metallicRoughnessFactor =
            def.metallicRoughness && def.metallicRoughness->factor ? *def.metallicRoughness->factor : Color4{1.0f};

        material.emissiveTexture = def.emissive
                                       ? def.emissive->texture
                                       : assetsManager.getTextures().find(AssetsManager::defaultTextureEmisName);
        material.emissiveFactor = def.emissive && def.emissive->factor ? *def.emissive->factor : Color4{1.0f};

        material.normalTexture =
            def.normal ? def.normal->texture : assetsManager.getTextures().find(AssetsManager::defaultTextureNormName);
        material.normalFactor = Color4{1.0f};

        material.ambientOcclusionTexture = def.ambientOcclusion
                                               ? def.ambientOcclusion->texture
                                               : assetsManager.getTextures().find(AssetsManager::defaultTextureAoName);
        material.ambientOcclusionFactor = Color4{1.0f};

        material.maskTexture =
            def.mask ? def.mask->texture : assetsManager.getTextures().find(AssetsManager::defaultTextureMaskName);

        for (const auto& side : def.faces) {
            shapeSideToMaterial[side] = static_cast<int>(materials.size()) - 1;
        }
    }

    singular = definition.materials.size() == 1;
}

void Block::allocateMaterials(AssetsManager& assetsManager) {
    for (int i = 0; i < materials.size(); i++) {
        const auto& material = materials[i];

        const auto [index, uniform] = assetsManager.addBlockMaterial();

        uniform->baseColorFactor = material.baseColorFactor;
        uniform->emissiveFactor = material.emissiveFactor;
        uniform->normalFactor = material.normalFactor;
        uniform->metallicRoughnessFactor = material.metallicRoughnessFactor;
        uniform->ambientOcclusionFactor = material.ambientOcclusionFactor;

        uniform->baseColorTexture = material.baseColorTexture->getLayer();
        uniform->emissiveTexture = material.emissiveTexture->getLayer();
        uniform->normalTexture = material.normalTexture->getLayer();
        uniform->metallicRoughnessTexture = material.metallicRoughnessTexture->getLayer();
        uniform->ambientOcclusionTexture = material.ambientOcclusionTexture->getLayer();
        uniform->maskTexture = material.maskTexture->getLayer();

        for (auto& m : shapeSideToMaterial) {
            if (m == i) {
                m = static_cast<int>(index);
            }
        }
    }
}

BlockPtr Block::from(const std::string& name) {
    return AssetsManager::getInstance().getBlocks().find(name);
}

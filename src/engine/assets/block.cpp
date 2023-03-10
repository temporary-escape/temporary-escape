#include "block.hpp"
#include "registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

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

void Block::load(Registry& registry, VulkanRenderer& vulkan) {
    auto& defaults = registry.getDefaultTextures();

    std::unordered_map<const Definition::MaterialDefinition*, size_t> map;

    for (const auto& def : definition.materials) {
        materials.emplace_back(std::make_unique<Material>());
        auto& material = materials.back();

        material->baseColorTexture = def.baseColor ? def.baseColor->texture : defaults.baseColor;
        material->uniform.baseColorFactor = def.baseColor->factor.has_value() ? *def.baseColor->factor : Color4{1.0f};

        material->uniform.metallicRoughnessFactor =
            def.metallicRoughness && def.metallicRoughness->factor ? *def.metallicRoughness->factor : Color4{1.0f};
        material->metallicRoughnessTexture =
            def.metallicRoughness ? def.metallicRoughness->texture : defaults.metallicRoughness;

        material->emissiveTexture = def.emissive ? def.emissive->texture : defaults.emissive;
        material->uniform.emissiveFactor = def.emissive && def.emissive->factor ? *def.emissive->factor : Color4{1.0f};

        material->normalTexture = def.normal ? def.normal->texture : defaults.normal;
        material->uniform.normalFactor = Color4{1.0f};

        material->ambientOcclusionTexture = def.ambientOcclusion ? def.ambientOcclusion->texture : defaults.ambient;
        material->uniform.ambientOcclusionFactor = Color4{1.0f};

        map.insert(std::make_pair(&def, materials.size() - 1));
    }

    for (const auto& def : definition.materials) {
        for (const auto& side : def.faces) {
            shapeSideToMaterial[side] = materials.at(map[&def]).get();
        }
    }

    for (auto& material : materials) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Material::Uniform);
        bufferInfo.usage =
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        material->ubo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(material->ubo, &material->uniform, sizeof(Material::Uniform));
    }
}

BlockPtr Block::from(const std::string& name) {
    return Registry::getInstance().getBlocks().find(name);
}

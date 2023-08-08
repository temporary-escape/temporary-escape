#include "component_grid.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentGrid::ComponentGrid(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

void ComponentGrid::update() {
}

void ComponentGrid::debugIterate(Grid::Iterator iterator) {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            // logger.debug("Add box: {}", pos);
            debug->addBox(glm::translate(pos), 0.95f, Color4{1.0f, 0.0f, 0.0f, 1.0f});
        } else {
            debugIterate(iterator.children());
        }

        iterator.next();
    }
}

void ComponentGrid::recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    if (debug) {
        debug->clear();
        auto iterator = iterate();
        debugIterate(iterator);
    }

    Grid::RawPrimitiveData map;
    generateMesh(voxelShapeCache, map);

    for (auto& primitive : primitives) {
        vulkan.dispose(std::move(primitive.mesh.ibo));
        vulkan.dispose(std::move(primitive.mesh.vbo));
    }
    primitives.clear();

    for (const auto& [material, data] : map) {
        logger.debug("Building mesh for type: {} of size: {} indices",
                     material->baseColorTexture->getName(),
                     data.indices.size());

        if (data.indices.empty()) {
            continue;
        }

        primitives.emplace_back();
        auto& primitive = primitives.back();

        primitive.material = material;

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = data.vertices.size() * sizeof(VoxelShape::VertexFinal);
        bufferInfo.usage =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        primitive.mesh.vbo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.mesh.vbo, data.vertices.data(), bufferInfo.size);

        bufferInfo.size = data.indices.size() * sizeof(uint32_t);
        bufferInfo.usage =
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        primitive.mesh.ibo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.mesh.ibo, data.indices.data(), bufferInfo.size);

        primitive.mesh.count = data.indices.size();
        primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
    }
}

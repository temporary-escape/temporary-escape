#include "component_grid.hpp"

#define CMP "componentGrid"

using namespace Engine;

void ComponentGrid::update() {
}

void ComponentGrid::debugIterate(Grid::Iterator iterator) {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            // Log::d(CMP, "Add box: {}", pos);
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

    primitives.clear();

    for (const auto& [material, data] : map) {
        Log::d(CMP, "Building mesh for type: {} of size: {} indices", material->baseColorTexture->getName(),
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
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        primitive.vbo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.vbo, data.vertices.data(), bufferInfo.size);

        bufferInfo.size = data.indices.size() * sizeof(uint32_t);
        bufferInfo.usage =
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        primitive.ibo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.ibo, data.indices.data(), bufferInfo.size);

        primitive.count = data.indices.size();
        primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
    }
}

/*void ComponentGrid::render(VulkanRenderer& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    if (primitives.empty()) {
        return;
    }

    const Matrix4 transform = getObject().getAbsoluteTransform();
    const Matrix3 transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    vulkan.pushConstant(0, transform);
    vulkan.pushConstant(sizeof(Matrix4), transformInverted);

    for (const auto& primitive : primitives) {
        vulkan.bindVertexBuffer(primitive.vbo, 0);
        vulkan.bindVertexInputFormat(primitive.vboFormat);
        vulkan.bindIndexBuffer(primitive.ibo, 0, VK_INDEX_TYPE_UINT32);
        vulkan.bindUniformBuffer(primitive.material->ubo, 1);
        vulkan.bindTexture(primitive.material->baseColorTexture->getVulkanTexture(), 2);
        vulkan.bindTexture(primitive.material->emissiveTexture->getVulkanTexture(), 3);
        vulkan.bindTexture(primitive.material->normalTexture->getVulkanTexture(), 4);
        vulkan.bindTexture(primitive.material->ambientOcclusionTexture->getVulkanTexture(), 5);
        vulkan.bindTexture(primitive.material->metallicRoughnessTexture->getVulkanTexture(), 6);
        vulkan.setInputAssembly(primitive.topology);
        vulkan.drawIndexed(primitive.count, 1, 0, 0, 0);
    }
}*/

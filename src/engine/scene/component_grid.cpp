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
    /*if (!isDirty()) {
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

        const auto vboSize = data.vertices.size() * sizeof(VoxelShape::VertexFinal);
        primitive.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic, vboSize);
        primitive.vbo.subData(data.vertices.data(), 0, vboSize);

        const auto iboSize = data.indices.size() * sizeof(uint32_t);
        primitive.ibo = vulkan.createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Dynamic, iboSize);
        primitive.ibo.subData(data.indices.data(), 0, iboSize);

        primitive.vboFormat = vulkan.createVertexInputFormat({
            {
                0,
                {
                    {0, 0, VulkanVertexInputFormat::Format::Vec3},
                    {1, 0, VulkanVertexInputFormat::Format::Vec3},
                    {2, 0, VulkanVertexInputFormat::Format::Vec2},
                    {3, 0, VulkanVertexInputFormat::Format::Vec4},
                },
            },
        });

        primitive.count = data.indices.size();
        primitive.indexType = VkFormat::VK_FORMAT_R32_UINT;
        primitive.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }*/
}

void ComponentGrid::render(VulkanRenderer& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    /*if (primitives.empty()) {
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
    }*/
}

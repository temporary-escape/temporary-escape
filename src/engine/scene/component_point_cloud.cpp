#include "component_point_cloud.hpp"

#define CMP "ComponentPointCloud"

using namespace Engine;

void ComponentPointCloud::recalculate(VulkanDevice& vulkan) {
    if (!isDirty()) {
        return;
    }

    Log::d(CMP, "Recreating with {} points", points.size());

    vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Static, sizeof(Point) * points.size());
    vbo.subData(points.data(), 0, sizeof(Point) * points.size());

    vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
                {1, 0, VulkanVertexInputFormat::Format::Vec2},
                {2, 0, VulkanVertexInputFormat::Format::Vec4},
            },
        },
    });

    count = points.size();
    points.clear();
    points.shrink_to_fit();

    setDirty(false);
}

void ComponentPointCloud::render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    recalculate(vulkan);

    if (!count) {
        return;
    }

    const Matrix4 transform = getObject().getAbsoluteTransform();
    vulkan.pushConstant(0, transform);

    vulkan.bindVertexBuffer(vbo, 0);
    vulkan.bindVertexInputFormat(vboFormat);
    vulkan.bindTexture(texture->getVulkanTexture(), 1);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    vulkan.draw(count, 1, 0, 0);
}

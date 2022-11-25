#include "component_lines.hpp"

using namespace Engine;

void ComponentLines::recalculate(VulkanDevice& vulkan) {
    if (!isDirty()) {
        return;
    }

    vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Static, sizeof(Line) * lines.size());
    vbo.subData(lines.data(), 0, sizeof(Line) * lines.size());

    vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
                {1, 0, VulkanVertexInputFormat::Format::Vec4},
            },
        },
    });

    setDirty(false);
}

void ComponentLines::render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    if (lines.empty()) {
        return;
    }

    recalculate(vulkan);

    const Matrix4 transform = getObject().getAbsoluteTransform();
    vulkan.pushConstant(0, transform);

    vulkan.bindVertexBuffer(vbo, 0);
    vulkan.bindVertexInputFormat(vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    vulkan.draw(lines.size() * 2, 1, 0, 0);
}

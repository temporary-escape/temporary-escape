#include "component_lines.hpp"

#define CMP "ComponentLines"

using namespace Engine;

void ComponentLines::recalculate(VulkanRenderer& vulkan) {
    /*if (!isDirty()) {
        return;
    }

    Log::d(CMP, "Recreating with {} lines", lines.size());

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

    count = lines.size() * 2;
    lines.clear();
    lines.shrink_to_fit();

    setDirty(false);*/
}

void ComponentLines::render(VulkanRenderer& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    /*recalculate(vulkan);

    if (!count) {
        return;
    }

    const Matrix4 transform = getObject().getAbsoluteTransform();
    vulkan.pushConstant(0, transform);

    vulkan.bindVertexBuffer(vbo, 0);
    vulkan.bindVertexInputFormat(vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vulkan.draw(count, 1, 0, 0);*/
}

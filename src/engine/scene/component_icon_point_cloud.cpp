#include "component_icon_point_cloud.hpp"
#include "../utils/log.hpp"

#define CMP "ComponentIconPointCloud"

using namespace Engine;

void ComponentIconPointCloud::add(const Vector3& pos, const Vector2& size, const Color4& color, const ImagePtr& image) {
    setDirty(true);
    imagePoints[image].push_back({pos, size, color, image->getAllocation().uv, image->getAllocation().st});
}

void ComponentIconPointCloud::recalculate(VulkanRenderer& vulkan) {
    /*if (!isDirty()) {
        return;
    }

    Log::d(CMP, "Recreating with {} images", imagePoints.size());

    for (const auto& [image, points] : imagePoints) {
        Log::d(CMP, "Recreating {} points with image: {}", points.size(), image->getName());

        vbos[image] =
            vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Static, sizeof(Point) * points.size());
        vbos[image].subData(points.data(), 0, sizeof(Point) * points.size());
    }

    vboFormat = vulkan.createVertexInputFormat({
        {
            0,
            {
                {0, 0, VulkanVertexInputFormat::Format::Vec3},
                {1, 0, VulkanVertexInputFormat::Format::Vec2},
                {2, 0, VulkanVertexInputFormat::Format::Vec4},
                {3, 0, VulkanVertexInputFormat::Format::Vec2},
                {4, 0, VulkanVertexInputFormat::Format::Vec2},
            },
        },
    });

    imagePoints.clear();

    setDirty(false);*/
}

/*void ComponentIconPointCloud::render(VulkanRenderer& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    recalculate(vulkan);

    if (vbos.empty()) {
        return;
    }

    const Matrix4 transform = getObject().getAbsoluteTransform();
    vulkan.pushConstant(0, transform);

    vulkan.bindVertexInputFormat(vboFormat);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

    for (const auto& [image, vbo] : vbos) {
        vulkan.bindVertexBuffer(vbo, 0);
        vulkan.bindTexture(*image->getAllocation().texture, 1);
        vulkan.draw(vbo.getSize() / sizeof(Point), 1, 0, 0);
    }
}*/

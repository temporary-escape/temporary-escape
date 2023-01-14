#include "component_point_cloud.hpp"

#define CMP "ComponentPointCloud"

using namespace Engine;

void ComponentPointCloud::add(const Vector3& pos, const Vector2& size, const Color4& color) {
    setDirty(true);
    points.push_back({pos, size, color, {0.0f, 0.0f}, {1.0f, 1.0f}});
}

void ComponentPointCloud::clear() {
    setDirty(true);
    points.clear();
    points.shrink_to_fit();
}

void ComponentPointCloud::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    Log::d(CMP, "Recreating {} points with image: {}", points.size(), texture->getName());

    vulkan.dispose(std::move(mesh.vbo));

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Point) * points.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, points.data(), sizeof(Point) * points.size());
    mesh.count = points.size();
}

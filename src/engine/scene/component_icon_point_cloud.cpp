#include "component_icon_point_cloud.hpp"
#include "../utils/log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void ComponentIconPointCloud::add(const Vector3& pos, const Vector2& size, const Color4& color, const ImagePtr& image) {
    setDirty(true);
    imagePoints[image].push_back({pos, size, color, image->getAllocation().uv, image->getAllocation().st});
}

void ComponentIconPointCloud::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    logger.debug("Recreating with {} images", imagePoints.size());

    for (const auto& [image, points] : imagePoints) {
        logger.debug("Recreating {} points with image: {}", points.size(), image->getName());

        vulkan.dispose(std::move(meshes[image].vbo));

        if (points.empty()) {
            continue;
        }

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Point) * points.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

        meshes[image].vbo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(meshes[image].vbo, points.data(), sizeof(Point) * points.size());
        meshes[image].instances = points.size();
        meshes[image].count = 4;
    }
}

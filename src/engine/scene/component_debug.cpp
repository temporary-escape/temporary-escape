#include "component_debug.hpp"
#include "component_camera.hpp"
#include "component_transform.hpp"

using namespace Engine;

void ComponentDebug::recalculate(VulkanRenderer& vulkan) {
    if (vertices.empty() || !isDirty()) {
        return;
    }

    setDirty(false);

    const auto vboSize = vertices.capacity() * sizeof(Vertex);

    if (!mesh.vbo || mesh.vbo.getSize() != vboSize) {
        vulkan.dispose(std::move(mesh.vbo));

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = vboSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

        mesh.vbo = vulkan.createBuffer(bufferInfo);
    }

    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), vboSize);

    const auto iboSize = indices.capacity() * sizeof(uint32_t);

    if (!mesh.ibo || mesh.ibo.getSize() != iboSize) {
        vulkan.dispose(std::move(mesh.ibo));

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = vboSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

        mesh.ibo = vulkan.createBuffer(bufferInfo);
    }

    vulkan.copyDataToBuffer(mesh.ibo, indices.data(), vboSize);

    mesh.count = indices.size();
    mesh.indexType = VK_INDEX_TYPE_UINT32;
}

void ComponentDebug::clear() {
    vertices.clear();
    indices.clear();
    setDirty(true);
}

void ComponentDebug::addBox(const Matrix4& transform, float width, const Color4& color) {
    const auto half = width / 2.0f;
    const auto start = vertices.size();
    vertices.reserve(vertices.size() + (1024 - vertices.size() % 1024));
    indices.reserve(indices.size() + (1024 - indices.size() % 1024));

    // Bottom
    vertices.push_back({Vector3{transform * Vector4{-half, -half, -half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{-half, -half, half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{half, -half, half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{half, -half, -half, 1.0f}}, color});

    // Top
    vertices.push_back({Vector3{transform * Vector4{-half, half, -half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{-half, half, half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{half, half, half, 1.0f}}, color});
    vertices.push_back({Vector3{transform * Vector4{half, half, -half, 1.0f}}, color});

    // Bottom
    indices.push_back(start);
    indices.push_back(start + 1);

    indices.push_back(start + 1);
    indices.push_back(start + 2);

    indices.push_back(start + 2);
    indices.push_back(start + 3);

    indices.push_back(start + 3);
    indices.push_back(start);

    // Top
    indices.push_back(start + 4);
    indices.push_back(start + 5);

    indices.push_back(start + 5);
    indices.push_back(start + 6);

    indices.push_back(start + 6);
    indices.push_back(start + 7);

    indices.push_back(start + 7);
    indices.push_back(start + 4);

    // Sides
    indices.push_back(start);
    indices.push_back(start + 4);

    indices.push_back(start + 1);
    indices.push_back(start + 5);

    indices.push_back(start + 2);
    indices.push_back(start + 6);

    indices.push_back(start + 3);
    indices.push_back(start + 7);

    setDirty(true);
}

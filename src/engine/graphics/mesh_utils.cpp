#include "mesh_utils.hpp"

using namespace Engine;

Mesh Engine::createFullScreenQuad(VulkanRenderer& vulkan) {
    static const std::vector<FullScreenVertex> vertices = {
        {{-1.0f, -1.0f}},
        {{1.0f, -1.0f}},
        {{1.0f, 1.0f}},
        {{-1.0f, 1.0f}},
    };

    static const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
    };

    Mesh mesh;

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(FullScreenVertex) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), sizeof(FullScreenVertex) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.ibo, indices.data(), sizeof(uint16_t) * indices.size());

    mesh.indexType = VK_INDEX_TYPE_UINT16;
    mesh.count = indices.size();

    return mesh;
}

Mesh Engine::createSkyboxCube(VulkanRenderer& vulkan) {
    static const std::vector<uint16_t> indices = {
        0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 21, 23,
    };

    static const std::vector<Vector3> vertices = {
        {-1.0f, 1.0f, -1.0f},  // 0
        {-1.0f, -1.0f, -1.0f}, // 1
        {1.0f, -1.0f, -1.0f},  // 2
        {1.0f, 1.0f, -1.0f},   // 3

        {-1.0f, -1.0f, 1.0f},  // 4
        {-1.0f, -1.0f, -1.0f}, // 5
        {-1.0f, 1.0f, -1.0f},  // 6
        {-1.0f, 1.0f, 1.0f},   // 7

        {1.0f, -1.0f, -1.0f}, // 8
        {1.0f, -1.0f, 1.0f},  // 9
        {1.0f, 1.0f, 1.0f},   // 10
        {1.0f, 1.0f, -1.0f},  // 11

        {-1.0f, -1.0f, 1.0f}, // 12
        {-1.0f, 1.0f, 1.0f},  // 13
        {1.0f, 1.0f, 1.0f},   // 14
        {1.0f, -1.0f, 1.0f},  // 15

        {-1.0f, 1.0f, -1.0f}, // 16
        {1.0f, 1.0f, -1.0f},  // 17
        {1.0f, 1.0f, 1.0f},   // 18
        {-1.0f, 1.0f, 1.0f},  // 19

        {-1.0f, -1.0f, -1.0f}, // 20
        {-1.0f, -1.0f, 1.0f},  // 21
        {1.0f, -1.0f, -1.0f},  // 22
        {1.0f, -1.0f, 1.0f},   // 23
    };

    Mesh mesh{};

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vector3) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), sizeof(Vector3) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.ibo, indices.data(), sizeof(uint16_t) * indices.size());

    mesh.indexType = VK_INDEX_TYPE_UINT16;
    mesh.count = indices.size();

    return mesh;
}

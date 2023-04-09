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
        0,
        1,
        2,
        2,
        3,
        0,
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

Mesh Engine::createPlanetMesh(VulkanRenderer& vulkan) {
    static const auto pi = 3.14159265358979323846f;
    static const auto pi2 = 1.57079632679489661923f;

    using Vertex = PlanetVertex;

    // Source: https://stackoverflow.com/a/5989676
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static const auto radius = 0.5f;
    static const auto rings = 32;
    static const auto sectors = 64;

    const auto R = 1.0f / static_cast<float>(rings - 1);
    const auto S = 1.0f / static_cast<float>(sectors - 1);

    vertices.resize(rings * sectors);
    indices.resize(rings * sectors * 6);

    auto v = vertices.begin();
    for (auto r = 0; r < rings; r++)
        for (auto s = 0; s < sectors; s++) {
            float const y = std::sin(-pi2 + pi * r * R);
            float const x = std::cos(2 * pi * s * S) * std::sin(pi * r * R);
            float const z = std::sin(2 * pi * s * S) * std::sin(pi * r * R);

            v->position = Vector3{x, y, z} * radius;
            v->normal = {x, y, z};
            v->texCoords = Vector2{s * S, r * R};
            v++;
        }

    auto i = indices.begin();
    for (auto r = 0; r < rings; r++) {
        for (auto s = 0; s < sectors; s++) {
            *i++ = r * sectors + s;
            *i++ = r * sectors + (s + 1);
            *i++ = (r + 1) * sectors + (s + 1);

            *i++ = r * sectors + s;
            *i++ = (r + 1) * sectors + (s + 1);
            *i++ = (r + 1) * sectors + s;
        }
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        auto& v0 = vertices[indices[i + 0]];
        auto& v1 = vertices[indices[i + 1]];
        auto& v2 = vertices[indices[i + 2]];

        const auto deltaPos1 = v1.position - v0.position;
        const auto deltaPos2 = v2.position - v0.position;

        const auto deltaUv1 = v1.texCoords - v0.texCoords;
        const auto deltaUv2 = v2.texCoords - v0.texCoords;

        float r = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv1.y * deltaUv2.x);
        auto tangent = Vector4{Vector3{deltaPos1 * deltaUv2.y - deltaPos2 * deltaUv1.y} * r, 0.0f};
        tangent = glm::normalize(tangent);

        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;
    }

    Mesh mesh{};

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), bufferInfo.size);

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    mesh.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.ibo, indices.data(), bufferInfo.size);

    mesh.indexType = VK_INDEX_TYPE_UINT16;
    mesh.count = indices.size();

    return mesh;
}

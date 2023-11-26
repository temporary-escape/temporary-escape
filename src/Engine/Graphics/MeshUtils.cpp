#include "MeshUtils.hpp"
#include "../Utils/Exceptions.hpp"

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
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), sizeof(FullScreenVertex) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

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
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), sizeof(Vector3) * vertices.size());

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

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

    // Sphere generation code source: http://www.songho.ca/opengl/gl_sphere.html

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    const float radius = 0.5f;
    const int stackCount = 32;
    const int sectorCount = 64;

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
    float s, t;                                  // vertex texCoord

    float sectorStep = 2 * pi / sectorCount;
    float stackStep = pi / stackCount;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = pi / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);            // r * cos(u)
        z = radius * sinf(stackAngle);             // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep; // starting from 0 to 2pi

            vertices.emplace_back();
            auto& v = vertices.back();

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            v.position = Vector3{x, y, z};

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            v.normal = Vector3{nx, ny, nz};

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            v.texCoords = Vector2{s, t};
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    for (int i = 0; i < stackCount; ++i) {
        auto k1 = i * (sectorCount + 1); // beginning of current stack
        auto k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding 1st and last stacks
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        auto& v0 = vertices.at(indices[i + 0]);
        auto& v1 = vertices.at(indices[i + 1]);
        auto& v2 = vertices.at(indices[i + 2]);

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
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), bufferInfo.size);

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint16_t) * indices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.ibo, indices.data(), bufferInfo.size);

    mesh.indexType = VK_INDEX_TYPE_UINT16;
    mesh.count = indices.size();

    return mesh;
}

Mesh Engine::createBulletMesh(VulkanRenderer& vulkan) {
    static const std::vector<Vector3> objVertex = {
        {0.000000, 0.000000, 0.500000},
        {0.000000, 0.018639, 0.488768},
        {0.013180, 0.013180, 0.488768},
        {0.018639, 0.000000, 0.488768},
        {0.013180, -0.013180, 0.488768},
        {0.000000, -0.018639, 0.488768},
        {-0.013180, -0.013180, 0.488768},
        {-0.018639, 0.000000, 0.488768},
        {-0.013180, 0.013180, 0.488768},
        {-0.000000, -0.000000, -0.500000},
        {0.000000, 0.025000, 0.464022},
        {0.017678, 0.017678, 0.464022},
        {0.025000, 0.000000, 0.464022},
        {0.017678, -0.017678, 0.464022},
        {0.000000, -0.025000, 0.464022},
        {-0.017678, -0.017678, 0.464022},
        {-0.025000, 0.000000, 0.464022},
        {-0.017678, 0.017678, 0.464022},
    };

    static const std::vector<uint8_t> objIndex = {
        1,  3,  2,  2,  12, 11, 1,  4,  3,  12, 4,  13, 1,  5,  4,  4,  14, 13, 1,  6,  5,  14, 6,  15,
        1,  7,  6,  6,  16, 15, 1,  8,  7,  16, 8,  17, 1,  9,  8,  8,  18, 17, 1,  2,  9,  18, 2,  11,
        2,  3,  12, 12, 3,  4,  4,  5,  14, 14, 5,  6,  6,  7,  16, 16, 7,  8,  8,  9,  18, 18, 9,  2,
        18, 11, 10, 17, 18, 10, 16, 17, 10, 15, 16, 10, 14, 15, 10, 13, 14, 10, 12, 13, 10, 11, 12, 10,
    };

    std::vector<BulletVertex> vertices;
    vertices.resize(objIndex.size());

    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].position = objVertex[objIndex[i] - 1];
    }

    Mesh mesh{};

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(BulletVertex) * vertices.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), bufferInfo.size);

    mesh.count = vertices.size();

    return mesh;
}

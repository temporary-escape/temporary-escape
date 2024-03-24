#pragma once

#include "../Vulkan/VulkanRenderer.hpp"
#include "Mesh.hpp"

namespace Engine {
struct ENGINE_API FullScreenVertex {
    Vector2 pos;

    static VulkanVertexLayoutMap getLayout() {
        return {
            {0, VK_FORMAT_R32G32_SFLOAT, offsetof(FullScreenVertex, pos)},
        };
    };
};

struct ENGINE_API SkyboxVertex {
    Vector3 pos;

    static VulkanVertexLayoutMap getLayout() {
        return {
            {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SkyboxVertex, pos)},
        };
    };
};

struct ENGINE_API PlanetVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
    Vector4 tangent;

    static VulkanVertexLayoutMap getLayout() {
        return {
            {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PlanetVertex, position)},
            {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PlanetVertex, normal)},
            {2, VK_FORMAT_R32G32_SFLOAT, offsetof(PlanetVertex, texCoords)},
            {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(PlanetVertex, tangent)},
        };
    };
};

struct ENGINE_API BulletVertex {
    Vector3 position;

    static VulkanVertexLayoutMap getLayout() {
        return {
            {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PlanetVertex, position)},
        };
    };
};

struct ENGINE_API SpaceDustVertex {
    Vector3 position;

    static VulkanVertexLayoutMap getLayout() {
        return {
            {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PlanetVertex, position)},
        };
    };
};

Mesh ENGINE_API createFullScreenQuad(VulkanRenderer& vulkan);
Mesh ENGINE_API createSkyboxCube(VulkanRenderer& vulkan);
Mesh ENGINE_API createPlanetMesh(VulkanRenderer& vulkan);
Mesh ENGINE_API createBulletMesh(VulkanRenderer& vulkan);
Mesh ENGINE_API createCircleMesh(VulkanRenderer& vulkan);
Mesh ENGINE_API createLineForwardMesh(VulkanRenderer& vulkan);
Mesh ENGINE_API createSpaceDustMesh(VulkanRenderer& vulkan);
Mesh ENGINE_API createTacticalOverlayMesh(VulkanRenderer& vulkan);
} // namespace Engine

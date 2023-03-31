#pragma once

#include "../vulkan/vulkan_renderer.hpp"
#include "mesh.hpp"

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

Mesh ENGINE_API createFullScreenQuad(VulkanRenderer& vulkan);
Mesh ENGINE_API createSkyboxCube(VulkanRenderer& vulkan);
} // namespace Engine

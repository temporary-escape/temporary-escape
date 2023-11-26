#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Vulkan/VulkanBuffer.hpp"
#include "Material.hpp"

namespace Engine {
struct Primitive {
    Primitive() = default;
    Primitive(const Primitive& other) = delete;
    Primitive(Primitive&& other) = default;
    Primitive& operator=(const Primitive& other) = delete;
    Primitive& operator=(Primitive&& other) = default;

    Mesh mesh;
    const Material* material{nullptr};
};
} // namespace Engine

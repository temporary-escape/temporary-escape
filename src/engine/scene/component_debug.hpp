#pragma once

#include "../graphics/mesh.hpp"
#include "../graphics/shaders/shader_component_debug.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentTransform;
class ENGINE_API ComponentCamera;

class ENGINE_API ComponentDebug : public Component {
public:
    using Vertex = ShaderComponentDebug::Vertex;
    using Uniforms = ShaderComponentDebug::Uniforms;

    ComponentDebug() = default;
    virtual ~ComponentDebug() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentDebug);

    void recalculate(VulkanRenderer& vulkan);
    void clear();
    void addBox(const Matrix4& transform, float width, const Color4& color);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Mesh mesh;
};
} // namespace Engine

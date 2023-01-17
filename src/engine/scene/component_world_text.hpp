#pragma once

#include "../font/font_face.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/shaders/shader_component_world_text.hpp"
#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentWorldText : public Component {
public:
    using Vertex = ShaderComponentWorldText::Vertex;

    ComponentWorldText() = default;
    explicit ComponentWorldText(const FontFace& fontFace, const Color4& color, const float height) :
        fontFace{&fontFace}, color{color}, height{height} {
    }
    virtual ~ComponentWorldText() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentWorldText);

    void recalculate(VulkanRenderer& vulkan);

    void add(const Vector3& pos, const std::string& text);

    void setColor(const Color4& value) {
        color = value;
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

    [[nodiscard]] const FontFace& getFontFace() const {
        return *fontFace;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

private:
    const FontFace* fontFace{nullptr};
    Color4 color;
    float height;
    Mesh mesh;
    std::vector<Vertex> vertices;
    Vector2 offset;
};
} // namespace Engine

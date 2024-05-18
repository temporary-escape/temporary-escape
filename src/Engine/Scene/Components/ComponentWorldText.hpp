#pragma once

#include "../../Font/FontFamily.hpp"
#include "../../Graphics/Mesh.hpp"
#include "../../Graphics/WorldSpaceText.hpp"
#include "../../Library.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentWorldText : public Component {
public:
    using Vertex = WorldSpaceText::Vertex;

    ComponentWorldText() = default;
    explicit ComponentWorldText(EntityId entity, const FontFamily& font, const Color4& color, const float height);
    COMPONENT_DEFAULTS(ComponentWorldText);

    void recalculate(VulkanRenderer& vulkan);
    void reset();
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

    [[nodiscard]] const FontFamily& getFont() const {
        return *font;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

    void setTextAlign(TextAlign value) {
        textAlign = value;
    }

private:
    bool dirty{false};
    const FontFamily* font{nullptr};
    Color4 color;
    float height;
    Mesh mesh;
    std::vector<Vertex> vertices;
    Vector2 offset;
    TextAlign textAlign{TextAlign::Center};
};
} // namespace Engine

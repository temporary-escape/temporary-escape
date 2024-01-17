#pragma once

#include "../../Font/FontFace.hpp"
#include "../../Graphics/Mesh.hpp"
#include "../../Library.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentWorldText : public Component {
public:
    struct Vertex {
        Vector3 position;
        Vector2 offset;
        Vector2 size;
        Vector2 uv;
        Vector2 st;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, offset)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, size)},
                {3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
                {4, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, st)},
            };
        };
    };

    ComponentWorldText() = default;
    explicit ComponentWorldText(EntityId entity, const FontFace& fontFace, const Color4& color, const float height);
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

    [[nodiscard]] const FontFace& getFontFace() const {
        return *fontFace;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

    void setTextAlign(TextAlign value) {
        textAlign = value;
    }

private:
    bool dirty{false};
    const FontFace* fontFace{nullptr};
    Color4 color;
    float height;
    Mesh mesh;
    std::vector<Vertex> vertices;
    Vector2 offset;
    TextAlign textAlign{TextAlign::Center};
};
} // namespace Engine

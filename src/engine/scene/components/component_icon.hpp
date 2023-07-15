#pragma once

#include "../../assets/image.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentIcon : public Component {
public:
    struct Point {
        Vector3 position;
        Vector2 size;
        Vector4 color;
        Vector2 uv;
        Vector2 st;
        Vector2 offset;
        float padding;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Point, position)},
                {1, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, size)},
                {2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Point, color)},
                {3, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, uv)},
                {4, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, st)},
                {5, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, offset)},
            };
        };
    };

    ComponentIcon() = default;
    explicit ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image);
    virtual ~ComponentIcon() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentIcon);

    const ImagePtr& getImage() const {
        return image;
    }

    void setImage(const ImagePtr& value) {
        image = value;
        setDirty(true);
    }

    const Vector2& getSize() const {
        return size;
    }

    void setSize(const Vector2& value) {
        size = value;
        setDirty(true);
    }

    const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
        setDirty(true);
    }

    const Vector2& getOffset() const {
        return offset;
    }

    void setOffset(const Vector2& value) {
        offset = value;
        setDirty(true);
    }

    bool isSelectable() const {
        return selectable;
    }

    void setSelectable(const bool value) {
        selectable = value;
    }

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(image, size, color, offset, selectable);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    ImagePtr image;
    Vector2 size{32.0f, 32.0f};
    Color4 color{0.7f, 0.7f, 0.7f, 0.0f};
    Vector2 offset{0.0f};
    bool selectable{true};
};
} // namespace Engine

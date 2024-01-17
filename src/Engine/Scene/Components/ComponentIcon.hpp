#pragma once

#include "../../Assets/Image.hpp"
#include "../Component.hpp"

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
    explicit ComponentIcon(EntityId entity, ImagePtr image);
    COMPONENT_DEFAULTS(ComponentIcon);

    const ImagePtr& getImage() const {
        return image;
    }

    void setImage(const ImagePtr& value) {
        image = value;
    }

    const Vector2& getSize() const {
        return size;
    }

    void setSize(const Vector2& value) {
        size = value;
    }

    const Color4& getColor() const {
        return color;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    const Vector2& getOffset() const {
        return offset;
    }

    void setOffset(const Vector2& value) {
        offset = value;
    }

    bool isSelectable() const {
        return selectable;
    }

    void setSelectable(const bool value) {
        selectable = value;
    }

    bool isEnvironment() const {
        return environment;
    }

    void setEnvironment(bool value);

    MSGPACK_DEFINE_ARRAY(image, size, color, offset, selectable, environment);

private:
    ImagePtr image;
    Vector2 size{32.0f, 32.0f};
    Color4 color{0.7f, 0.7f, 0.7f, 1.0f};
    Vector2 offset{0.0f};
    bool selectable{true};
    bool environment{false};
};
} // namespace Engine

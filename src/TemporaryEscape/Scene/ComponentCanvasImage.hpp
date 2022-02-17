#pragma once

#include "../Assets/AssetImage.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentCanvasImage : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentCanvasImage() = default;
    explicit ComponentCanvasImage(Object& object, AssetImagePtr image, const Vector2& size, const Color4& color)
        : Component(object), image(std::move(image)), size(size), color(color) {
    }
    virtual ~ComponentCanvasImage() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    const AssetImagePtr& getImage() const {
        return image;
    }

    const Color4& getColor() const {
        return color;
    }

    const Vector2& getSize() const {
        return size;
    }

    const Vector2& getOffset() const {
        return offset;
    }
    void setOffset(const Vector2& offset) {
        ComponentCanvasImage::offset = offset;
    }

private:
    AssetImagePtr image;
    Vector2 size;
    Color4 color;
    Vector2 offset{0.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine

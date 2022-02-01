#pragma once

#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentDirectionalLight : public Component {
public:
    ComponentDirectionalLight() = default;
    explicit ComponentDirectionalLight(Object& object, const Color4& color) : Component(object), color(color) {
    }
    virtual ~ComponentDirectionalLight() = default;

    Color4 getColor() const {
        return color;
    }

private:
    Color4 color;

public:
    MSGPACK_DEFINE_ARRAY(color);
};
} // namespace Engine
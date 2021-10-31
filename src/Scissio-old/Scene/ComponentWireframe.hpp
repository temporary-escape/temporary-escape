#pragma once

#include "../Graphics/Primitives.hpp"
#include "Component.hpp"

namespace Scissio {
class ShaderWireframe;

class SCISSIO_API ComponentWireframe : public Component {
public:
    static constexpr ComponentType Type = 6;

    ComponentWireframe();
    explicit ComponentWireframe(Object& object, WireframeModel model, const Color4& color);
    ~ComponentWireframe() override = default;

    [[nodiscard]] WireframeModel getModel() const {
        return model;
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

    void setModel(const WireframeModel model) {
        this->model = model;
    }

    void setColor(const Color4& color) {
        this->color = color;
    }

    void render(ShaderWireframe& shader);

private:
    WireframeModel model;
    Color4 color;
    Mesh mesh;

public:
    MSGPACK_DEFINE_ARRAY(model, color);
};
} // namespace Scissio

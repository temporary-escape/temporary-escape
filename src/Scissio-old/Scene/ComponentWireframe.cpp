#include "ComponentWireframe.hpp"
#include "../Shaders/ShaderWireframe.hpp"

using namespace Scissio;

ComponentWireframe::ComponentWireframe() : Component(Type), model{WireframeModel::None}, color{0.0f}, mesh{NO_CREATE} {
}

ComponentWireframe::ComponentWireframe(Object& object, WireframeModel model, const Color4& color)
    : Component(Type, object), model{model}, color{color}, mesh{NO_CREATE} {
}

void ComponentWireframe::render(ShaderWireframe& shader) {
    const auto& transform = getObject().getTransform();

    if (!mesh) {
        mesh = createWireframeMesh(model);
    }

    shader.setColor(color);
    shader.setModelMatrix(transform);
    shader.draw(mesh);
}

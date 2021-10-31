#include "ComponentModel.hpp"

#include "../Shaders/ShaderModel.hpp"

using namespace Scissio;

ComponentModel::ComponentModel() : Component(Type) {
}

ComponentModel::ComponentModel(Object& object, ModelPtr model) : Component(Type, object), model(std::move(model)) {
}

void ComponentModel::render(ShaderModel& shader) {
    const auto& transform = getObject().getTransform();

    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));

    shader.setModelMatrix(transform);
    shader.setNormalMatrix(transformInverted);

    for (const auto& primitive : model->getPrimitives()) {
        const auto& material = primitive.material;
        if (material.baseColorTexture) {
            shader.bindBaseColorTexture(material.baseColorTexture->getTexture());
        } else {
            shader.bindBaseColorTextureDefault();
        }

        if (material.emissiveTexture) {
            shader.bindEmissiveTexture(material.emissiveTexture->getTexture());
        } else {
            shader.bindEmissiveTextureDefault();
        }

        if (material.normalTexture) {
            shader.bindNormalTexture(material.normalTexture->getTexture());
        } else {
            shader.bindNormalTextureDefault();
        }

        if (material.metallicRoughnessTexture) {
            shader.bindMetallicRoughnessTexture(material.metallicRoughnessTexture->getTexture());
        } else {
            shader.bindMetallicRoughnessTextureDefault();
        }

        if (material.ambientOcclusionTexture) {
            shader.bindAmbientOcclusionTexture(material.ambientOcclusionTexture->getTexture());
        } else {
            shader.bindAmbientOcclusionTextureDefault();
        }

        shader.draw(primitive.mesh);
    }
}

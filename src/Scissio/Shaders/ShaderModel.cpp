#include "ShaderModel.hpp"

using namespace Scissio;

ShaderModel::ShaderModel(const Config& config) {
    addVertexShader(config.shadersPath / "model.vert");
    addFragmentShader(config.shadersPath / "gbuffer.frag");
    link();
    use();

    setUniform("baseColorTexture", int(BaseColorTexture));
    setUniform("normalTexture", int(NormalTexture));
    setUniform("emissiveTexture", int(EmissiveTexture));
    setUniform("metallicRoughnessTexture", int(MetallicRoughnessTexture));
    setUniform("ambientOcclusionTexture", int(AmbientOcclusionTexture));

    transformationProjectionMatrixUniform = getUniformLocation("transformationProjectionMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
}

void ShaderModel::setTransformationProjectionMatrix(const glm::mat4x4& matrix) {
    setUniform(transformationProjectionMatrixUniform, matrix);
}

void ShaderModel::setProjectionMatrix(const glm::mat4x4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderModel::setModelMatrix(const glm::mat4x4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderModel::setNormalMatrix(const glm::mat3x3& matrix) {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderModel::bindBaseColorTexture(const Texture2D& texture) {
    texture.bind(BaseColorTexture);
}

void ShaderModel::bindNormalTexture(const Texture2D& texture) {
    texture.bind(NormalTexture);
}

void ShaderModel::bindEmissiveTexture(const Texture2D& texture) {
    texture.bind(EmissiveTexture);
}

void ShaderModel::bindMetallicRoughnessTexture(const Texture2D& texture) {
    texture.bind(MetallicRoughnessTexture);
}

void ShaderModel::bindAmbientOcclusionTexture(const Texture2D& texture) {
    texture.bind(AmbientOcclusionTexture);
}

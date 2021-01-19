#include "ShaderGrid.hpp"

using namespace Scissio;

ShaderGrid::ShaderGrid(const Config& config) {
    addVertexShader(config.shadersPath / "grid.vert");
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
}

void ShaderGrid::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(transformationProjectionMatrixUniform, matrix);
}

void ShaderGrid::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderGrid::setModelMatrix(const Matrix4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

/*void ShaderGrid::setNormalMatrix(const glm::mat3x3& matrix) {
    setUniform(normalMatrixUniform, matrix);
}*/

void ShaderGrid::bindBaseColorTexture(const Texture2D& texture) {
    texture.bind(BaseColorTexture);
}

void ShaderGrid::bindNormalTexture(const Texture2D& texture) {
    texture.bind(NormalTexture);
}

void ShaderGrid::bindEmissiveTexture(const Texture2D& texture) {
    texture.bind(EmissiveTexture);
}

void ShaderGrid::bindMetallicRoughnessTexture(const Texture2D& texture) {
    texture.bind(MetallicRoughnessTexture);
}

void ShaderGrid::bindAmbientOcclusionTexture(const Texture2D& texture) {
    texture.bind(AmbientOcclusionTexture);
}

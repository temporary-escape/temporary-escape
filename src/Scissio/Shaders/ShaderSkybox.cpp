#include "ShaderSkybox.hpp"

using namespace Scissio;

ShaderSkybox::ShaderSkybox(const Config& config) {
    addVertexShader(config.shadersPath / "skybox.vert");
    addFragmentShader(config.shadersPath / "skybox.frag");
    link();
    use();

    setUniform("skyboxTexture", int(SkyboxTexture));

    transformationProjectionMatrixUniform = getUniformLocation("transformationProjectionMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderSkybox::setTransformationProjectionMatrix(const glm::mat4x4& matrix) {
    setUniform(transformationProjectionMatrixUniform, matrix);
}

void ShaderSkybox::setModelMatrix(const Matrix4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderSkybox::bindSkyboxTexture(const TextureCubemap& texture) {
    texture.bind(SkyboxTexture);
}

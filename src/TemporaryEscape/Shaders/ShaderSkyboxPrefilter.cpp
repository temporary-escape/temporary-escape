#include "ShaderSkyboxPrefilter.hpp"

using namespace Engine;

ShaderSkyboxPrefilter::ShaderSkyboxPrefilter(const Config& config) : Shader("ShaderSkyboxPrefilter") {
    addVertexShader(config.shadersPath / "skybox-prefilter.vert");
    addFragmentShader(config.shadersPath / "skybox-prefilter.frag");
    link();
    use();

    setUniform("skyboxTexture", int(SkyboxTexture));

    projectionMatrixUniform = getUniformLocation("projection");
    viewMatrixUniform = getUniformLocation("view");
    roughnessUniform = getUniformLocation("roughness");
}

void ShaderSkyboxPrefilter::setProjectionMatrix(const glm::mat4x4& matrix) const {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderSkyboxPrefilter::setViewMatrix(const Matrix4& matrix) const {
    setUniform(viewMatrixUniform, matrix);
}

void ShaderSkyboxPrefilter::setRoughness(const float value) const {
    setUniform(roughnessUniform, value);
}

void ShaderSkyboxPrefilter::bindSkyboxTexture(const TextureCubemap& texture) const {
    texture.bind(SkyboxTexture);
}

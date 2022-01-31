#include "ShaderSkyboxIrradiance.hpp"

using namespace Engine;

ShaderSkyboxIrradiance::ShaderSkyboxIrradiance(const Config& config) : Shader("ShaderSkyboxIrradiance") {
    addVertexShader(config.shadersPath / "skybox-irradiance.vert");
    addFragmentShader(config.shadersPath / "skybox-irradiance.frag");
    link();
    use();

    setUniform("skyboxTexture", int(SkyboxTexture));

    projectionMatrixUniform = getUniformLocation("projection");
    viewMatrixUniform = getUniformLocation("view");
}

void ShaderSkyboxIrradiance::setProjectionMatrix(const glm::mat4x4& matrix) const {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderSkyboxIrradiance::setViewMatrix(const Matrix4& matrix) const {
    setUniform(viewMatrixUniform, matrix);
}

void ShaderSkyboxIrradiance::bindSkyboxTexture(const TextureCubemap& texture) const {
    texture.bind(SkyboxTexture);
}

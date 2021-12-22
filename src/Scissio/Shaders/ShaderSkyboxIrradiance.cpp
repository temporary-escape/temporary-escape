#include "ShaderSkyboxIrradiance.hpp"

using namespace Scissio;

ShaderSkyboxIrradiance::ShaderSkyboxIrradiance(const Config& config) : Shader("ShaderSkyboxIrradiance") {
    addVertexShader(config.shadersPath / "skybox-irradiance.vert");
    addFragmentShader(config.shadersPath / "skybox-irradiance.frag");
    link();
    use();

    setUniform("skyboxTexture", int(SkyboxTexture));

    projectionMatrixUniform = getUniformLocation("projection");
    viewMatrixUniform = getUniformLocation("view");
}

void ShaderSkyboxIrradiance::setProjectionMatrix(const glm::mat4x4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderSkyboxIrradiance::setViewMatrix(const Matrix4& matrix) {
    setUniform(viewMatrixUniform, matrix);
}

void ShaderSkyboxIrradiance::bindSkyboxTexture(const TextureCubemap& texture) {
    texture.bind(SkyboxTexture);
}

#include "ShaderSkyboxStars.hpp"

using namespace Scissio;

ShaderSkyboxStars::ShaderSkyboxStars(const Config& config) : Shader("ShaderSkyboxStars") {
    addVertexShader(config.shadersPath / "skybox-stars.vert");
    addFragmentShader(config.shadersPath / "skybox-stars.frag");
    addGeometryShader(config.shadersPath / "skybox-stars.geom");
    link();
    use();

    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    viewMatrixUniform = getUniformLocation("viewMatrix");
    particleSizeUniform = getUniformLocation("particleSize");
}

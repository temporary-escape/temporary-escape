#include "ShaderSkyboxNebula.hpp"

using namespace Scissio;

ShaderSkyboxNebula::ShaderSkyboxNebula(const Config& config) : Shader("ShaderSkyboxNebula") {
    addVertexShader(config.shadersPath / "skybox-nebula.vert");
    addFragmentShader(config.shadersPath / "skybox-nebula.frag");
    link();
    use();

    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    viewMatrixUniform = getUniformLocation("viewMatrix");
    colorUniform = getUniformLocation("uColor");
    offsetUniform = getUniformLocation("uOffset");
    scaleUniform = getUniformLocation("uScale");
    intensityUniform = getUniformLocation("uIntensity");
    falloffUniform = getUniformLocation("uFalloff");
}

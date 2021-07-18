#include "ShaderLines.hpp"

using namespace Scissio;

ShaderLines::ShaderLines(const Config& config) {
    addVertexShader(config.shadersPath / "lines.vert");
    addFragmentShader(config.shadersPath / "lines.frag");
    link();
    use();

    projectionViewMatrixUniform = getUniformLocation("projectionViewMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderLines::setProjectionViewMatrix(const glm::mat4x4& matrix) {
    setUniform(projectionViewMatrixUniform, matrix);
}

void ShaderLines::setModelMatrix(const glm::mat4x4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

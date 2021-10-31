#include "ShaderWireframe.hpp"

using namespace Scissio;

ShaderWireframe::ShaderWireframe(const Config& config) {
    addVertexShader(config.shadersPath / "wireframe.vert");
    addFragmentShader(config.shadersPath / "wireframe.frag");
    link();
    use();

    transformationProjectionMatrixUniform = getUniformLocation("transformationProjectionMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
    colorUniform = getUniformLocation("color");
    eyesPosUniform = getUniformLocation("eyesPos");
}

void ShaderWireframe::setTransformationProjectionMatrix(const glm::mat4x4& matrix) {
    setUniform(transformationProjectionMatrixUniform, matrix);
}

void ShaderWireframe::setModelMatrix(const glm::mat4x4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderWireframe::setEyesPos(const Vector3& pos) {
    setUniform(eyesPosUniform, pos);
}

void ShaderWireframe::setColor(const Color4& color) {
    setUniform(colorUniform, color);
}

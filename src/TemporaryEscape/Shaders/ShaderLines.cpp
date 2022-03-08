#include "ShaderLines.hpp"

using namespace Engine;

ShaderLines::ShaderLines(const Config& config) : Shader("ShaderLines") {
    addFragmentShader(config.shadersPath / "lines.frag");
    addVertexShader(config.shadersPath / "lines.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderLines::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderLines::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

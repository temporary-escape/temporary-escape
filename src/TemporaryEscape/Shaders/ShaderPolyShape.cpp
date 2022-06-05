#include "ShaderPolyShape.hpp"

using namespace Engine;

ShaderPolyShape::ShaderPolyShape(const Config& config) : Shader("ShaderPolyShape") {
    addFragmentShader(config.shadersPath / "poly-shape.frag");
    addVertexShader(config.shadersPath / "poly-shape.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderPolyShape::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPolyShape::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

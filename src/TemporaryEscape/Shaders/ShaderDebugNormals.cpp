#include "ShaderDebugNormals.hpp"

using namespace Engine;

ShaderDebugNormals::ShaderDebugNormals(const Config& config) : Shader("ShaderDebugNormals") {
    addFragmentShader(config.shadersPath / "debug-normals.frag");
    addGeometryShader(config.shadersPath / "debug-normals.geom");
    addVertexShader(config.shadersPath / "debug-normals.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
}

void ShaderDebugNormals::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderDebugNormals::setNormalMatrix(const Matrix3& matrix) const {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderDebugNormals::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

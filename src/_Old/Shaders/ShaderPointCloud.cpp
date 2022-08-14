#include "ShaderPointCloud.hpp"

using namespace Engine;

ShaderPointCloud::ShaderPointCloud(const Config& config) : Shader("ShaderPointCloud") {
    addFragmentShader(config.shadersPath / "point-cloud.frag");
    addGeometryShader(config.shadersPath / "point-cloud.geom");
    addVertexShader(config.shadersPath / "point-cloud.vert");
    link();
    use();

    setUniform("pointTexture", int(Samplers::PointTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderPointCloud::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPointCloud::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderPointCloud::bindTexture(const Texture& texture) const {
    texture.bind(Samplers::PointTexture);
}

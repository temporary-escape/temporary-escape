#include "ShaderPointCloud.hpp"

using namespace Scissio;

ShaderPointCloud::ShaderPointCloud(const Config& config) {
    addVertexShader(config.shadersPath / "point-cloud.vert");
    addGeometryShader(config.shadersPath / "point-cloud.geom");
    addFragmentShader(config.shadersPath / "point-cloud.frag");
    link();
    use();

    setUniform("pointTexture", int(PointTexture));
    viewMatrixUniform = getUniformLocation("viewMatrix");
    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderPointCloud::setViewMatrix(const glm::mat4x4& matrix) {
    setUniform(viewMatrixUniform, matrix);
}

void ShaderPointCloud::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderPointCloud::setModelMatrix(const glm::mat4x4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPointCloud::bindPointTexture(const Texture2D& texture) {
    texture.bind(PointTexture);
}

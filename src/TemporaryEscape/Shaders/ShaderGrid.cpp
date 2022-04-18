#include "ShaderGrid.hpp"

using namespace Engine;

ShaderGrid::ShaderGrid(const Config& config) : Shader("ShaderGrid") {
    addFragmentShader(config.shadersPath / "grid.frag");
    addGeometryShader(config.shadersPath / "grid.geom");
    addVertexShader(config.shadersPath / "grid.vert");
    link();
    use();

    setUniform("baseColorTexture", int(BaseColorTexture));
    setUniform("normalTexture", int(NormalTexture));
    setUniform("emissiveTexture", int(EmissiveTexture));
    setUniform("metallicRoughnessTexture", int(MetallicRoughnessTexture));
    setUniform("ambientOcclusionTexture", int(AmbientOcclusionTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    materialUniformIndex = getUniformBlockIndex("Material");
    uniformBlockBinding(materialUniformIndex, Bindings::Material);

    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
    objectIdUniform = getUniformLocation("objectId");
}

void ShaderGrid::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderGrid::setNormalMatrix(const Matrix3& matrix) const {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderGrid::setObjectId(const Vector2& color) const {
    setUniform(objectIdUniform, color);
}

void ShaderGrid::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderGrid::bindBaseColorTexture(const Texture& texture) const {
    texture.bind(BaseColorTexture);
}

void ShaderGrid::bindNormalTexture(const Texture& texture) const {
    texture.bind(NormalTexture);
}

void ShaderGrid::bindEmissiveTexture(const Texture& texture) const {
    texture.bind(EmissiveTexture);
}

void ShaderGrid::bindMetallicRoughnessTexture(const Texture& texture) const {
    texture.bind(MetallicRoughnessTexture);
}

void ShaderGrid::bindAmbientOcclusionTexture(const Texture& texture) const {
    texture.bind(AmbientOcclusionTexture);
}

void ShaderGrid::bindMaterialUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Material);
}

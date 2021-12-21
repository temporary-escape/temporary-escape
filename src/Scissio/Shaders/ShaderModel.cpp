#include "ShaderModel.hpp"

using namespace Scissio;

ShaderModel::ShaderModel(const Config& config) : Shader("ShaderModel") {
    addFragmentShader(config.shadersPath / "model.frag");
    addVertexShader(config.shadersPath / "model.vert");
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

    objectIdUniform = getUniformLocation("objectId");
    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
}

void ShaderModel::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderModel::setNormalMatrix(const Matrix3& matrix) const {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderModel::bindBaseColorTexture(const Texture& texture) const {
    texture.bind(BaseColorTexture);
}

void ShaderModel::bindNormalTexture(const Texture& texture) const {
    texture.bind(NormalTexture);
}

void ShaderModel::bindEmissiveTexture(const Texture& texture) const {
    texture.bind(EmissiveTexture);
}

void ShaderModel::bindMetallicRoughnessTexture(const Texture& texture) const {
    texture.bind(MetallicRoughnessTexture);
}

void ShaderModel::bindAmbientOcclusionTexture(const Texture& texture) const {
    texture.bind(AmbientOcclusionTexture);
}

void ShaderModel::setObjectId(const uint16_t id) const {
    Color4 color;
    color.r = (id & 0xFF) / 255.0f;
    color.g = ((id & 0xFF00) >> 8) / 255.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    setUniform(objectIdUniform, color);
}

void ShaderModel::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderModel::bindMaterialUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Material);
}

#include "ShaderModel.hpp"

using namespace Scissio;

ShaderPbr::ShaderPbr(const Config& config) {
}

void ShaderPbr::complete() {
    setUniform("baseColorTexture", int(BaseColorTexture));
    setUniform("normalTexture", int(NormalTexture));
    setUniform("emissiveTexture", int(EmissiveTexture));
    setUniform("metallicRoughnessTexture", int(MetallicRoughnessTexture));
    setUniform("ambientOcclusionTexture", int(AmbientOcclusionTexture));

    transformationProjectionMatrixUniform = getUniformLocation("transformationProjectionMatrix");
    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
    objectIdUniform = getUniformLocation("objectId");

    const auto setColor = [](Texture2D& texture, const Color4& color) {
        std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
        for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
            pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
            pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
            pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
            pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
        }

        texture.setStorage(0, {8, 8}, PixelType::Rgba8u);
        texture.setPixels(0, {0, 0}, {8, 8}, PixelType::Rgba8u, pixels.get());
    };

    setColor(defaultBaseColor, Color4{1.0f, 0.0f, 1.0f, 1.0f});
    setColor(defaultNormal, Color4{0.5f, 0.5f, 1.0f, 1.0f});
    setColor(defaultEmissive, Color4{0.0f, 0.0f, 0.0f, 1.0f});
    setColor(defaultMetallicRoughness, Color4{0.0f, 0.5f, 0.5f, 1.0f});
    setColor(defaultAmbientOcclusion, Color4{1.0f, 1.0f, 1.0f, 1.0f});
}

void ShaderPbr::setModelMatrix(const glm::mat4x4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPbr::setNormalMatrix(const glm::mat3x3& matrix) {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderPbr::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(transformationProjectionMatrixUniform, matrix);
}

void ShaderPbr::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderPbr::bindBaseColorTexture(const Texture2D& texture) {
    texture.bind(BaseColorTexture);
}

void ShaderPbr::bindNormalTexture(const Texture2D& texture) {
    texture.bind(NormalTexture);
}

void ShaderPbr::bindEmissiveTexture(const Texture2D& texture) {
    texture.bind(EmissiveTexture);
}

void ShaderPbr::bindMetallicRoughnessTexture(const Texture2D& texture) {
    texture.bind(MetallicRoughnessTexture);
}

void ShaderPbr::bindAmbientOcclusionTexture(const Texture2D& texture) {
    texture.bind(AmbientOcclusionTexture);
}

void ShaderPbr::bindBaseColorTextureDefault() {
    defaultBaseColor.bind(BaseColorTexture);
}

void ShaderPbr::bindNormalTextureDefault() {
    defaultNormal.bind(NormalTexture);
}

void ShaderPbr::bindEmissiveTextureDefault() {
    defaultEmissive.bind(EmissiveTexture);
}

void ShaderPbr::bindMetallicRoughnessTextureDefault() {
    defaultMetallicRoughness.bind(MetallicRoughnessTexture);
}

void ShaderPbr::bindAmbientOcclusionTextureDefault() {
    defaultAmbientOcclusion.bind(AmbientOcclusionTexture);
}

void ShaderPbr::setObjectId(const uint16_t id) {
    Color4 color;
    color.r = (id & 0xFF) / 255.0f;
    color.g = ((id & 0xFF00) >> 8) / 255.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    setUniform(objectIdUniform, color);
}

#include "ShaderPbrBuffer.hpp"

using namespace Scissio;

ShaderPbrBuffer::ShaderPbrBuffer(const Config& config) {
    addVertexShader(config.shadersPath / "quad.vert");
    addFragmentShader(config.shadersPath / "pbr.frag");
    link();
    use();

    setUniform("depthTexture", int(DepthTexture));
    setUniform("baseColorRoughnessTexture", int(BaseColorRoughnessTexture));
    setUniform("normalMetallicTexture", int(NormalMetallicTexture));
    setUniform("emissiveAmbientTexture", int(EmissiveAmbientTexture));
    setUniform("brdfTexture", int(BrdfTexture));
    setUniform("irradianceTexture", int(SkyboxIrradianceTexture));
    setUniform("prefilterTexture", int(SkyboxPrefilterTexture));

    viewportUniform = getUniformLocation("viewport");
    viewProjectionInverseMatrixUniform = getUniformLocation("viewProjectionInverseMatrix");
    eyesPosUniform = getUniformLocation("eyesPos");
}

void ShaderPbrBuffer::setViewProjectionInverseMatrix(const Matrix4& matrix) {
    setUniform(viewProjectionInverseMatrixUniform, matrix);
}

void ShaderPbrBuffer::bindDepthTexture(const Texture& texture) {
    texture.bind(DepthTexture);
}

void ShaderPbrBuffer::bindBaseColorRoughnessTexture(const Texture& texture) {
    texture.bind(BaseColorRoughnessTexture);
}

void ShaderPbrBuffer::bindNormalMetallicTexture(const Texture& texture) {
    texture.bind(NormalMetallicTexture);
}

void ShaderPbrBuffer::bindEmissiveAmbientTexture(const Texture& texture) {
    texture.bind(EmissiveAmbientTexture);
}

void ShaderPbrBuffer::bindBrdfTexture(const Texture& texture) {
    texture.bind(BrdfTexture);
}

void ShaderPbrBuffer::bindSkyboxIrradianceTexture(const Texture& texture) {
    texture.bind(SkyboxIrradianceTexture);
}

void ShaderPbrBuffer::bindSkyboxPrefilterTexture(const Texture& texture) {
    texture.bind(SkyboxPrefilterTexture);
}

void ShaderPbrBuffer::setViewport(const Vector2i& viewport) {
    setUniform(viewportUniform, viewport);
}

void ShaderPbrBuffer::setEyesPos(const Vector3& pos) {
    setUniform(eyesPosUniform, pos);
}

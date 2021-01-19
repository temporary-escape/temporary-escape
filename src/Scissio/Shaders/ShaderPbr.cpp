#include "ShaderPbr.hpp"

using namespace Scissio;

ShaderPbr::ShaderPbr(const Config& config) {
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

void ShaderPbr::setViewProjectionInverseMatrix(const Matrix4& matrix) {
    setUniform(viewProjectionInverseMatrixUniform, matrix);
}

void ShaderPbr::bindDepthTexture(const Texture& texture) {
    texture.bind(DepthTexture);
}

void ShaderPbr::bindBaseColorRoughnessTexture(const Texture& texture) {
    texture.bind(BaseColorRoughnessTexture);
}

void ShaderPbr::bindNormalMetallicTexture(const Texture& texture) {
    texture.bind(NormalMetallicTexture);
}

void ShaderPbr::bindEmissiveAmbientTexture(const Texture& texture) {
    texture.bind(EmissiveAmbientTexture);
}

void ShaderPbr::bindBrdfTexture(const Texture& texture) {
    texture.bind(BrdfTexture);
}

void ShaderPbr::bindSkyboxIrradianceTexture(const Texture& texture) {
    texture.bind(SkyboxIrradianceTexture);
}

void ShaderPbr::bindSkyboxPrefilterTexture(const Texture& texture) {
    texture.bind(SkyboxPrefilterTexture);
}

void ShaderPbr::setViewport(const Vector2i& viewport) {
    setUniform(viewportUniform, viewport);
}

void ShaderPbr::setEyesPos(const Vector3& pos) {
    setUniform(eyesPosUniform, pos);
}

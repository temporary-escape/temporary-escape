#include "ShaderPbr.hpp"

using namespace Scissio;

ShaderPbr::ShaderPbr(const Config& config) : Shader("ShaderPbr") {
    addVertexShader(config.shadersPath / "pbr.vert");
    addFragmentShader(config.shadersPath / "pbr.frag");
    link();
    use();

    setUniform("depthTexture", int(DepthTexture));
    setUniform("baseColorTexture", int(BaseColorTexture));
    setUniform("emissiveTexture", int(EmissiveTexture));
    setUniform("metallicRoughnessAmbientTexture", int(MetallicRoughnessAmbientTexture));
    setUniform("normalTexture", int(NormalTexture));
    setUniform("brdfTexture", int(BrdfTexture));
    setUniform("irradianceTexture", int(SkyboxIrradianceTexture));
    setUniform("prefilterTexture", int(SkyboxPrefilterTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    directionalLightsIndex = getUniformBlockIndex("DirectionalLights");
    uniformBlockBinding(directionalLightsIndex, Bindings::DirectionalLights);
}

void ShaderPbr::bindDepthTexture(const Texture& texture) const {
    texture.bind(DepthTexture);
}

void ShaderPbr::bindBaseColorTexture(const Texture& texture) const {
    texture.bind(BaseColorTexture);
}

void ShaderPbr::bindNormalTexture(const Texture& texture) const {
    texture.bind(NormalTexture);
}

void ShaderPbr::bindEmissiveTexture(const Texture& texture) const {
    texture.bind(EmissiveTexture);
}

void ShaderPbr::bindMetallicRoughnessAmbientTexture(const Texture& texture) const {
    texture.bind(MetallicRoughnessAmbientTexture);
}

void ShaderPbr::bindBrdfTexture(const Texture& texture) const {
    texture.bind(BrdfTexture);
}

void ShaderPbr::bindSkyboxIrradianceTexture(const Texture& texture) const {
    texture.bind(SkyboxIrradianceTexture);
}

void ShaderPbr::bindSkyboxPrefilterTexture(const Texture& texture) const {
    texture.bind(SkyboxPrefilterTexture);
}

void ShaderPbr::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderPbr::bindDirectionalLightsUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::DirectionalLights);
}

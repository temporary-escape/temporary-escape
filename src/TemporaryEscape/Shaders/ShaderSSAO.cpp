#include "ShaderSSAO.hpp"

#define CMP "ShaderSSAO"

using namespace Engine;

ShaderSSAO::ShaderSSAO(const Config& config) : Shader("ShaderSSAO") {
    addVertexShader(config.shadersPath / "ssao.vert");
    addFragmentShader(config.shadersPath / "ssao.frag");
    link();
    use();

    setUniform("depthTexture", int(DepthTexture));
    setUniform("normalTexture", int(NormalTexture));
    setUniform("noiseTexture", int(NoiseTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    samplesUniform = getUniformLocation("samples");
    noiseScaleUniform = getUniformLocation("noiseScale");
}

void ShaderSSAO::bindDepthTexture(const Texture& texture) const {
    texture.bind(DepthTexture);
}

void ShaderSSAO::bindNormalTexture(const Texture& texture) const {
    texture.bind(NormalTexture);
}

void ShaderSSAO::bindNoiseTexture(const Texture& texture) const {
    texture.bind(NoiseTexture);
}

void ShaderSSAO::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderSSAO::setNoiseScale(const Vector2& value) const {
    setUniform(noiseScaleUniform, value);
}

void ShaderSSAO::setSamples(const std::vector<Vector3>& samples) const {
    if (samples.size() != 64) {
        EXCEPTION(CMP, "Expected 64 SSAO samples");
    }
    setUniform(samplesUniform, samples.data(), samples.size());
}

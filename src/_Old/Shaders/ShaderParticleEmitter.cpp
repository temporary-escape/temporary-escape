#include "ShaderParticleEmitter.hpp"

using namespace Engine;

ShaderParticleEmitter::ShaderParticleEmitter(const Config& config) : Shader("ShaderParticleEmitter") {
    addFragmentShader(config.shadersPath / "particle-emitter.frag");
    addGeometryShader(config.shadersPath / "particle-emitter.geom");
    addVertexShader(config.shadersPath / "particle-emitter.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    particleDataUniformIndex = getUniformBlockIndex("ParticleData");
    uniformBlockBinding(particleDataUniformIndex, Bindings::ParticleData);

    modelMatrixUniform = getUniformLocation("modelMatrix");
    modelViewMatrixUniform = getUniformLocation("modelViewMatrix");
    timeUniform = getUniformLocation("time");
}

void ShaderParticleEmitter::setTime(const float value) const {
    setUniform(timeUniform, value);
}

void ShaderParticleEmitter::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderParticleEmitter::setModelViewMatrix(const Matrix4& matrix) const {
    setUniform(modelViewMatrixUniform, matrix);
}

void ShaderParticleEmitter::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderParticleEmitter::bindParticleData(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::ParticleData);
}

void ShaderParticleEmitter::bindParticleTexture(const Texture& texture) const {
    texture.bind(int(Samplers::ParticleTexture));
}

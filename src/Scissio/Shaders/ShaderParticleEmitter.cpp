#include "ShaderParticleEmitter.hpp"

using namespace Scissio;

ShaderParticleEmitter::ShaderParticleEmitter(const Config& config) {
    addVertexShader(config.shadersPath / "particle-emitter.vert");
    addFragmentShader(config.shadersPath / "particle-emitter.frag");
    link();
    use();

    projectionMatrixUniform = getUniformLocation("projectionMatrix");
    viewMatrixUniform = getUniformLocation("viewMatrix");
    modelMatrixUniform = getUniformLocation("modelMatrix");
    positionUniform = getUniformLocation("position");
}

void ShaderParticleEmitter::setViewMatrix(const Matrix4& matrix) {
    setUniform(viewMatrixUniform, matrix);
}

void ShaderParticleEmitter::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(projectionMatrixUniform, matrix);
}

void ShaderParticleEmitter::setModelMatrix(const Matrix4& matrix) {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderParticleEmitter::bindParticleTexture(const Texture2D& texture) {
    texture.bind(ParticleTexture);
}

void ShaderParticleEmitter::setPosition(const Vector3& vector) {
    setUniform(positionUniform, vector);
}

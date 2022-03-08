#include "ShaderFXAA.hpp"

using namespace Engine;

ShaderFXAA::ShaderFXAA(const Config& config) : Shader("ShaderFXAA") {
    addVertexShader(config.shadersPath / "fxaa.vert");
    addFragmentShader(config.shadersPath / "fxaa.frag");
    link();
    use();

    setUniform("Texture", int(InputTexture));

    frameDirectionUniform = getUniformLocation("FrameDirection");
    frameCountUniform = getUniformLocation("FrameCount");
    outputSizeUniform = getUniformLocation("OutputSize");
    textureSizeUniform = getUniformLocation("TextureSize");
    inputSizeUniform = getUniformLocation("InputSize");
    mvpMatrixUniform = getUniformLocation("MVPMatrix");
}

void ShaderFXAA::bindTexture(const Texture& texture) const {
    texture.bind(InputTexture);
}

void ShaderFXAA::setFrameDirection(int value) const {
    setUniform(frameDirectionUniform, value);
}

void ShaderFXAA::setFrameCount(int value) const {
    setUniform(frameCountUniform, value);
}

void ShaderFXAA::setOutputSize(const Vector2& value) const {
    setUniform(outputSizeUniform, value);
}

void ShaderFXAA::setTextureSize(const Vector2& value) const {
    setUniform(textureSizeUniform, value);
}

void ShaderFXAA::setInputSize(const Vector2& value) const {
    setUniform(inputSizeUniform, value);
}

void ShaderFXAA::setMvpMatrix(const Matrix4& value) const {
    setUniform(mvpMatrixUniform, value);
}

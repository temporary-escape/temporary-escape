#include "ShaderSkybox.hpp"

using namespace Engine;

ShaderSkybox::ShaderSkybox(const Config& config) : Shader("ShaderSkybox") {
    addFragmentShader(config.shadersPath / "skybox.frag");
    addVertexShader(config.shadersPath / "skybox.vert");
    link();
    use();

    setUniform("skyboxTexture", int(SkyboxTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    modelMatrixUniform = getUniformLocation("modelMatrix");
}

void ShaderSkybox::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderSkybox::bindSkyboxTexture(const Texture& texture) const {
    texture.bind(SkyboxTexture);
}

void ShaderSkybox::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

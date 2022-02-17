#include "ShaderBullet.hpp"

using namespace Engine;

ShaderBullet::ShaderBullet(const Config& config) : Shader("ShaderBullet") {
    addFragmentShader(config.shadersPath / "bullet.frag");
    addVertexShader(config.shadersPath / "bullet.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);
}

void ShaderBullet::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

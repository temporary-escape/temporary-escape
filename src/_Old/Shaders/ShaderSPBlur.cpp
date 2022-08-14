#include "ShaderSPBlur.hpp"

#define CMP "ShaderSSAO"

using namespace Engine;

ShaderSPBlur::ShaderSPBlur(const Config& config) : Shader("ShaderSPBlur") {
    addVertexShader(config.shadersPath / "sp-blur.vert");
    addFragmentShader(config.shadersPath / "sp-blur.frag");
    link();
    use();

    setUniform("colorTexture", int(ColorTexture));
}

void ShaderSPBlur::bindColorTexture(const Texture& texture) const {
    texture.bind(ColorTexture);
}

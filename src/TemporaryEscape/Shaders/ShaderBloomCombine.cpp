#include "ShaderBloomCombine.hpp"

using namespace Engine;

ShaderBloomCombine::ShaderBloomCombine(const Config& config) : Shader("ShaderBloomCombine") {
    addVertexShader(config.shadersPath / "bloom-combine.vert");
    addFragmentShader(config.shadersPath / "bloom-combine.frag");
    link();
    use();

    setUniform("colorTexture", int(ColorTexture));
    setUniform("bloomTexture", int(BloomTexture));
}

void ShaderBloomCombine::bindBloomTexture(const Texture& texture) const {
    texture.bind(BloomTexture);
}

void ShaderBloomCombine::bindColorTexture(const Texture& texture) const {
    texture.bind(ColorTexture);
}

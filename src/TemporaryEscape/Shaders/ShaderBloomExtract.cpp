#include "ShaderBloomExtract.hpp"

using namespace Engine;

ShaderBloomExtract::ShaderBloomExtract(const Config& config) : Shader("ShaderBloomExtract") {
    addVertexShader(config.shadersPath / "bloom-extract.vert");
    addFragmentShader(config.shadersPath / "bloom-extract.frag");
    link();
    use();

    setUniform("inputTexture", int(InputTexture));

    brightnessUniform = getUniformLocation("brightness");
}

void ShaderBloomExtract::bindTexture(const Texture& texture) const {
    texture.bind(InputTexture);
}

void ShaderBloomExtract::setBrightness(float value) const {
    setUniform(brightnessUniform, value);
}

#include "ShaderGBufferView.hpp"

using namespace Scissio;

ShaderGBufferView::ShaderGBufferView(const Config& config) {

    addVertexShader(config.shadersPath / "quad.vert");
    addFragmentShader(config.shadersPath / "gbuffer-debug.frag");
    link();
    use();

    setUniform("depthTexture", int(DepthTexture));
    setUniform("baseColorRoughnessTexture", int(BaseColorRoughnessTexture));
    setUniform("normalMetallicTexture", int(NormalMetallicTexture));
    setUniform("emissiveAmbientTexture", int(EmissiveAmbientTexture));

    viewportUniform = getUniformLocation("viewport");
    modeUniform = getUniformLocation("mode");
}

void ShaderGBufferView::bindDepthTexture(const Texture& texture) {
    texture.bind(DepthTexture);
}

void ShaderGBufferView::bindBaseColorRoughnessTexture(const Texture& texture) {
    texture.bind(BaseColorRoughnessTexture);
}

void ShaderGBufferView::bindNormalMetallicTexture(const Texture& texture) {
    texture.bind(NormalMetallicTexture);
}

void ShaderGBufferView::bindEmissiveAmbientTexture(const Texture& texture) {
    texture.bind(EmissiveAmbientTexture);
}

void ShaderGBufferView::setViewport(const Vector2i& viewport) {
    setUniform(viewportUniform, viewport);
}

void ShaderGBufferView::setMode(const int mode) {
    setUniform(modeUniform, mode);
}

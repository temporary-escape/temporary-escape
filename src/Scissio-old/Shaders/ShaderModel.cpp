#include "ShaderModel.hpp"

using namespace Scissio;

ShaderModel::ShaderModel(const Config& config) : ShaderPbr(config) {
    addFragmentShader(config.shadersPath / "gbuffer.frag");
    addVertexShader(config.shadersPath / "model.vert");
    link();
    use();
    complete();
}

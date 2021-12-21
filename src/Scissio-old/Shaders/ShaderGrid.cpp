#include "ShaderGrid.hpp"

using namespace Scissio;

ShaderGrid::ShaderGrid(const Config& config) : ShaderPbr(config) {
    addFragmentShader(config.shadersPath / "model.frag");
    addVertexShader(config.shadersPath / "grid.vert");
    link();
    use();
    complete();
}

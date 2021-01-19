#include "ShaderBrdf.hpp"

using namespace Scissio;

ShaderBrdf::ShaderBrdf(const Config& config) {
    addVertexShader(config.shadersPath / "brdf.vert");
    addFragmentShader(config.shadersPath / "brdf.frag");
    link();
    use();
}

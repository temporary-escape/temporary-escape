#include "ShaderBrdf.hpp"

using namespace Scissio;

ShaderBrdf::ShaderBrdf(const Config& config) : Shader("ShaderBrdf") {
    addVertexShader(config.shadersPath / "brdf.vert");
    addFragmentShader(config.shadersPath / "brdf.frag");
    link();
    use();
}

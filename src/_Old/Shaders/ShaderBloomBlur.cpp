#include "ShaderBloomBlur.hpp"

using namespace Engine;

ShaderBloomBlur::ShaderBloomBlur(const Config& config) : Shader("ShaderBloomBlur") {
    addVertexShader(config.shadersPath / "bloom-blur.vert");
    addFragmentShader(config.shadersPath / "bloom-blur.frag");
    link();
    use();

    setUniform("inputTexture", int(InputTexture));

    horizontalUniform = getUniformLocation("horizontal");
    weightsUniform = getUniformLocation("weight");
    radiusUniform = getUniformLocation("radius");
}

void ShaderBloomBlur::bindTexture(const Texture& texture) const {
    texture.bind(InputTexture);
}

void ShaderBloomBlur::setHorizontal(const bool value) const {
    setUniform(horizontalUniform, value);
}

static float gaussian(float x, float mean, float stddev) {
    const float a = (x - mean) / stddev;
    return std::exp(-0.5 * a * a);
}

void ShaderBloomBlur::setSize(const int size) const {
    const auto stddev = static_cast<float>(size) / 2.0f;

    const auto kernelMean = static_cast<float>(size) * 2 + 1;
    const auto kernelSize = size + 1;

    std::vector<float> weights;
    weights.resize(kernelSize);

    for (size_t i = 0; i < kernelSize; i++) {
        weights[i] = gaussian(static_cast<float>(i), (kernelMean - 1) / 2.0f, stddev);
    }

    float sum = 0.0;
    for (size_t i = 1; i < kernelSize; i++) {
        sum += weights[i] * 2;
    }
    sum += weights[0];

    for (size_t i = 0; i < weights.size(); ++i) {
        weights[i] = weights[i] / sum;
    }

    sum = 0.0;
    for (size_t i = 1; i < kernelSize; i++) {
        sum += weights[i] * 2;
    }
    sum += weights[0];

    setUniform(weightsUniform, weights.data(), weights.size());
    setUniform(radiusUniform, size);
}

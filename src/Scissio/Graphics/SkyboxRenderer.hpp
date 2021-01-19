#pragma once

#include "../Config.hpp"
#include "../Shaders/ShaderSkyboxIrradiance.hpp"
#include "../Shaders/ShaderSkyboxNebula.hpp"
#include "../Shaders/ShaderSkyboxPrefilter.hpp"
#include "../Shaders/ShaderSkyboxStars.hpp"
#include "Framebuffer.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"
#include "TextureCubemap.hpp"

#include <random>

namespace Scissio {
class SCISSIO_API SkyboxRenderer {
public:
    explicit SkyboxRenderer(const Config& config);
    virtual ~SkyboxRenderer() = default;

    TextureCubemap render(int64_t seed);
    TextureCubemap irradiance(const TextureCubemap& texture);
    TextureCubemap prefilter(const TextureCubemap& texture);

    Skybox renderAndFilter(const int64_t seed) {
        Skybox skybox;
        skybox.texture = render(seed);
        skybox.irradiance = irradiance(skybox.texture);
        skybox.prefilter = prefilter(skybox.texture);
        return skybox;
    }

private:
    void renderStars(std::mt19937_64& rng, TextureCubemap& result, int count, float particleSize);
    void renderNebulas(std::mt19937_64& rng, TextureCubemap& result);

    const Config& config;

    ShaderSkyboxStars shaderStars;
    ShaderSkyboxNebula shaderNebula;
    ShaderSkyboxIrradiance shaderIrradiance;
    ShaderSkyboxPrefilter shaderPrefilter;

    Framebuffer fbo;
    Mesh box;
    Renderbuffer fboDepth;
};
} // namespace Scissio

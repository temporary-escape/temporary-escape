#pragma once

#include "../Shaders/ShaderBloomBlur.hpp"
#include "../Shaders/ShaderBloomCombine.hpp"
#include "../Shaders/ShaderBloomExtract.hpp"
#include "../Shaders/ShaderBrdf.hpp"
#include "../Shaders/ShaderBullet.hpp"
#include "../Shaders/ShaderDebugNormals.hpp"
#include "../Shaders/ShaderFXAA.hpp"
#include "../Shaders/ShaderGrid.hpp"
#include "../Shaders/ShaderLines.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Shaders/ShaderParticleEmitter.hpp"
#include "../Shaders/ShaderPbr.hpp"
#include "../Shaders/ShaderPlanetAtmosphere.hpp"
#include "../Shaders/ShaderPlanetSurface.hpp"
#include "../Shaders/ShaderPointCloud.hpp"
#include "../Shaders/ShaderSPBlur.hpp"
#include "../Shaders/ShaderSSAO.hpp"
#include "../Shaders/ShaderSkybox.hpp"

namespace Engine {
struct Shaders {
    explicit Shaders(const Config& config)
        : skybox(config), model(config), grid(config), brdf(config), pbr(config), planetSurface(config),
          planetAtmosphere(config), particleEmitter(config), bullet(config), fxaa(config), pointCloud(config),
          lines(config), ssao(config), spBlur(config), bloomExtract(config), bloomBlur(config), bloomCombine(config),
          debugNormals(config) {
    }

    Shaders(const Shaders& other) = delete;
    Shaders(Shaders&& other) = default;
    Shaders& operator=(const Shaders& other) = delete;
    Shaders& operator=(Shaders&& other) = default;

    ShaderSkybox skybox;
    ShaderModel model;
    ShaderGrid grid;
    ShaderBrdf brdf;
    ShaderPbr pbr;
    ShaderPlanetSurface planetSurface;
    ShaderPlanetAtmosphere planetAtmosphere;
    ShaderParticleEmitter particleEmitter;
    ShaderBullet bullet;
    ShaderFXAA fxaa;
    ShaderPointCloud pointCloud;
    ShaderLines lines;
    ShaderSSAO ssao;
    ShaderSPBlur spBlur;
    ShaderBloomExtract bloomExtract;
    ShaderBloomBlur bloomBlur;
    ShaderBloomCombine bloomCombine;
    ShaderDebugNormals debugNormals;
};
} // namespace Engine

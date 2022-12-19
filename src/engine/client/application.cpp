#include "application.hpp"

#define CMP "Application"

using namespace Engine;

Application::Application(const Config& config) :
    VulkanWindow(config, config.windowName, {config.windowWidth, config.windowHeight}),
    config{config},
    renderer{config, *this, rendererPipelines},
    skyboxGenerator{config, *this, skyboxGeneratorPipelines},
    canvas{*this},
    font{*this, config.fontsPath, "iosevka-aile", 42.0f} {

    shaderQueue.emplace([this]() {
        rendererPipelines.brdf = createPipeline({
            {this->config.shadersPath / "brdf.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "brdf.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.copy = createPipeline({
            {this->config.shadersPath / "pass-copy.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-copy.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.pbr = createPipeline({
            {this->config.shadersPath / "pass-pbr.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-pbr.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.skybox = createPipeline({
            {this->config.shadersPath / "pass-skybox.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-skybox.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.fxaa = createPipeline({
            {this->config.shadersPath / "pass-fxaa.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-fxaa.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.ssao = createPipeline({
            {this->config.shadersPath / "pass-ssao.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-ssao.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.bloomExtract = createPipeline({
            {this->config.shadersPath / "pass-bloom-extract.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-bloom-extract.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.bloomBlur = createPipeline({
            {this->config.shadersPath / "pass-bloom-blur.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-bloom-blur.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        rendererPipelines.bloomCombine = createPipeline({
            {this->config.shadersPath / "pass-bloom-combine.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "pass-bloom-combine.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        skyboxGeneratorPipelines.stars = createPipeline({
            {this->config.shadersPath / "skybox-stars.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "skybox-stars.geom", "", ShaderType::Geometry},
            {this->config.shadersPath / "skybox-stars.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        skyboxGeneratorPipelines.nebula = createPipeline({
            {this->config.shadersPath / "skybox-nebula.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "skybox-nebula.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        skyboxGeneratorPipelines.irradiance = createPipeline({
            {this->config.shadersPath / "skybox-irradiance.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "skybox-irradiance.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        skyboxGeneratorPipelines.prefilter = createPipeline({
            {this->config.shadersPath / "skybox-prefilter.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "skybox-prefilter.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.debug = createPipeline({
            {this->config.shadersPath / "component-debug.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-debug.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.grid = createPipeline({
            {this->config.shadersPath / "component-grid.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-grid.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.wireframe = createPipeline({
            {this->config.shadersPath / "component-wireframe.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-wireframe.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.pointCloud = createPipeline({
            {this->config.shadersPath / "component-point-cloud.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-point-cloud.geom", "", ShaderType::Geometry},
            {this->config.shadersPath / "component-point-cloud.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.iconPointCloud = createPipeline({
            {this->config.shadersPath / "component-icon-point-cloud.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-icon-point-cloud.geom", "", ShaderType::Geometry},
            {this->config.shadersPath / "component-icon-point-cloud.vert", "", ShaderType::Vertex},
        });
    });

    shaderQueue.emplace([this]() {
        scenePipelines.lines = createPipeline({
            {this->config.shadersPath / "component-lines.frag", "", ShaderType::Fragment},
            {this->config.shadersPath / "component-lines.vert", "", ShaderType::Vertex},
        });
    });

    status.message = "Loading shaders...";
    status.value = 0.1f;
}

Application::~Application() {
}

void Application::render(const Vector2i& viewport, const float deltaTime) {
    startCommandBuffer();

    if (loadGame) {
        loadGame = false;
        try {
            game =
                std::make_unique<Game>(config, *this, *registry, canvas, font, scenePipelines, skyboxGenerator, status);
            status.message = "Starting...";
            status.value = 0.8f;
        } catch (...) {
            EXCEPTION_NESTED("Something went wrong during game init");
        }
    }

    if (loadAssets && !registry->isReady()) {
        status.message = "Loading assets...";
        status.value = 0.5f;

        registry->load(*this);
        if (registry->isReady()) {
            status.message = "Loading scene...";
            status.value = 0.7f;
            loadGame = true;
        }
    }

    if (loadRegistry) {
        status.message = "Loading mods...";
        status.value = 0.2f;

        futureRegistry = async([this]() {
            try {
                registry = std::make_unique<Registry>(this->config);
            } catch (...) {
                EXCEPTION_NESTED("Failed to load mods");
            }
            loadAssets = true;
        });
        loadRegistry = false;
    }

    if (loadShaders && !shaderQueue.empty()) {
        shaderQueue.front()();
        shaderQueue.pop();

        if (shaderQueue.empty()) {
            loadRegistry = true;
        }
    }

    if (futureRegistry.valid() && futureRegistry.ready()) {
        futureRegistry.get();
    }

    if (game) {
        try {
            game->update(deltaTime);
        } catch (...) {
            EXCEPTION_NESTED("Something went wrong during game update");
        }
    }

    skyboxGenerator.render();

    renderer.update(viewport);
    renderer.begin();
    if (game && game->hasView()) {
        try {
            game->render(viewport, renderer);
        } catch (...) {
            EXCEPTION_NESTED("Render frame caught exception");
        }

        // renderer.renderPassFront(false);

        try {
            game->renderCanvas(viewport);
        } catch (...) {
            EXCEPTION_NESTED("Render canvas caught exception");
        }
    } else {
        renderer.renderPassFront(true);
        renderStatus(viewport);
    }
    renderer.end();

    endCommandBuffer();
    renderer.present();

    if (!loadShaders && !shaderQueue.empty()) {
        loadShaders = true;
    }
}

void Application::renderStatus(const Vector2i& viewport) {
    canvas.begin(viewport);

    static const Vector2 size{400.0f, 60.0f};
    const auto pos = Vector2{viewport} / 2.0f - size / 2.0f;

    canvas.text(pos + Vector2{0.0f, 0.0f}, status.message, font.regular, 21.0f, Color4{1.0f});
    canvas.rect(pos + Vector2{0.0f, 30.0f}, {size.x * status.value, 25.0f}, Color4{1.0f});

    canvas.end();
}

void Application::eventMouseMoved(const Vector2i& pos) {
    if (game) {
        game->eventMouseMoved(pos);
    }
}

void Application::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (game) {
        game->eventMousePressed(pos, button);
    }
}

void Application::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (game) {
        game->eventMouseReleased(pos, button);
    }
}

void Application::eventMouseScroll(int xscroll, int yscroll) {
    if (game) {
        game->eventMouseScroll(xscroll, yscroll);
    }
}

void Application::eventKeyPressed(Key key, Modifiers modifiers) {
    if (game) {
        game->eventKeyPressed(key, modifiers);
    }
}

void Application::eventKeyReleased(Key key, Modifiers modifiers) {
    if (game) {
        game->eventKeyReleased(key, modifiers);
    }
}

void Application::eventWindowResized(const Vector2i& size) {
}

void Application::eventCharTyped(uint32_t code) {
    if (game) {
        game->eventCharTyped(code);
    }
}

#include "game.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Game::Game(const Config& config, Renderer& renderer, SkyboxGenerator& skyboxGenerator, PlanetGenerator& planetGenerator,
           Registry& registry, FontFamily& font, Client& client) :
    config{config},
    renderer{renderer},
    skyboxGenerator{skyboxGenerator},
    planetGenerator{planetGenerator},
    registry{registry},
    font{font},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.1f, 0.1f, 0.1f, 1.0f}} {

    viewSpace = std::make_unique<ViewSpace>(*this, config, renderer, registry, skybox, client);
    viewGalaxy = std::make_unique<ViewGalaxy>(*this, config, renderer, registry, client, font);
    viewSystem = std::make_unique<ViewSystem>(*this, config, renderer, registry, client, font);
    view = viewSpace.get();
}

Game::~Game() = default;

void Game::update(float deltaTime) {
    view->update(deltaTime);
}

void Game::render(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    if (client.getSystemSeed() != 0 && client.getSystemSeed() != skyboxSeed) {
        skyboxSeed = client.getSystemSeed();
        renderer.getVulkan().waitQueueIdle();
        skybox = skyboxGenerator.generate(skyboxSeed);
        client.getScene()->setSkybox(skybox);
    }

    planetGenerator.run();

    if (auto scene = client.getScene(); scene != nullptr) {
        planetGenerator.update(*scene);
    }

    renderer.render(vkb, viewport, view->getScene());
}

void Game::renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport) {
    nuklear.begin(viewport);
    // gui.draw(nuklear, viewport);
    nuklear.end();
}

/*void Game::renderCanvas(const Vector2i& viewport) {
    canvas.begin(viewport);

    if (view) {
        view->renderCanvas(viewport, canvas);

        nuklear.begin(viewport);
        view->renderGui(viewport, nuklear);
        nuklear.end();
    }

    canvas.end();
}*/

void Game::eventMouseMoved(const Vector2i& pos) {
    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Game::switchToGalaxyMap() {
    if (view != viewGalaxy.get()) {
        logger.info("Switching to galaxy map scene...");
        view = viewGalaxy.get();
        viewGalaxy->load();
    } else {
        logger.info("Switching to space scene...");
        view = viewSpace.get();
    }
}

void Game::switchToSystemMap() {
    if (view != viewSystem.get()) {
        logger.info("Switching to system map scene...");
        view = viewSystem.get();
        viewSystem->load();
    } else {
        logger.info("Switching to system scene...");
        view = viewSpace.get();
    }
}

void Game::switchToSystemMap(const std::string& galaxyId, const std::string& systemId) {
    if (view != viewSystem.get()) {
        logger.info("Switching to system map scene...");
        view = viewSystem.get();
        viewSystem->load(galaxyId, systemId);
    } else {
        logger.info("Switching to system scene...");
        view = viewSpace.get();
    }
}

void Game::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    /*if (gui.contextMenu.isEnabled() && !gui.contextMenu.isCursorInside(pos)) {
        gui.contextMenu.setEnabled(false);
    }*/

    if (view) {
        view->eventMousePressed(pos, button);
    }
}

void Game::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (view) {
        view->eventMouseReleased(pos, button);
    }
}

void Game::eventMouseScroll(const int xscroll, const int yscroll) {
    if (view) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}

void Game::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (view) {
        view->eventKeyPressed(key, modifiers);
    }

    if (config.input.galaxyMapToggle(key, modifiers)) {
        switchToGalaxyMap();
    } else if (config.input.systemMapToggle(key, modifiers)) {
        switchToSystemMap();
    }
}

void Game::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (view) {
        view->eventKeyReleased(key, modifiers);
    }
}

void Game::eventCharTyped(const uint32_t code) {
    if (view) {
        view->eventCharTyped(code);
    }
}

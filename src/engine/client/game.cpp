#include "game.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Game::Game(const Config& config, Renderer& renderer, SkyboxGenerator& skyboxGenerator, PlanetGenerator& planetGenerator,
           AssetsManager& assetsManager, FontFamily& font, Client& client) :
    config{config},
    renderer{renderer},
    skyboxGenerator{skyboxGenerator},
    planetGenerator{planetGenerator},
    assetsManager{assetsManager},
    font{font},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.1f, 0.1f, 0.1f, 1.0f}},
    guiGalaxy{config, assetsManager},
    guiSystem{config, assetsManager} {

    viewSpace = std::make_unique<ViewSpace>(*this, config, renderer, assetsManager, font, skybox, client);
    viewGalaxy = std::make_unique<ViewGalaxy>(*this, config, renderer, assetsManager, client, guiGalaxy, font);
    viewSystem = std::make_unique<ViewSystem>(*this, config, renderer, assetsManager, client, guiSystem, font);
    view = viewSpace.get();
}

Game::~Game() = default;

void Game::update(float deltaTime) {
    if (const auto scene = client.getScene(); scene) {
        scene->update(deltaTime);
    }
    view->update(deltaTime);
}

bool Game::isReady() const {
    return !planetGenerator.isBusy() && !skyboxGenerator.isBusy() && client.getScene() &&
           client.getScene()->getPrimaryCamera();
}

void Game::render(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    if (!planetGenerator.isBusy()) {
        skyboxGenerator.run();
        if (auto scene = client.getScene(); scene != nullptr) {
            skyboxGenerator.update(*scene);
        }
    }

    if (!skyboxGenerator.isBusy()) {
        planetGenerator.run();
        if (auto scene = client.getScene(); scene != nullptr) {
            planetGenerator.update(*scene);
        }
    }

    auto* scene = view->getScene();
    if (scene && scene->getPrimaryCamera()) {
        renderer.render(vkb, viewport, *scene);
    }
}

void Game::renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport) {
    if (view) {
        view->renderCanvas(canvas, viewport);
    }
    nuklear.begin(viewport);
    guiGalaxy.modalLoading.draw(nuklear, viewport);
    guiSystem.modalLoading.draw(nuklear, viewport);
    nuklear.end();
}

void Game::eventMouseMoved(const Vector2i& pos) {
    renderer.setMousePos(pos);

    if (view) {
        view->eventMouseMoved(pos);
    }
}

void Game::switchToGalaxyMap() {
    if (view != viewGalaxy.get()) {
        logger.info("Switching to galaxy map scene...");
        view = viewGalaxy.get();
        viewGalaxy->onEnter();
    } else {
        logger.info("Switching to space scene...");
        viewGalaxy->onExit();
        view = viewSpace.get();
    }
}

void Game::switchToSystemMap() {
    if (view != viewSystem.get()) {
        logger.info("Switching to system map scene...");
        view = viewSystem.get();
        viewSystem->onEnter();
    } else {
        logger.info("Switching to system scene...");
        viewSystem->onExit();
        view = viewSpace.get();
    }
}

void Game::switchToSystemMap(const std::string& galaxyId, const std::string& systemId) {
    if (view != viewSystem.get()) {
        logger.info("Switching to system map scene...");
        view = viewSystem.get();
        viewSystem->reset(galaxyId, systemId);
    } else {
        logger.info("Switching to system scene...");
        viewSystem->onExit();
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

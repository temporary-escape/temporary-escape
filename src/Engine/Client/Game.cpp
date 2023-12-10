#include "Game.hpp"
#include "../Graphics/Theme.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Game::Game(const Config& config, VulkanRenderer& vulkan, RendererSkybox& rendererSkybox,
           RendererPlanetSurface& rendererPlanetSurface, AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache,
           FontFamily& font, Client& client) :
    config{config},
    vulkan{vulkan},
    rendererSkybox{rendererSkybox},
    rendererPlanetSurface{rendererPlanetSurface},
    assetsManager{assetsManager},
    font{font},
    client{client} {

    guiMainMenu.setItems({
        {"Continue", [this]() { guiMainMenu.setEnabled(false); }},
        {"Exit Game", [this]() { this->vulkan.closeWindow(); }},
    });
    guiMainMenu.setEnabled(false);

    viewSpace = std::make_unique<ViewSpace>(*this, config, vulkan, assetsManager, voxelShapeCache, font, client);
    viewGalaxy = std::make_unique<ViewGalaxy>(*this, config, vulkan, assetsManager, voxelShapeCache, client, font);
    viewSystem = std::make_unique<ViewSystem>(*this, config, vulkan, assetsManager, voxelShapeCache, client, font);
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
    return !rendererPlanetSurface.isBusy() && !rendererSkybox.isBusy() && client.isReady() &&
           client.getScene()->getPrimaryCamera();
}

void Game::render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport) {
    rendererSkybox.render();
    if (!rendererPlanetSurface.isBusy()) {
        if (auto scene = client.getScene(); scene != nullptr) {
            rendererSkybox.update(*scene);
        }
    }

    rendererPlanetSurface.render();
    if (!rendererSkybox.isBusy()) {
        if (auto scene = client.getScene(); scene != nullptr) {
            rendererPlanetSurface.update(*scene);
        }
    }

    auto* scene = view->getScene();
    if (scene && scene->getPrimaryCamera()) {
        renderer.render(vkb, *scene);
    }
}

void Game::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    /*if (view) {
        view->renderCanvas(canvas, viewport);
        nuklear.begin(viewport);
        if (guiMainMenu.isEnabled()) {
            guiMainMenu.draw(nuklear, viewport);
        } else {
            view->renderNuklear(nuklear, viewport);
        }
        nuklear.end();
    }*/
}

void Game::eventMouseMoved(const Vector2i& pos) {
    if (view && !guiMainMenu.isEnabled()) {
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

    if (view && !guiMainMenu.isEnabled()) {
        view->eventMousePressed(pos, button);
    }
}

void Game::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventMouseReleased(pos, button);
    }
}

void Game::eventMouseScroll(const int xscroll, const int yscroll) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventMouseScroll(xscroll, yscroll);
    }
}

void Game::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventKeyPressed(key, modifiers);
    }

    if (key == Key::Escape) {
        guiMainMenu.setEnabled(!guiMainMenu.isEnabled());
    } else if (config.input.galaxyMapToggle(key, modifiers)) {
        switchToGalaxyMap();
    } else if (config.input.systemMapToggle(key, modifiers)) {
        switchToSystemMap();
    }
}

void Game::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventKeyReleased(key, modifiers);
    }
}

void Game::eventCharTyped(const uint32_t code) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventCharTyped(code);
    }
}

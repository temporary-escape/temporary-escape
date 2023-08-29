#include "game.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

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

    if (client.getCache().playerEntityId && control.update) {
        control.update = false;
        MessageShipControlEvent msg{};

        if (control.forward && !control.backwards) {
            msg.speed = 50.0f;
        } else if (!control.forward && control.backwards) {
            msg.speed = -20.0f;
        } else {
            msg.speed = 0.0f;
        }

        if (control.left && !control.right) {
            msg.leftRight = -1;
        } else if (!control.left && control.right) {
            msg.leftRight = 1;
        } else {
            msg.leftRight = 0;
        }

        if (control.up && !control.down) {
            msg.upDown = 1;
        } else if (!control.up && control.down) {
            msg.upDown = -1;
        } else {
            msg.upDown = 0;
        }

        client.send(msg);
    }
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

void Game::renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport) {
    if (view) {
        view->renderCanvas(canvas, viewport);
        nuklear.begin(viewport);
        if (guiMainMenu.isEnabled()) {
            guiMainMenu.draw(nuklear, viewport);
        } else {
            view->renderNuklear(nuklear, viewport);
        }
        nuklear.end();
    }
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

    if (key == Key::LetterW) {
        control.forward = true;
        control.update = true;
    } else if (key == Key::LetterS) {
        control.backwards = true;
        control.update = true;
    } else if (key == Key::LetterA) {
        control.left = true;
        control.update = true;
    } else if (key == Key::LetterD) {
        control.right = true;
        control.update = true;
    } else if (key == Key::SpaceBar) {
        control.up = true;
        control.update = true;
    } else if (key == Key::LeftControl) {
        control.down = true;
        control.update = true;
    }
}

void Game::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventKeyReleased(key, modifiers);
    }

    if (key == Key::LetterW) {
        control.forward = false;
        control.update = true;
    } else if (key == Key::LetterS) {
        control.backwards = false;
        control.update = true;
    } else if (key == Key::LetterA) {
        control.left = false;
        control.update = true;
    } else if (key == Key::LetterD) {
        control.right = false;
        control.update = true;
    } else if (key == Key::SpaceBar) {
        control.up = false;
        control.update = true;
    } else if (key == Key::LeftControl) {
        control.down = false;
        control.update = true;
    }
}

void Game::eventCharTyped(const uint32_t code) {
    if (view && !guiMainMenu.isEnabled()) {
        view->eventCharTyped(code);
    }
}

#include "game.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

#define CMP "Game"

using namespace Engine;

Game::Game(const Config& config, Renderer& renderer, Canvas& canvas, Nuklear& nuklear, SkyboxGenerator& skyboxGenerator,
           Registry& registry, Client& client) :
    config{config},
    renderer{renderer},
    canvas{canvas},
    nuklear{nuklear},
    skyboxGenerator{skyboxGenerator},
    registry{registry},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.1f, 0.1f, 0.1f, 1.0f}} {

    viewSpace = std::make_unique<ViewSpace>(config, renderer, registry, skybox, client);
    viewGalaxy = std::make_unique<ViewGalaxy>(config, renderer, registry, client);
    viewSystem = std::make_unique<ViewSystem>(config, renderer, registry, client);
    view = viewSpace.get();
}

Game::~Game() = default;

void Game::update(float deltaTime) {
    view->update(deltaTime);
}

void Game::render(const Vector2i& viewport) {
    if (client.getSystemSeed() != 0 && client.getSystemSeed() != skyboxSeed) {
        skyboxSeed = client.getSystemSeed();
        renderer.getVulkan().waitQueueIdle();
        skybox = skyboxGenerator.generate(skyboxSeed);
    }

    view->render(viewport);
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

void Game::eventMousePressed(const Vector2i& pos, const MouseButton button) {
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
        if (view != viewGalaxy.get()) {
            Log::i(CMP, "Switching to galaxy map scene...");
            view = viewGalaxy.get();
            viewGalaxy->load();
        } else {
            Log::i(CMP, "Switching to space scene...");
            view = viewSpace.get();
        }
    } else if (config.input.systemMapToggle(key, modifiers)) {
        if (view != viewSystem.get()) {
            Log::i(CMP, "Switching to system map scene...");
            view = viewSystem.get();
            viewSystem->load();
        } else {
            Log::i(CMP, "Switching to system scene...");
            view = viewSpace.get();
        }
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

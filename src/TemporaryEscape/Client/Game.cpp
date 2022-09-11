#include "Game.hpp"

#define CMP "Game"

using namespace Engine;

Game::Game(const Config& config, VulkanDevice& vulkan, Registry& registry, Canvas& canvas, FontFamily& font,
           Scene::Pipelines& scenePipelines) :
    config{config},
    vulkan{vulkan},
    registry{registry},
    canvas{canvas},
    font{font},
    scenePipelines{scenePipelines},
    userInput{config, *this},
    nuklear{canvas, font.regular, 21.0f} {
}

Game::~Game() {
}

void Game::update(float deltaTime) {
    if (!viewBuild) {
        viewBuild = std::make_unique<ViewBuild>(config, vulkan, scenePipelines, registry, canvas, font, nuklear);
        view = viewBuild.get();
    }

    if (view) {
        view->update(deltaTime);
    }
}

void Game::render(const Vector2i& viewport, Renderer& renderer) {
    if (view) {
        view->render(viewport, renderer);
    }
}

void Game::renderCanvas(const Vector2i& viewport) {
    canvas.begin(viewport);

    if (view) {
        view->renderCanvas(viewport);
    }

    /*nuklear.begin(viewport);

    const auto flags = Nuklear::WindowFlags::Border | Nuklear::WindowFlags::Background | Nuklear::WindowFlags::Title;
    if (nuklear.beginWindow("Hello World!", {200.0f, 200.0f}, {500.0f, 500.0f}, flags)) {
        nuklear.layoutDynamic(30.0f, 1);
        nuklear.button("Click me");
        nuklear.endWindow();
    }

    nuklear.end();*/
    canvas.end();
}

const Skybox* Game::getSkybox() {
    if (view) {
        return view->getSkybox();
    }
    return nullptr;
}

void Game::eventUserInput(const UserInput::Event& event) {
    if (view) {
        view->eventUserInput(event);
    }
}

void Game::eventMouseMoved(const Vector2i& pos) {
    nuklear.eventMouseMoved(pos);
    userInput.eventMouseMoved(pos);
}

void Game::eventMousePressed(const Vector2i& pos, MouseButton button) {
    nuklear.eventMousePressed(pos, button);
    userInput.eventMousePressed(pos, button);
}

void Game::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    nuklear.eventMouseReleased(pos, button);
    userInput.eventMouseReleased(pos, button);
}

void Game::eventMouseScroll(int xscroll, int yscroll) {
    nuklear.eventMouseScroll(xscroll, yscroll);
    userInput.eventMouseScroll(xscroll, yscroll);
}

void Game::eventKeyPressed(Key key, Modifiers modifiers) {
    nuklear.eventKeyPressed(key, modifiers);
    userInput.eventKeyPressed(key, modifiers);
}

void Game::eventKeyReleased(Key key, Modifiers modifiers) {
    nuklear.eventKeyReleased(key, modifiers);
    userInput.eventKeyReleased(key, modifiers);
}

void Game::eventWindowResized(const Vector2i& size) {
}

void Game::eventCharTyped(uint32_t code) {
    nuklear.eventCharTyped(code);
}

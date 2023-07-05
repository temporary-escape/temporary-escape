#include "editor.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Editor::Editor(const Config& config, Renderer& renderer, AssetsManager& assetsManager, FontFamily& font) :
    config{config},
    renderer{renderer},
    assetsManager{assetsManager},
    font{font},
    guiBuild{config, assetsManager},
    view{config, renderer, assetsManager, guiBuild} {

    view.onEnter();
}

Editor::~Editor() {
    view.onExit();
}

void Editor::update(float deltaTime) {
    view.update(deltaTime);
}

void Editor::render(VulkanCommandBuffer& vkb, const Vector2i& viewport) {
    auto* scene = view.getScene();
    if (scene) {
        renderer.render(vkb, viewport, *scene);
    }
}

void Editor::renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport) {
    nuklear.begin(viewport);
    guiBuild.blockActionBar.draw(nuklear, viewport);
    guiBuild.blockSelector.draw(nuklear, viewport);
    guiBuild.blockSideMenu.draw(nuklear, viewport);
    nuklear.end();
}

void Editor::eventMouseMoved(const Vector2i& pos) {
    view.eventMouseMoved(pos);
}

void Editor::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    view.eventMousePressed(pos, button);
}

void Editor::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    view.eventMouseReleased(pos, button);
}

void Editor::eventMouseScroll(const int xscroll, const int yscroll) {
    view.eventMouseScroll(xscroll, yscroll);
}

void Editor::eventKeyPressed(const Key key, const Modifiers modifiers) {
    view.eventKeyPressed(key, modifiers);
}

void Editor::eventKeyReleased(const Key key, const Modifiers modifiers) {
    view.eventKeyReleased(key, modifiers);
}

void Editor::eventCharTyped(const uint32_t code) {
    view.eventCharTyped(code);
}

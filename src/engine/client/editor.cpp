#include "editor.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

Editor::Editor(const Config& config, Renderer& renderer, Canvas& canvas, Nuklear& nuklear, Registry& registry,
               FontFamily& font) :
    config{config},
    renderer{renderer},
    canvas{canvas},
    nuklear{nuklear},
    registry{registry},
    font{font},
    gui{config, registry},
    view{config, renderer, registry, gui} {

    gui.blockSelector.setBlocks(registry.getBlocks().findAll());
    view.onEnter();
}

Editor::~Editor() {
    view.onExit();
}

void Editor::update(float deltaTime) {
    view.update(deltaTime);
}

void Editor::render(const Vector2i& viewport) {
    renderer.render(viewport, view.getRenderScene(), view.getRenderSkybox(), view.getRenderOptions(), gui);
}

void Editor::eventMouseMoved(const Vector2i& pos) {
    view.eventMouseMoved(pos);
}

void Editor::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (gui.contextMenu.isEnabled() && !gui.contextMenu.isCursorInside(pos)) {
        gui.contextMenu.setEnabled(false);
    }

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

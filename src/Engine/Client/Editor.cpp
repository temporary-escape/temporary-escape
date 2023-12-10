#include "Editor.hpp"
#include "../Graphics/Theme.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Editor::Editor(const Config& config, VulkanRenderer& vulkan, AudioContext& audio, AssetsManager& assetsManager,
               VoxelShapeCache& voxelShapeCache, FontFamily& font) :
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    font{font},
    view{config, vulkan, audio, assetsManager, voxelShapeCache} {

    guiMainMenu.setItems({
        {"Continue", [this]() { guiMainMenu.setEnabled(false); }},
        {"Exit Game", [this]() { this->vulkan.closeWindow(); }},
    });
    guiMainMenu.setEnabled(false);

    view.onEnter();
}

Editor::~Editor() {
    view.onExit();
}

void Editor::update(float deltaTime) {
    view.update(deltaTime);
}

void Editor::render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport) {
    auto* scene = view.getScene();
    if (scene) {
        renderer.render(vkb, *scene);
    }
}

void Editor::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    /*view.renderCanvas(canvas, viewport);
    nuklear.begin(viewport);
    if (guiMainMenu.isEnabled()) {
        guiMainMenu.draw(nuklear, viewport);
    } else {
        view.renderNuklear(nuklear, viewport);
    }
    nuklear.end();*/
}

void Editor::eventMouseMoved(const Vector2i& pos) {
    if (!guiMainMenu.isEnabled()) {
        view.eventMouseMoved(pos);
    }
}

void Editor::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (!guiMainMenu.isEnabled()) {
        view.eventMousePressed(pos, button);
    }
}

void Editor::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (!guiMainMenu.isEnabled()) {
        view.eventMouseReleased(pos, button);
    }
}

void Editor::eventMouseScroll(const int xscroll, const int yscroll) {
    if (!guiMainMenu.isEnabled()) {
        view.eventMouseScroll(xscroll, yscroll);
    }
}

void Editor::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (!guiMainMenu.isEnabled()) {
        view.eventKeyPressed(key, modifiers);
    }

    if (key == Key::Escape) {
        guiMainMenu.setEnabled(!guiMainMenu.isEnabled());
    }
}

void Editor::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (!guiMainMenu.isEnabled()) {
        view.eventKeyReleased(key, modifiers);
    }
}

void Editor::eventCharTyped(const uint32_t code) {
    if (!guiMainMenu.isEnabled()) {
        view.eventCharTyped(code);
    }
}

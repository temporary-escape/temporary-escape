#include "editor.hpp"
#include "../graphics/theme.hpp"
#include "../utils/random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Editor::Editor(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
               VoxelShapeCache& voxelShapeCache, FontFamily& font) :
    config{config}, assetsManager{assetsManager}, font{font}, view{config, vulkan, assetsManager, voxelShapeCache} {

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

void Editor::renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport) {
    view.renderCanvas(canvas, viewport);
    nuklear.begin(viewport);
    view.renderNuklear(nuklear, viewport);
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

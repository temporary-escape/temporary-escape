#include "View.hpp"
#include "../Graphics/RendererBackground.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewContext::ViewContext(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                         GuiManager& guiManager, RendererBackground& rendererBackground) :
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    guiManager{guiManager},
    rendererBackground{rendererBackground},
    current{nullptr} {
}

void ViewContext::setCurrent(View* value) {
    for (auto& view : views) {
        if (view.get() == value) {
            if (current) {
                current->onExit();
            }
            current = view.get();
            current->onEnter();
        }
    }
}

void ViewContext::clear() {
    if (current) {
        current->onExit();
    }
    current = nullptr;
    views.clear();
}

void ViewContext::update(const float deltaTime, const Vector2i& viewport) {
    for (auto& view : views) {
        view->update(deltaTime, viewport);
    }
}

void ViewContext::render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport) {
    if (!current) {
        return;
    }

    auto* scene = current->getScene();

    rendererBackground.render();
    if (scene) {
        rendererBackground.update(*scene);
    }

    if (scene && scene->getPrimaryCamera()) {
        renderer.render(vkb, *scene);
    }
}

void ViewContext::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    if (!current) {
        return;
    }

    current->renderCanvas(canvas, viewport);
}

void ViewContext::eventMouseMoved(const Vector2i& pos) {
    if (current) {
        current->eventMouseMoved(pos);
    }
}

void ViewContext::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (current) {
        current->eventMousePressed(pos, button);
    }
}

void ViewContext::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (current) {
        current->eventMouseReleased(pos, button);
    }
}

void ViewContext::eventMouseScroll(const int xscroll, const int yscroll) {
    if (current) {
        current->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewContext::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (current) {
        current->eventKeyPressed(key, modifiers);
    }
}

void ViewContext::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (current) {
        current->eventKeyReleased(key, modifiers);
    }
}

void ViewContext::eventCharTyped(const uint32_t code) {
    if (current) {
        current->eventCharTyped(code);
    }
}

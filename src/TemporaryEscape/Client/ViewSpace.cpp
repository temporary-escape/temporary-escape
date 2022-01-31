#include "ViewSpace.hpp"
#include "Client.hpp"

using namespace Engine;

ViewSpace::ViewSpace(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Client& client, GuiContext& gui)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client), gui(gui) {
}

void ViewSpace::render(const Vector2i& viewport) {
    if (auto scene = client.getScene(); scene != nullptr) {
        renderer.render(viewport, *scene);
    }
}

void ViewSpace::renderGui(const Vector2i& viewport) {
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSpace::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewSpace::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewSpace::eventMouseScroll(int xscroll, int yscroll) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewSpace::eventKeyPressed(Key key, Modifiers modifiers) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewSpace::eventKeyReleased(Key key, Modifiers modifiers) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventKeyReleased(key, modifiers);
    }
}

#include "ViewSpace.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"

#define CMP "ViewSpace"

using namespace Engine;

ViewSpace::ViewSpace(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                     Client& client, Widgets& widgets, Store& store)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client), widgets(widgets),
      store(store) {

    images.info = assetManager.find<AssetImage>("icon_info");
    images.rotation = assetManager.find<AssetImage>("icon_anticlockwise_rotation");
    images.approach = assetManager.find<AssetImage>("icon_convergence_target");
    images.target = assetManager.find<AssetImage>("icon_target");
    fontFaceRegular = assetManager.find<AssetFontFamily>("iosevka-aile")->get("regular");
}

void ViewSpace::render(const Vector2i& viewport) {
    if (auto scene = client.getScene(); scene != nullptr) {
        renderer.render(*scene);
        // selected.hover = renderer.queryEntityAtPos(*scene, mousePosCurrent);
    } else {
        selected.hover.reset();
    }
}

void ViewSpace::renderGui(const Vector2i& viewport) {
    auto scene = client.getScene();
    if (!scene) {
        return;
    }

    if (contextMenu.show) {
        selected.hover.reset();
    }

    if (selected.hover) {
        const auto camera = scene->getPrimaryCamera();
        const auto pos = camera->worldToScreen(selected.hover->getPosition());

        auto label = selected.hover->findComponent<ComponentLabel>();
        if (label) {
            canvas.fontFace(fontFaceRegular->getHandle());
            canvas.fontSize(22.0f);

            canvas.beginPath();
            canvas.fillColor(GuiColors::text);
            canvas.text(pos, label->getLabel());
            canvas.closePath();
        };

        static const Vector2 markerSize{64.0f};

        canvas.beginPath();
        canvas.fillColor(GuiColors::text);
        canvas.rectImage(pos - markerSize / 2.0f, markerSize, images.target->getImage());
        canvas.fill();
        canvas.closePath();
    }

    if (contextMenu.show) {
        // const auto camera = renderer.getPrimaryCamera(*scene);
        // const auto pos = camera->worldToScreen(contextMenu.entity->getPosition());
        std::vector<Widgets::ContextMenuItem> items = {
            {images.approach, "Approach", [&]() { contextMenu.show = false; }},
            {images.rotation, "Orbit", [&]() { contextMenu.show = false; }},
            {images.info, "Info", [&]() { contextMenu.show = false; }},
        };
        widgets.contextMenu(contextMenu.pos, items);
    }
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSpace::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventMousePressed(pos, button);

        if (contextMenu.show) {
            contextMenu.show = false;
        } else if (button == MouseButton::Right && selected.hover) {
            contextMenu.show = true;
            contextMenu.entity = selected.hover;
            contextMenu.pos = pos;
        }
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

        auto changed = false;
        if (key == Key::LetterA) {
            movement.req.left = true;
            changed = true;
        }
        if (key == Key::LetterD) {
            movement.req.right = true;
            changed = true;
        }
        if (key == Key::LetterS) {
            movement.req.down = true;
            changed = true;
        }
        if (key == Key::LetterW) {
            movement.req.up = true;
            changed = true;
        }

        if (changed) {
            client.send(movement.req);
        }
    }
}

void ViewSpace::eventKeyReleased(Key key, Modifiers modifiers) {
    if (auto scene = client.getScene(); scene != nullptr) {
        scene->eventKeyReleased(key, modifiers);

        auto changed = false;
        if (key == Key::LetterA) {
            movement.req.left = false;
            changed = true;
        }
        if (key == Key::LetterD) {
            movement.req.right = false;
            changed = true;
        }
        if (key == Key::LetterS) {
            movement.req.down = false;
            changed = true;
        }
        if (key == Key::LetterW) {
            movement.req.up = false;
            changed = true;
        }

        if (changed) {
            client.send(movement.req);
        }
    }
}

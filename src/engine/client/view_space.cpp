#include "view_space.hpp"
#include "client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewSpace::ViewSpace(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                     VoxelShapeCache& voxelShapeCache, FontFamily& font, Client& client) :
    parent{parent},
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    voxelShapeCache{voxelShapeCache},
    font{font},
    client{client} {
}

void ViewSpace::update(const float deltaTime) {
    /*auto scene = client.getScene();
    if (const auto selected = scene->getSelectedEntity(); selected != selectedEntity) {
        if (selectedEntity) {
            auto* icon = selectedEntity->tryGetComponent<ComponentIcon>();
            if (icon) {
                auto color = icon->getColor();
                color.a = 0.0f;
                icon->setColor(color);
            }
        }

        selectedEntity = selected;
        if (selectedEntity) {
            logger.info("Selected entity: {}", static_cast<uint32_t>(selectedEntity->getHandle()));

            if (selectedEntity) {
                auto* icon = selectedEntity->tryGetComponent<ComponentIcon>();
                if (icon) {
                    auto color = icon->getColor();
                    color.a = 1.0f;
                    icon->setColor(color);
                }
            }
        } else {
            logger.info("Selected entity: null");
        }
    }*/
}

void ViewSpace::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    auto scene = client.getScene();
    auto camera = scene ? scene->getPrimaryCamera() : nullptr;
    if (scene && camera) {
        renderCanvasSelectedEntity(canvas, *scene, *camera);
    }
}

void ViewSpace::renderNuklear(Nuklear& nuklear, const Vector2i& viewport) {
}

void ViewSpace::renderCanvasSelectedEntity(Canvas& canvas, const Scene& scene, const ComponentCamera& camera) {
    const auto selectedEntity = scene.getSelectedEntity();
    if (selectedEntity) {
        const auto& transform = selectedEntity->getComponent<ComponentTransform>();
        const auto worldPos = transform.getAbsolutePosition();
        const auto screenPos = scene.worldToScreen(worldPos);

        const auto dist = glm::distance(camera.getEyesPos(), worldPos) / 1000.0f;
        const char* fmt;

        if (dist < 1.0f) {
            fmt = "{:.2f} km";
        } else if (dist < 10.0f) {
            fmt = "{:.1f} km";
        } else {
            fmt = "{:.0f} km";
        }

        const auto text = fmt::format(fmt, dist);

        canvas.font(font.light, config.guiFontSize);
        canvas.color(Color4{1.0f});
        canvas.text(screenPos + Vector2{20.0f, 0.0f}, text);

        const auto* label = selectedEntity->tryGetComponent<ComponentLabel>();
        if (label) {
            canvas.text(screenPos + Vector2{20.0f, config.guiFontSize}, label->getLabel());
        }
    }
}

Scene* ViewSpace::getScene() {
    auto scene = client.getScene();
    if (!scene) {
        EXCEPTION("No scene present!");
    }
    return scene;
}

void ViewSpace::onEnter() {
}

void ViewSpace::onExit() {
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSpace::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewSpace::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewSpace::eventMouseScroll(const int xscroll, const int yscroll) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewSpace::eventKeyPressed(const Key key, const Modifiers modifiers) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewSpace::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyReleased(key, modifiers);
    }
}

void ViewSpace::eventCharTyped(const uint32_t code) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventCharTyped(code);
    }
}

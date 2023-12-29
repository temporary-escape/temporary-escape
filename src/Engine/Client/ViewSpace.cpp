#include "ViewSpace.hpp"
#include "../Gui/GuiManager.hpp"
#include "../Gui/Windows/GuiWindowShipToolbar.hpp"
#include "Client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ViewSpace::ViewSpace(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager, AssetsManager& assetsManager,
                     VoxelShapeCache& voxelShapeCache, FontFamily& font, Client& client) :
    config{config},
    vulkan{vulkan},
    guiManager{guiManager},
    assetsManager{assetsManager},
    voxelShapeCache{voxelShapeCache},
    font{font},
    client{client} {

    gui.toolbar = guiManager.addWindow<GuiWindowShipToolbar>(assetsManager);
}

void ViewSpace::update(const float deltaTime, const Vector2i& viewport) {
    gui.toolbar->updatePos(viewport);

    /*if (client.getCache().playerEntityId && control.update) {
        control.update = false;
        MessageControlMovementEvent msg{};
        msg.boost = control.boost;

        if (control.forward && !control.backwards) {
            msg.speed = 100.0f;
        } else if (!control.forward && control.backwards) {
            msg.speed = -20.0f;
        } else {
            msg.speed = 0.0f;
        }

        if (control.left && !control.right) {
            msg.leftRight = -1;
        } else if (!control.left && control.right) {
            msg.leftRight = 1;
        } else {
            msg.leftRight = 0;
        }

        if (control.up && !control.down) {
            msg.upDown = 1;
        } else if (!control.up && control.down) {
            msg.upDown = -1;
        } else {
            msg.upDown = 0;
        }

        client.send(msg);
    }*/
}

void ViewSpace::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    auto scene = client.getScene();
    auto camera = scene ? scene->getPrimaryCamera() : nullptr;
    if (scene && camera) {
        renderCanvasSelectedEntity(canvas, *scene, *camera);
    }
}

void ViewSpace::renderCanvasSelectedEntity(Canvas& canvas, const Scene& scene, const ComponentCamera& camera) {
    if (!client.getCache().playerEntityId) {
        return;
    }

    const auto* playerEntityTransform = client.getCache().playerEntityId.tryGetComponent<ComponentTransform>();
    if (!playerEntityTransform) {
        return;
    }

    const auto selectedEntity = scene.getSelectedEntity();
    if (selectedEntity) {
        const auto& transform = selectedEntity->getComponent<ComponentTransform>();
        const auto worldPos = transform.getAbsolutePosition();
        const auto screenPos = scene.worldToScreen(worldPos);

        const auto dist = glm::distance(playerEntityTransform->getAbsolutePosition(), worldPos) / 1000.0f;
        const char* fmt;

        if (dist < 1.0f) {
            fmt = "{:.2f} km";
        } else if (dist < 10.0f) {
            fmt = "{:.1f} km";
        } else {
            fmt = "{:.0f} km";
        }

        const auto text = fmt::format(fmt, dist);

        canvas.drawText(screenPos + Vector2{20.0f, 0.0f}, text, font, config.guiFontSize, Color4{1.0f});

        const auto* label = selectedEntity->tryGetComponent<ComponentLabel>();
        if (label) {
            canvas.drawText(screenPos + Vector2{20.0f, config.guiFontSize},
                            label->getLabel(),
                            font,
                            config.guiFontSize,
                            Color4{1.0f});
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
    // guiContextMenu.setEnabled(false);
    gui.toolbar->setEnabled(true);
}

void ViewSpace::onExit() {
    gui.toolbar->setEnabled(false);
    // guiContextMenu.setEnabled(false);
}

void ViewSpace::doTargetEntity(const Entity& entity) {
    if (client.getCache().playerEntityId) {
        const auto* remoteHandle = entity.tryGetComponent<ComponentRemoteHandle>();
        if (remoteHandle) {
            if (const auto remoteId = remoteHandle->getRemoteId(); remoteId != ComponentTransform::NullParentId) {
                MessageControlTargetEvent msg{};
                msg.entityId = remoteId;

                client.send(msg);
            } else {
                logger.warn("Can not target entity, invalid remote handle");
            }
        } else {
            logger.warn("Can not target entity, no remote handle");
        }
    }
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

        // guiContextMenu.setEnabled(false);
    }
}

void ViewSpace::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseReleased(pos, button);

        const auto& camera = *scene->getPrimaryCamera();
        if (const auto selected = scene->getSelectedEntity();
            selected.has_value() && button == MouseButton::Right && !camera.isPanning()) {

            const auto* transform = selected->tryGetComponent<ComponentTransform>();
            if (transform) {
                guiManager.clearContextMenu();
                guiManager.addContextMenuItem("Approach", []() {});
                guiManager.addContextMenuItem("Info", []() {});
                guiManager.addContextMenuItem("Attach", []() {});
                guiManager.showContextMenu(pos);
            }
        } else if (guiManager.isContextMenuVisible()) {
            guiManager.hideContextMenu();
        }
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

        /*if (key == Key::LetterW) {
            control.forward = true;
            control.update = true;
        } else if (key == Key::LetterS) {
            control.backwards = true;
            control.update = true;
        } else if (key == Key::LetterA) {
            control.left = true;
            control.update = true;
        } else if (key == Key::LetterD) {
            control.right = true;
            control.update = true;
        } else if (key == Key::SpaceBar) {
            control.up = true;
            control.update = true;
        } else if (key == Key::LeftControl) {
            control.down = true;
            control.update = true;
        } else if (key == Key::LeftShift) {
            control.boost = true;
            control.update = true;
        }*/
    }
}

void ViewSpace::eventKeyReleased(const Key key, const Modifiers modifiers) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyReleased(key, modifiers);

        /*if (key == Key::LetterW) {
            control.forward = false;
            control.update = true;
        } else if (key == Key::LetterS) {
            control.backwards = false;
            control.update = true;
        } else if (key == Key::LetterA) {
            control.left = false;
            control.update = true;
        } else if (key == Key::LetterD) {
            control.right = false;
            control.update = true;
        } else if (key == Key::SpaceBar) {
            control.up = false;
            control.update = true;
        } else if (key == Key::LeftControl) {
            control.down = false;
            control.update = true;
        } else if (key == Key::LeftShift) {
            control.boost = false;
            control.update = true;
        }*/
    }
}

void ViewSpace::eventCharTyped(const uint32_t code) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventCharTyped(code);
    }
}

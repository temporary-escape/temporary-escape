#include "ViewSpace.hpp"
#include "../Gui/GuiManager.hpp"
#include "../Gui/Windows/GuiWindowCurrentLocation.hpp"
#include "../Gui/Windows/GuiWindowSceneOverview.hpp"
#include "../Gui/Windows/GuiWindowShipStatus.hpp"
#include "../Gui/Windows/GuiWindowShipToolbar.hpp"
#include "Client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const std::vector<std::tuple<std::string, float>> actionDistances = {
    {"500 m", 500.0f},
    {"1 Km", 1 * 1000.0f},
    {"2 Km", 2 * 1000.0f},
    {"5 Km", 5 * 1000.0f},
    {"10 Km", 10 * 1000.0f},
    {"20 Km", 20 * 1000.0f},
    {"50 Km", 50 * 1000.0f},
    {"100 Km", 100 * 1000.0f},
};

static std::string formatDistance(float dist) {
    dist = dist / 1000.0f;

    const char* fmt;

    if (dist < 1.0f) {
        fmt = "{:.2f} km";
    } else if (dist < 10.0f) {
        fmt = "{:.1f} km";
    } else {
        fmt = "{:.0f} km";
    }

    return fmt::format(fmt, dist);
}

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
    gui.status = guiManager.addWindow<GuiWindowShipStatus>(assetsManager);
    gui.location = guiManager.addWindow<GuiWindowCurrentLocation>(assetsManager);
    gui.overview = guiManager.addWindow<GuiWindowSceneOverview>();

    icons.nested = assetsManager.getImages().find("icon_menu_nested");
    icons.approach = assetsManager.getImages().find("icon_transform");
    icons.orbit = assetsManager.getImages().find("icon_orbit");
    icons.distance = assetsManager.getImages().find("icon_distance");
    icons.info = assetsManager.getImages().find("icon_info");
    icons.attack = assetsManager.getImages().find("icon_convergence_target");
}

void ViewSpace::update(const float deltaTime, const Vector2i& viewport) {
    gui.toolbar->updatePos(viewport);
    gui.status->updatePos(viewport);
    gui.location->updatePos(viewport);
    gui.overview->updatePos(viewport);

    updateGuiCurrentLocation();
}

void ViewSpace::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
    auto scene = client.getScene();
    auto camera = scene ? scene->getPrimaryCamera() : nullptr;
    if (scene && camera) {
        renderCanvasSelectedEntity(canvas, viewport, *scene, *camera);
        renderCanvasApproaching(canvas, viewport, *scene, *camera);
    }
}

void ViewSpace::updateGuiCurrentLocation() {
    const auto& cache = client.getCache();
    if (cache.system) {
        const auto& region = cache.galaxy.regions.find(cache.system->regionId);
        gui.location->setSystemLabel(fmt::format(
            "{} ({})", cache.system->name, region != cache.galaxy.regions.end() ? region->second.name : ""));
    } else {
        gui.location->setSystemLabel("");
    }
    gui.location->setSectorLabel(cache.sector ? cache.sector->name : "");
}

void ViewSpace::renderCanvasApproaching(Canvas& canvas, const Vector2i& viewport, const Scene& scene,
                                        const ComponentCamera& camera) {
    const auto& cache = client.getCache();
    const auto playerEntity = cache.player.entity;

    if (playerEntity == NullEntity) {
        return;
    }

    const auto mid = Vector2{viewport.x / 2.0f, viewport.y - 120.0f};

    if (cache.player.autopilotAction != ShipAutopilotAction::Idle &&
        cache.player.autopilotAction != ShipAutopilotAction::CancellingRotation) {
        { // What is the ship doing?
            std::string keyword = "Idle";

            if (cache.player.autopilotAction == ShipAutopilotAction::Approach) {
                keyword = "Approach";
            } else if (cache.player.autopilotAction == ShipAutopilotAction::KeepDistance) {
                keyword = fmt::format("Keep at distance of {}", formatDistance(cache.player.keepAtDistance));
            }

            const auto bounds = font.getBounds(keyword, config.guiFontSize);
            const auto pos = mid - bounds / 2.0f;
            canvas.drawText(pos, keyword, font, config.guiFontSize, Colors::overlayText);
        }

        if (cache.player.approaching != NullEntity) { // Entity Name
            const auto entityLabel = scene.tryGetComponent<ComponentLabel>(cache.player.approaching);
            std::string_view label = "";
            if (entityLabel) {
                label = entityLabel->getLabel();
            }

            const auto distance = scene.getEntityDistance(cache.player.entity, cache.player.approaching);
            const auto text = fmt::format("{} - {}", label, formatDistance(distance));
            const auto bounds = font.getBounds(text, config.guiFontSize);
            const auto pos = mid - bounds / 2.0f + Vector2{0.0f, config.guiFontSize};
            canvas.drawText(pos, text, font, config.guiFontSize, Colors::overlayText);
        }

        { // Our speed
            const auto text = fmt::format("{:.0f} m/s", cache.player.forwardVelocity);
            const auto bounds = font.getBounds(text, config.guiFontSize);
            const auto pos = mid - bounds / 2.0f + Vector2{0.0f, config.guiFontSize} * 2.0f;
            canvas.drawText(pos, text, font, config.guiFontSize, Colors::overlayText);
        }
    }
}

void ViewSpace::renderCanvasSelectedEntity(Canvas& canvas, const Vector2i& viewport, const Scene& scene,
                                           const ComponentCamera& camera) {
    const auto playerEntity = client.getCache().player.entity;

    if (playerEntity == NullEntity) {
        return;
    }

    const auto selectedEntity = scene.getSelectedEntity();
    if (selectedEntity) {
        const auto& transform = selectedEntity->getComponent<ComponentTransform>();
        const auto worldPos = transform.getAbsoluteInterpolatedPosition();
        const auto screenPos = scene.worldToScreen(worldPos);

        const auto dist = scene.getEntityDistance(playerEntity, selectedEntity->getHandle());
        const auto text = formatDistance(dist);

        canvas.drawText(screenPos + Vector2{20.0f, 0.0f}, text, font, config.guiFontSize, Colors::text);

        const auto* label = selectedEntity->tryGetComponent<ComponentLabel>();
        if (label) {
            canvas.drawText(screenPos + Vector2{20.0f, config.guiFontSize},
                            label->getLabel(),
                            font,
                            config.guiFontSize,
                            Colors::text);
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
    gui.status->setEnabled(true);
    gui.location->setEnabled(true);
    gui.overview->setEnabled(true);
}

void ViewSpace::onExit() {
    gui.toolbar->setEnabled(false);
    gui.status->setEnabled(false);
    gui.location->setEnabled(false);
    gui.overview->setEnabled(false);
    // guiContextMenu.setEnabled(false);
}

void ViewSpace::showContextMenu(const Vector2i& mousePos) {
    guiManager.showContextMenu(mousePos, [=](GuiWindowContextMenu& menu) {
        menu.addItem(icons.approach, "Go Here", [=]() {
            auto& scene = *client.getScene();
            const auto [_, pos] = scene.screenToWorld(mousePos, 10000.0f);

            const auto playerEntity = client.getCache().player.entity;
            if (scene.valid(playerEntity)) {
                const auto* transform = scene.tryGetComponent<ComponentTransform>(playerEntity);
                if (transform) {
                    const auto direction = glm::normalize(pos - transform->getAbsolutePosition());
                    doGoHere(direction);
                }
            }
        });
    });
}

void ViewSpace::showContextMenu(const Vector2i& mousePos, const Entity& entity) {
    const auto* transform = entity.tryGetComponent<ComponentTransform>();
    if (!transform) {
        return;
    }

    guiManager.showContextMenu(mousePos, [=](GuiWindowContextMenu& menu) {
        menu.addItem(icons.info, "Info", []() {});
        menu.addItem(icons.approach, "Approach", [=]() { doApproachEntity(entity); });
        menu.addNested(icons.orbit, icons.nested, "Orbit", [=](GuiWindowContextMenu& menuOrbit) {
            for (const auto& [label, distance] : actionDistances) {
                menuOrbit.addItem(nullptr, label, [=, d = distance]() { doOrbitEntity(entity, d); });
            }
        });
        menu.addNested(icons.distance, icons.nested, "Distance", [=](GuiWindowContextMenu& menuOrbit) {
            for (const auto& [label, distance] : actionDistances) {
                menuOrbit.addItem(nullptr, label, [=, d = distance]() { doKeepAtDistanceEntity(entity, d); });
            }
        });
        menu.addItem(icons.attack, "Target", []() {});
    });
}

void ViewSpace::doTargetEntity(const Entity& entity) {
    /*if (client.getCache().playerEntityId) {
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
    }*/
}

void ViewSpace::doGoHere(const Vector3& direction) {
    auto& scene = *client.getScene();
    MessageActionGoDirection msg{};
    msg.direction = direction;
    client.send(msg);
}

void ViewSpace::doApproachEntity(const Engine::Entity& entity) {
    auto& scene = *client.getScene();
    MessageActionApproach msg{};
    msg.entityId = scene.getRemoteId(entity.getHandle());
    client.send(msg);
}

void ViewSpace::doOrbitEntity(const Entity& entity, const float radius) {
    auto& scene = *client.getScene();
    MessageActionOrbit msg{};
    msg.entityId = scene.getRemoteId(entity.getHandle());
    msg.radius = radius;
    client.send(msg);
}

void ViewSpace::doKeepAtDistanceEntity(const Entity& entity, const float distance) {
    auto& scene = *client.getScene();
    MessageActionKeepDistance msg{};
    msg.entityId = scene.getRemoteId(entity.getHandle());
    msg.distance = distance;
    client.send(msg);
}

void ViewSpace::doStopMovement() {
    MessageActionStopMovement msg{};
    client.send(msg);
}

void ViewSpace::eventMouseMoved(const Vector2i& pos) {
    auto scene = client.getScene();
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSpace::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (guiManager.isContextMenuVisible()) {
        guiManager.getContextMenu().setEnabled(false);
    }

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
        const auto selected = scene->getSelectedEntity();
        if (button == MouseButton::Right && !camera.isPanning()) {
            if (selected.has_value()) {
                showContextMenu(pos, selected.value());
            } else {
                showContextMenu(pos);
            }
        } else if (guiManager.isContextMenuVisible()) {
            guiManager.getContextMenu().setEnabled(false);
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
    if (key == Key::Escape && guiManager.isContextMenuVisible()) {
        guiManager.getContextMenu().setEnabled(false);
    }

    auto scene = client.getScene();
    if (scene) {
        scene->eventKeyPressed(key, modifiers);

        if (key == Key::LetterS) {
            doStopMovement();
        }

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

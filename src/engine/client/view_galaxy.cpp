#include "view_galaxy.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/theme.hpp"
#include "client.hpp"

#define CMP "ViewGalaxy"

using namespace Engine;

static const Vector2 systemStarSelectable{32.0f, 32.0f};
static const Vector2 systemStarSize{24.0f, 24.0f};

ViewGalaxy::ViewGalaxy(const Config& config, Renderer& renderer, Registry& registry, Client& client) :
    config{config},
    renderer{renderer},
    registry{registry},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.02f, 0.02f, 0.02f, 1.0f}},
    scene{} {

    textures.systemStar = registry.getTextures().find("star_flare");
    images.iconSelect = registry.getImages().find("icon_target");

    // To keep the renderer away from complaining
    {
        auto entity = scene.createEntity();
        entity->addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        entity->addComponent<ComponentTransform>().translate(Vector3{0.0f, 1.0f, 0.0f});
    }

    // Our primary camera
    {
        auto entity = scene.createEntity();
        auto& transform = entity->addComponent<ComponentTransform>();
        auto& camera = entity->addComponent<ComponentCamera>(transform);
        entity->addComponent<ComponentUserInput>(camera);
        camera.setOrthographic(25.0f);
        camera.lookAt({0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
        camera.setZoomRange(3.0f, 250.0f);
        scene.setPrimaryCamera(entity);

        entities.camera = entity;
    }

    gui.contextMenu.setEnabled(false);
}

void ViewGalaxy::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewGalaxy::render(const Vector2i& viewport) {
    Renderer::Options options{};
    options.bloomEnabled = false;
    renderer.render(viewport, scene, skybox, options);
}

void ViewGalaxy::renderCanvas(const Vector2i& viewport) {
    /*if (input.hover != nullptr) {
        auto pos = camera->worldToScreen(Vector3{input.hover->pos.x, 0.0f, input.hover->pos.y}, true);
        canvas.color(Theme::primary);
        canvas.rectOutline(pos - systemStarSelectable / 2.0f, systemStarSelectable, 1.0f);
    }*/
}

void ViewGalaxy::renderGui(const Vector2i& viewport) {
    /*gui.modalLoading.setEnabled(loading);
    gui.modalLoading.setProgress(loadingValue);

    nuklear.draw(gui.modalLoading);
    nuklear.draw(gui.contextMenu);*/
}

void ViewGalaxy::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    if (!gui.contextMenu.isEnabled()) {
        auto& camera = entities.camera->getComponent<ComponentCamera>();
        // input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera.getViewport().y) - pos.y});
    }
}

void ViewGalaxy::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
    gui.oldMousePos = pos;

    if (button == MouseButton::Right && gui.contextMenu.isEnabled()) {
        gui.contextMenu.setEnabled(false);
        auto& camera = entities.camera->getComponent<ComponentCamera>();
        // input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera.getViewport().y) - pos.y});
    }
}

void ViewGalaxy::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);
}

void ViewGalaxy::eventMouseScroll(const int xscroll, const int yscroll) {
    scene.eventMouseScroll(xscroll, yscroll);
}

void ViewGalaxy::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene.eventKeyPressed(key, modifiers);
}

void ViewGalaxy::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene.eventKeyReleased(key, modifiers);
}

void ViewGalaxy::eventCharTyped(const uint32_t code) {
    scene.eventCharTyped(code);
}

void ViewGalaxy::load() {
    loading = true;
    loadingValue = 0.1f;

    // Reset entities
    clearEntities();

    // Reset gui
    gui.contextMenu.setEnabled(false);

    // Cancel previous load sequence
    stopToken.stop();

    stopToken = StopToken{};
    fetchCurrentLocation(stopToken);
}

void ViewGalaxy::fetchCurrentLocation(const StopToken& stop) {
    MessagePlayerLocationRequest req{};

    client.send(req, [this, stop](MessagePlayerLocationResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        Log::d(CMP, "Received player location info");

        location.galaxyId = res.galaxyId;
        location.systemId = res.systemId;
        location.sectorId = res.sectorId;

        loadingValue = 0.2f;

        fetchGalaxyInfo(stop);
    });
}

void ViewGalaxy::fetchGalaxyInfo(const StopToken& stop) {
    MessageFetchGalaxyRequest req{};
    req.galaxyId = location.galaxyId;

    client.send(req, [this, stop](MessageFetchGalaxyResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        Log::d(CMP, "Received galaxy info for: '{}'", location.galaxyId);

        galaxy.name = res.name;
        galaxy.systems.clear();
        galaxy.regions.clear();
        factions.clear();

        loadingValue = 0.3f;

        fetchFactionsPage(stop, "");
    });
}

void ViewGalaxy::fetchFactionsPage(const StopToken& stop, const std::string& token) {
    MessageFetchFactionsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this, stop](MessageFetchFactionsResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& faction : res.factions) {
            factions[faction.id] = faction;
        }

        if (res.hasNext) {
            fetchRegionsPage(stop, res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} factions", factions.size());
            loadingValue = 0.5f;
            fetchRegionsPage(stop, "");
        }
    });
}

void ViewGalaxy::fetchRegionsPage(const StopToken& stop, const std::string& token) {
    MessageFetchRegionsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this, stop](MessageFetchRegionsResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& region : res.regions) {
            galaxy.regions[region.id] = region;
        }

        if (res.hasNext) {
            fetchRegionsPage(stop, res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} regions", galaxy.regions.size());
            loadingValue = 0.5f;
            fetchSystemsPage(stop, "");
        }
    });
}

void ViewGalaxy::fetchSystemsPage(const StopToken& stop, const std::string& token) {
    MessageFetchSystemsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this, stop](MessageFetchSystemsResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& system : res.systems) {
            galaxy.systems[system.id] = system;
        }

        if (res.hasNext) {
            fetchSystemsPage(stop, res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} systems", galaxy.systems.size());
            loadingValue = 0.7f;
            updateGalaxy();
        }
    });
}

void ViewGalaxy::updateGalaxy() {
    Log::i(CMP, "Recreating galaxy objects with {} systems", galaxy.systems.size());
    loading = false;
    loadingValue = 1.0f;

    clearEntities();
    createEntitiesRegions();
}

void ViewGalaxy::clearEntities() {
    if (!entities.regions.empty()) {
        for (const auto& [_, entity] : entities.regions) {
            scene.removeEntity(entity);
        }
        entities.regions.clear();
    }
    if (entities.positions) {
        scene.removeEntity(entities.positions);
        entities.positions.reset();
    }
    if (entities.cursor) {
        scene.removeEntity(entities.cursor);
        entities.cursor.reset();
    }
}

void ViewGalaxy::createEntitiesRegions() {
    std::unordered_map<std::string, ComponentPointCloud*> pointCloudMap;
    std::unordered_map<std::string, ComponentLines*> linesMap;

    entities.positions = scene.createEntity();
    entities.positions->addComponent<ComponentTransform>();
    auto& clickable = entities.positions->addComponent<ComponentClickablePoints>();
    entities.positions->addComponent<ComponentUserInput>(clickable);

    clickable.setOnHoverCallback([this](size_t i) {
        const auto& system = input.systemsOrdered[i];
        const auto systemPos = Vector3{system->pos.x, 0.0f, system->pos.y};
        entities.cursor->getComponent<ComponentTransform>().move(systemPos);
        entities.cursor->setDisabled(false);
    });

    clickable.setOnBlurCallback([this]() {
        entities.cursor->getComponent<ComponentTransform>().move({0.0f, 0.0f, 0.0f});
        entities.cursor->setDisabled(true);
    });

    entities.cursor = scene.createEntity();
    entities.cursor->setDisabled(true);
    entities.cursor->addComponent<ComponentTransform>();
    entities.cursor->addComponent<ComponentIcon>(images.iconSelect, Vector2{32.0f, 32.0f}, Theme::primary);

    for (const auto& [regionId, _] : galaxy.regions) {
        auto entity = scene.createEntity();
        entity->addComponent<ComponentTransform>();
        auto& pointCloud = entity->addComponent<ComponentPointCloud>(textures.systemStar);
        auto& lines = entity->addComponent<ComponentLines>();

        pointCloudMap[regionId] = &pointCloud;
        linesMap[regionId] = &lines;

        entities.regions[regionId] = entity;
    }

    input.systemsOrdered.clear();
    input.systemsOrdered.reserve(galaxy.systems.size());

    for (const auto& [systemId, system] : galaxy.systems) {
        auto starColor = Color4{0.8f, 0.8f, 0.8f, 1.0f};
        auto connectionColor = Color4{0.7f, 0.7f, 0.7f, 0.2f};

        clickable.add(Vector3{system.pos.x, 0.0f, system.pos.y});
        input.systemsOrdered.push_back(&system);

        const auto region = galaxy.regions.find(system.regionId);
        if (region == galaxy.regions.end()) {
            EXCEPTION("No such region: '{}' for system: '{}'", system.regionId);
        }

        if (system.factionId) {
            const auto faction = factions.find(*system.factionId);
            if (faction == factions.end()) {
                EXCEPTION("No such faction: '{}' for system: '{}'", *system.factionId);
            }

            starColor = hsvToRgb(Color4{faction->second.color, 0.6f, 1.0f, 1.0f});
        }

        auto* pointCloud = pointCloudMap.at(region->second.id);
        pointCloud->add(Vector3{system.pos.x, 0.0f, system.pos.y}, systemStarSize, starColor);

        for (const auto& otherId : system.connections) {
            const auto other = galaxy.systems.find(otherId);
            if (other == galaxy.systems.end()) {
                EXCEPTION("No such connection found: '{}' for system: '{}'", otherId, system.id);
            }

            auto* lines = linesMap.at(region->second.id);
            lines->add(system.pos, other->second.pos, connectionColor);
        }
    }
}

void ViewGalaxy::onEnter() {
}

void ViewGalaxy::onExit() {
}

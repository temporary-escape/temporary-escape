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
    skybox{renderer.getVulkan(), Color4{0.1f, 0.1f, 0.1f, 1.0f}},
    scene{&registry.getVoxelShapeCache()} {

    textures.systemStar = registry.getTextures().find("star_flare");

    // To keep the renderer away from complaining
    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
    sun->translate(Vector3{0.0f, 1.0f, 0.0f});
    scene.addEntity(sun);

    // Our primary camera
    auto cameraEntity = std::make_shared<Entity>();
    camera = cameraEntity->addComponent<ComponentCamera>();
    cameraEntity->addComponent<ComponentUserInput>(*camera);
    camera->setOrthographic(25.0f);
    camera->lookAt({0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    camera->setZoomRange(3.0f, 250.0f);
    scene.addEntity(cameraEntity);
    scene.setPrimaryCamera(cameraEntity);

    gui.contextMenu.setEnabled(false);
}

void ViewGalaxy::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewGalaxy::render(const Vector2i& viewport) {
    /*Renderer::Options options{};
    options.blurStrength = 0.0f;
    options.exposure = 1.0f;
    renderer.render(viewport, scene, skybox, options);*/
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
        input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera->getViewport().y) - pos.y});
    }
}

void ViewGalaxy::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
    gui.oldMousePos = pos;

    if (button == MouseButton::Right && gui.contextMenu.isEnabled()) {
        gui.contextMenu.setEnabled(false);
        input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera->getViewport().y) - pos.y});
    }
}

void ViewGalaxy::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);

    if (button == MouseButton::Right && input.hover && !gui.contextMenu.isEnabled() && gui.oldMousePos == pos) {
        gui.contextMenu.setPos(camera->worldToScreen({input.hover->pos.x, 0.0f, input.hover->pos.y}, true));
        gui.contextMenu.setEnabled(true);
        gui.contextMenu.setItems({
            {"Travel to", [this]() { gui.contextMenu.setEnabled(false); }},
            {"View", [this]() { gui.contextMenu.setEnabled(false); }},
            {"Info", [this]() { gui.contextMenu.setEnabled(false); }},
            {"Add notes", [this]() { gui.contextMenu.setEnabled(false); }},
        });
    }
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
    input.hover = nullptr;
    input.selected = nullptr;

    // Reset entities
    clearInputIndices();
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
    createInputIndices();
}

void ViewGalaxy::clearEntities() {
    if (!entities.regions.empty()) {
        for (const auto& [_, entity] : entities.regions) {
            scene.removeEntity(entity);
        }
        entities.regions.clear();
    }
}

void ViewGalaxy::createEntitiesRegions() {
    std::unordered_map<std::string, ComponentPointCloud*> pointCloudMap;
    std::unordered_map<std::string, ComponentLines*> linesMap;

    for (const auto& [regionId, _] : galaxy.regions) {
        auto entity = std::make_shared<Entity>();
        auto pointCloud = entity->addComponent<ComponentPointCloud>(textures.systemStar).get();
        auto lines = entity->addComponent<ComponentLines>().get();

        pointCloud->setRenderOrder(0);
        lines->setRenderOrder(1);

        pointCloudMap[regionId] = pointCloud;
        linesMap[regionId] = lines;

        entities.regions[regionId] = entity;
        scene.addEntity(entity);
    }

    for (const auto& [systemId, system] : galaxy.systems) {
        auto starColor = Color4{0.8f, 0.8f, 0.8f, 1.0f};
        auto connectionColor = Color4{0.7f, 0.7f, 0.7f, 0.1f};

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

void ViewGalaxy::createInputIndices() {
    clearInputIndices();

    input.indices.reserve(galaxy.systems.size());
    input.positions.reserve(galaxy.systems.size());

    for (const auto& [_, system] : galaxy.systems) {
        input.indices.push_back(&system);
        input.positions.emplace_back(system.pos.x, 0.0f, system.pos.y);
    }
}

void ViewGalaxy::clearInputIndices() {
    input.indices.clear();
    input.positions.clear();
}

const SystemData* ViewGalaxy::rayCast(const Vector2& mousePos) {
    std::vector<std::tuple<float, const SystemData*>> found;

    const auto positions = camera->worldToScreen(input.positions);

    for (size_t i = 0; i < positions.size(); i++) {
        const auto& pos = positions.at(i);

        if (pos.x - systemStarSelectable.x <= mousePos.x && pos.x + systemStarSelectable.x >= mousePos.x &&
            pos.y - systemStarSelectable.y <= mousePos.y && pos.y + systemStarSelectable.y >= mousePos.y) {

            const auto& system = input.indices.at(i);
            found.emplace_back(glm::distance(pos, mousePos), system);
        }
    }

    if (found.empty()) {
        return nullptr;
    }

    std::sort(found.begin(), found.end(),
              [](const auto& a, const auto& b) -> bool { return std::get<0>(a) < std::get<0>(b); });

    return std::get<1>(found.front());
}

void ViewGalaxy::onEnter() {
}

void ViewGalaxy::onExit() {
}

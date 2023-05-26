#include "view_galaxy.hpp"
#include "../graphics/theme.hpp"
#include "../math/convex_hull.hpp"
#include "client.hpp"
#include "game.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const Vector2 systemStarSelectable{32.0f, 32.0f};
static const Vector2 systemStarSize{32.0f, 32.0f};

ViewGalaxy::ViewGalaxy(Game& parent, const Config& config, Renderer& renderer, AssetsManager& assetsManager,
                       Client& client, FontFamily& font) :
    parent{parent}, config{config}, assetsManager{assetsManager}, client{client}, font{font}, scene{} {

    textures.systemStar = assetsManager.getTextures().find("star_flare");
    images.iconSelect = assetsManager.getImages().find("icon_target");

    { // To keep the renderer away from complaining
        auto entity = scene.createEntity();
        entity->addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        entity->addComponent<ComponentTransform>().translate(Vector3{0.0f, 1.0f, 0.0f});
    }

    { // Skybox
        auto entity = scene.createEntity();
        auto& skybox = entity->addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{renderer.getVulkan(), Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(renderer.getVulkan(), std::move(skyboxTextures));
    }

    { // Our primary camera
        auto entity = scene.createEntity();
        auto& transform = entity->addComponent<ComponentTransform>();
        auto& camera = entity->addComponent<ComponentCamera>(transform);
        entity->addComponent<ComponentUserInput>(camera);
        camera.setOrthographic(25.0f);
        camera.lookAt({0.0f, 10.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
        camera.setZoomRange(3.0f, 250.0f);
        scene.setPrimaryCamera(entity);

        entities.camera = entity;
    }
}

void ViewGalaxy::update(const float deltaTime) {
    scene.update(deltaTime);

    if (futureVoronoi.valid() && futureVoronoi.ready()) {
        // createBackground(futureVoronoi.get());
    }

    const auto* camera = scene.getPrimaryCamera();

    // Update connections alpha color
    if (!entities.regions.empty()) {
        const auto f = 1.0f - glm::clamp(map(camera->getOrthoScale(), 10.0f, 70.0f, 0.0f, 0.8f), 0.0f, 0.8f);
        for (auto& [_, entity] : entities.regions) {
            entity->getComponent<ComponentLines>().setColor(alpha(f));
        }
    }

    // Update region labels alpha color
    if (!entities.labels.empty()) {
        const auto f = glm::clamp(map(camera->getOrthoScale(), 10.0f, 70.0f, 0.1f, 0.5f), 0.1f, 0.5f);
        for (auto& [regionId, entity] : entities.labels) {
            entity->getComponent<ComponentText>().setColor(Theme::text * alpha(f));
        }
    }

    // Update system names
    if (entities.names) {
        const auto f = 1.0f - glm::clamp(map(camera->getOrthoScale(), 5.0f, 30.0f, 0.5f, 1.0f), 0.5f, 1.0f);
        entities.names->getComponent<ComponentWorldText>().setColor(Theme::text * alpha(f));
    }
}

void ViewGalaxy::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);
}

void ViewGalaxy::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
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

Scene& ViewGalaxy::getScene() {
    return scene;
}

void ViewGalaxy::load() {
    loading = true;
    loadingValue = 0.1f;

    // Reset entities
    // clearEntities();

    // Cancel previous load sequence
    // stopToken.stop();

    // stopToken = StopToken{};
    // fetchCurrentLocation(stopToken);
}

/*void ViewGalaxy::fetchCurrentLocation(const StopToken& stop) {
    MessagePlayerLocationRequest req{};

    client.send(req, [this, stop](MessagePlayerLocationResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        logger.debug("Received player location info");

        location.galaxyId = res.galaxyId;
        location.systemId = res.systemId;
        location.sectorId = res.sectorId;

        loadingValue = 0.2f;

        fetchGalaxyInfo(stop);
    });
}*/

/*void ViewGalaxy::fetchGalaxyInfo(const StopToken& stop) {
    MessageFetchGalaxyRequest req{};
    req.galaxyId = location.galaxyId;

    client.send(req, [this, stop](MessageFetchGalaxyResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        logger.debug("Received galaxy info for: '{}'", location.galaxyId);

        galaxy.name = res.name;
        galaxy.systems.clear();
        galaxy.regions.clear();
        factions.clear();

        loadingValue = 0.3f;

        fetchFactionsPage(stop, "");
    });
}*/

/*void ViewGalaxy::fetchFactionsPage(const StopToken& stop, const std::string& token) {
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
            logger.info("Received galaxy with {} factions", factions.size());
            loadingValue = 0.5f;
            fetchRegionsPage(stop, "");
        }
    });
}*/

/*void ViewGalaxy::fetchRegionsPage(const StopToken& stop, const std::string& token) {
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
            logger.info("Received galaxy with {} regions", galaxy.regions.size());
            loadingValue = 0.5f;
            fetchSystemsPage(stop, "");
        }
    });
}*/

/*void ViewGalaxy::fetchSystemsPage(const StopToken& stop, const std::string& token) {
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
            logger.info("Received galaxy with {} systems", galaxy.systems.size());
            loadingValue = 0.7f;
            updateGalaxy();
        }
    });
}*/

/*void ViewGalaxy::updateGalaxy() {
    logger.info("Recreating galaxy objects with {} systems", galaxy.systems.size());
    loading = false;
    loadingValue = 1.0f;

    clearEntities();
    createEntitiesRegions();
}*/

/*void ViewGalaxy::clearEntities() {
    for (const auto& [_, entity] : entities.regions) {
        scene.removeEntity(entity);
    }
    entities.regions.clear();
    for (const auto& [_, entity] : entities.labels) {
        scene.removeEntity(entity);
    }
    entities.labels.clear();
    for (const auto& [_, entity] : entities.factions) {
        scene.removeEntity(entity);
    }
    entities.factions.clear();
    if (entities.positions) {
        scene.removeEntity(entities.positions);
        entities.positions.reset();
    }
    if (entities.cursor) {
        scene.removeEntity(entities.cursor);
        entities.cursor.reset();
    }
    if (entities.voronoi) {
        scene.removeEntity(entities.voronoi);
        entities.voronoi.reset();
    }
    if (entities.names) {
        scene.removeEntity(entities.names);
        entities.names.reset();
    }
}*/

/*void ViewGalaxy::createEntitiesRegions() {
    std::unordered_map<std::string, ComponentPointCloud*> pointCloudMap;
    std::unordered_map<std::string, ComponentLines*> linesMap;

    entities.positions = scene.createEntity();
    entities.positions->addComponent<ComponentTransform>();
    auto& clickable = entities.positions->addComponent<ComponentClickablePoints>();
    entities.positions->addComponent<ComponentUserInput>(clickable);

    clickable.setOnHoverCallback([this](const size_t i) {
        const auto& system = galaxy.systemsOrdered[i];
        const auto systemPos = Vector3{system->pos.x, 0.1f, system->pos.y};
        entities.cursor->getComponent<ComponentTransform>().move(systemPos);
        auto& text = entities.cursor->getComponent<ComponentText>();
        text.setText(system->name);
        entities.cursor->setDisabled(false);
    });

    clickable.setOnClickCallback([this](const size_t i, const bool pressed, const MouseButton button) {
        if (pressed || scene.getPrimaryCamera()->isPanning() || button != MouseButton::Right) {
            return;
        }

        const auto& system = galaxy.systemsOrdered[i];
        const auto systemPos = Vector3{system->pos.x, 0.0f, system->pos.y};
        const auto viewPos = scene.getPrimaryCamera()->worldToScreen(systemPos, true);

        //gui.contextMenu.setEnabled(true);
        //gui.contextMenu.setPos(viewPos);
        //gui.contextMenu.setItems({
        //    {"View", [this, system]() { parent.switchToSystemMap(system->galaxyId, system->id); }},
        //    {"Info", [this]() {}},
        //    {"Note", [this]() {}},
        //    {"Set Destination", [this]() {}},
        //});
    });

    clickable.setOnBlurCallback([this]() {
        entities.cursor->getComponent<ComponentTransform>().move({0.0f, 0.1f, 0.0f});
        entities.cursor->setDisabled(true);
    });

    entities.cursor = scene.createEntity();
    entities.cursor->setDisabled(true);
    entities.cursor->addComponent<ComponentTransform>().move(Vector3{0.0f, 0.1f, 0.0f});
    entities.cursor->addComponent<ComponentIcon>(images.iconSelect, systemStarSelectable, Theme::primary);
    auto& cursorText = entities.cursor->addComponent<ComponentText>("", Theme::primary, config.guiFontSize);
    cursorText.setCentered(true);
    cursorText.setOffset(Vector2{0, -(systemStarSelectable.y / 2.0f)});

    entities.names = scene.createEntity();
    entities.names->addComponent<ComponentTransform>().move(Vector3{0.0f, 0.1f, 0.0f});
    auto& names =
        entities.names->addComponent<ComponentWorldText>(font.regular, Theme::text * alpha(0.5f), config.guiFontSize);
    names.setOffset(Vector2{0.0f, -systemStarSize.y});

    for (const auto& [regionId, region] : galaxy.regions) {
        auto entity = scene.createEntity();
        entity->addComponent<ComponentTransform>();
        auto& pointCloud = entity->addComponent<ComponentPointCloud>(textures.systemStar);
        auto& lines = entity->addComponent<ComponentLines>();

        pointCloudMap[regionId] = &pointCloud;
        linesMap[regionId] = &lines;

        entities.regions[regionId] = entity;

        entity = scene.createEntity();
        entity->addComponent<ComponentTransform>().move({region.pos.x, 0.0f, region.pos.y});
        auto& text = entity->addComponent<ComponentText>(region.name, Theme::text * alpha(0.1f), 32.0f);
        text.setCentered(true);

        entities.labels[regionId] = entity;
    }

    //for (const auto& [factionId, faction] : factions) {
    //    const auto& system = galaxy.systems[faction.homeSectorId];
    //    const auto color = hsvToRgb(Color4{faction.color, 0.6f, 1.0f, 0.1f});
//
    //    auto entity = scene.createEntity();
    //    entity->addComponent<ComponentTransform>().move({system.pos.x, 0.0f, system.pos.y});
    //    auto& text = entity->addComponent<ComponentText>(faction.name, color, 18.0f);
    //    text.setCentered(true);
//
    //    entities.labels[factionId] = entity;
    //}

    galaxy.systemsOrdered.clear();
    galaxy.systemsOrdered.reserve(galaxy.systems.size());

    for (const auto& [systemId, system] : galaxy.systems) {
        auto starColor = Color4{0.8f, 0.8f, 0.8f, 1.0f};
        auto connectionColor = Color4{0.7f, 0.7f, 0.7f, 0.3f};

        clickable.add(Vector3{system.pos.x, 0.0f, system.pos.y});
        galaxy.systemsOrdered.push_back(&system);

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

        names.add(Vector3{system.pos.x, 0.0f, system.pos.y}, system.name);
    }

    calculateBackground();
}*/

/*void ViewGalaxy::calculateBackground() {
    logger.info("Calculating background");

    std::vector<Vector2> positions;
    positions.reserve(galaxy.systemsOrdered.size());

    for (const auto* system : galaxy.systemsOrdered) {
        positions.push_back(system->pos);
    }

    auto clip = computeConvexHull(positions);
    for (auto& pos : clip) {
        const auto l = 1.0f / glm::length(pos);
        pos *= 1.0f + l * 10.0f;
    }

    futureVoronoi =
        std::async([p = std::move(positions), c = std::move(clip)]() { return computeVoronoiDiagram(p, c); });
}*/

/*void ViewGalaxy::createBackground(const VoronoiResult& voronoi) {
    logger.info("Creating background");

    if (voronoi.cells.size() != galaxy.systemsOrdered.size()) {
        EXCEPTION("Voronoi incorrect number of cells");
    }

    entities.voronoi = scene.createEntity();
    entities.voronoi->addComponent<ComponentTransform>().move(Vector3{0.0f, -0.1f, 0.0f});
    auto& polyShape = entities.voronoi->addComponent<ComponentPolyShape>();

    for (size_t i = 0; i < galaxy.systemsOrdered.size(); i++) {
        const auto& cell = voronoi.cells[i];
        const auto* system = galaxy.systemsOrdered[i];

        Color4 cellColor = hsvToRgb(Vector4{0.0f, 0.0f, 0.2f, 0.2f});

        if (system->factionId) {
            const auto& faction = factions[system->factionId.value()];
            cellColor = hsvToRgb(Vector4{faction.color, 0.6f, 0.2f, 0.2f});
        }

        for (const auto& triangle : cell) {
            polyShape.add({triangle[0].x, 0.0f, triangle[0].y}, cellColor);
            polyShape.add({triangle[1].x, 0.0f, triangle[1].y}, cellColor);
            polyShape.add({triangle[2].x, 0.0f, triangle[2].y}, cellColor);
        }
    }
}*/

void ViewGalaxy::onEnter() {
}

void ViewGalaxy::onExit() {
}

#include "view_system.hpp"
#include "../graphics/theme.hpp"
#include "../utils/overloaded.hpp"
#include "client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const Vector2 systemBodySelectable{32.0f, 32.0f};
static const Vector2 systemBodySize{32.0f, 32.0f};

ViewSystem::ViewSystem(Game& parent, const Config& config, Renderer& renderer, AssetsManager& assetsManager,
                       Client& client, FontFamily& font) :
    parent{parent}, config{config}, assetsManager{assetsManager}, client{client}, font{font}, scene{} {

    images.systemPlanet = assetsManager.getImages().find("icon_ringed_planet");
    images.systemMoon = assetsManager.getImages().find("icon_world");
    images.iconSelect = assetsManager.getImages().find("icon_target");

    { // To keep the renderer away from complaining
        auto sun = scene.createEntity();
        sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        sun->addComponent<ComponentTransform>().translate(Vector3{0.0f, 0.0f, 1.0f});
    }

    { // Skybox
        auto entity = scene.createEntity();
        auto& skybox = entity->addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{renderer.getVulkan(), Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(renderer.getVulkan(), std::move(skyboxTextures));
    }

    { // Our primary camera
        auto cameraEntity = scene.createEntity();
        auto& cameraTransform = cameraEntity->addComponent<ComponentTransform>();
        camera = &cameraEntity->addComponent<ComponentCamera>(cameraTransform);
        cameraEntity->addComponent<ComponentUserInput>(*camera);
        camera->setOrthographic(25.0f);
        camera->lookAt({0.0f, 1000.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
        camera->setZoomRange(3.0f, 250.0f);
        scene.setPrimaryCamera(cameraEntity);
    }
}

void ViewSystem::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewSystem::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);
}

void ViewSystem::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
}

void ViewSystem::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    scene.eventMouseReleased(pos, button);

    /*if (button == MouseButton::Right && input.hover && !gui.contextMenu.isEnabled() && gui.oldMousePos == pos) {
        gui.contextMenu.setPos(camera->worldToScreen({input.hover->pos.x, 0.0f, input.hover->pos.y}, true));
        gui.contextMenu.setEnabled(true);
        gui.contextMenu.setItems({
            {"Travel to", [this]() { gui.contextMenu.setEnabled(false); }},
            {"View", [this]() { gui.contextMenu.setEnabled(false); }},
            {"Info", [this]() { gui.contextMenu.setEnabled(false); }},
            {"Add notes", [this]() { gui.contextMenu.setEnabled(false); }},
        });
    }*/
}

void ViewSystem::eventMouseScroll(const int xscroll, const int yscroll) {
    scene.eventMouseScroll(xscroll, yscroll);
}

void ViewSystem::eventKeyPressed(const Key key, const Modifiers modifiers) {
    scene.eventKeyPressed(key, modifiers);
}

void ViewSystem::eventKeyReleased(const Key key, const Modifiers modifiers) {
    scene.eventKeyReleased(key, modifiers);
}

void ViewSystem::eventCharTyped(const uint32_t code) {
    scene.eventCharTyped(code);
}

Scene& ViewSystem::getScene() {
    return scene;
}

/*void ViewSystem::clear() {
    loading = true;
    loadingValue = 0.1f;
    input.hover = nullptr;
    input.selected = nullptr;

    // Reset entities
    clearEntities();

    // Cancel previous load sequence
    stopToken.stop();

    system.bodies.clear();
    system.planets.clear();
    system.sectors.clear();
}*/

void ViewSystem::load() {
    // clear();

    stopToken = StopToken{};
    // fetchCurrentLocation(stopToken);
}

void ViewSystem::load(const std::string& galaxyId, const std::string& systemId) {
    // clear();

    location.galaxyId = galaxyId;
    location.systemId = systemId;

    // stopToken = StopToken{};
    // fetchSectors(stopToken, "");
}

/*void ViewSystem::fetchCurrentLocation(const StopToken& stop) {
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

        fetchSectors(stop, "");
    });
}*/

/*void ViewSystem::fetchSectors(const StopToken& stop, const std::string& token) {
    MessageFetchSectorsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;
    req.systemId = location.systemId;

    client.send(req, [this, stop](MessageFetchSectorsResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& sector : res.sectors) {
            system.sectors[sector.id] = sector;
            system.bodies.emplace_back(&system.sectors[sector.id]);
        }

        if (res.hasNext) {
            fetchPlanetaryBodiesPage(stop, res.token);
        } else {
            logger.info("Received system with {} sector", system.sectors.size());
            loadingValue = 0.7f;
            fetchPlanetaryBodiesPage(stop, "");
        }
    });
}*/

/*void ViewSystem::fetchPlanetaryBodiesPage(const StopToken& stop, const std::string& token) {
    MessageFetchPlanetsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;
    req.systemId = location.systemId;

    client.send(req, [this, stop](MessageFetchPlanetsResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& body : res.bodies) {
            system.planets[body.id] = body;
            system.bodies.emplace_back(&system.planets[body.id]);
        }

        if (res.hasNext) {
            fetchPlanetaryBodiesPage(stop, res.token);
        } else {
            logger.info("Received system with {} planetary bodies", system.bodies.size());
            loadingValue = 0.7f;
            updateSystem();
        }
    });
}*/

/*void ViewSystem::updateSystem() {
    logger.info("Recreating system with objects");
    loading = false;
    loadingValue = 1.0f;

    clearEntities();
    createEntityCursor();
    createEntityPositions();
    createEntitiesBodies();
}*/

/*void ViewSystem::clearEntities() {
    for (auto& entity : entities.bodies) {
        scene.removeEntity(entity);
    }
    entities.bodies.clear();

    for (auto& entity : entities.orbits) {
        scene.removeEntity(entity);
    }
    entities.orbits.clear();

    scene.removeEntity(entities.names);
    entities.names.reset();

    scene.removeEntity(entities.positions);
    entities.positions.reset();

    scene.removeEntity(entities.icons);
    entities.icons.reset();

    scene.removeEntity(entities.cursor);
    entities.cursor.reset();
}*/

/*void ViewSystem::createEntityPositions() {
    entities.positions = scene.createEntity();
    entities.positions->addComponent<ComponentTransform>();
    auto& clickable = entities.positions->addComponent<ComponentClickablePoints>();
    entities.positions->addComponent<ComponentUserInput>(clickable);

    clickable.setOnHoverCallback([this](const size_t i) {
        auto& body = system.bodies[i];

        std::visit(overloaded{
                       [&](PlanetData* planet) {
                           const auto pos = Vector3{planet->pos.x, 0.1f, planet->pos.y};
                           entities.cursor->getComponent<ComponentTransform>().move(pos);
                           auto& text = entities.cursor->getComponent<ComponentText>();
                           text.setText(planet->name);
                       },
                       [&](SectorData* sector) {
                           const auto pos = Vector3{sector->pos.x, 0.1f, sector->pos.y};
                           entities.cursor->getComponent<ComponentTransform>().move(pos);
                           auto& text = entities.cursor->getComponent<ComponentText>();
                           text.setText(sector->name);
                       },
                   },
                   body);

        entities.cursor->setDisabled(false);
    });

    clickable.setOnClickCallback([this](const size_t i, const bool pressed, const MouseButton button) {
        if (pressed || scene.getPrimaryCamera()->isPanning() || button != MouseButton::Right) {
            return;
        }

        //const auto& system = galaxy.systemsOrdered[i];
        //const auto systemPos = Vector3{system->pos.x, 0.0f, system->pos.y};
        //const auto viewPos = scene.getPrimaryCamera()->worldToScreen(systemPos, true);

        //gui.contextMenu.setEnabled(true);
        //gui.contextMenu.setPos(viewPos);
        //gui.contextMenu.setItems({
        //    {"View", [this]() {}},
        //    {"Info", [this]() {}},
        //    {"Note", [this]() {}},
        //    {"Set Destination", [this]() {}},
        //});
    });

    clickable.setOnBlurCallback([this]() {
        entities.cursor->getComponent<ComponentTransform>().move({0.0f, 0.1f, 0.0f});
        entities.cursor->setDisabled(true);
    });
}*/

/*void ViewSystem::createEntityCursor() {
    entities.cursor = scene.createEntity();
    entities.cursor->setDisabled(true);
    entities.cursor->addComponent<ComponentTransform>().move(Vector3{0.0f, 0.1f, 0.0f});
    entities.cursor->addComponent<ComponentIcon>(images.iconSelect, systemBodySelectable, Theme::primary);
    auto& cursorText = entities.cursor->addComponent<ComponentText>("", Theme::primary, config.guiFontSize);
    cursorText.setCentered(true);
    cursorText.setOffset(Vector2{0, -(systemBodySelectable.y / 2.0f)});
}*/

/*void ViewSystem::createEntitiesBodies() {
    static const Color4 color{1.0f, 1.0f, 1.0f, 1.0f};

    auto& clickable = entities.positions->getComponent<ComponentClickablePoints>();

    entities.icons = scene.createEntity();
    entities.icons->addComponent<ComponentTransform>().move(Vector3{0.0f, 1.0f, 0.0f});
    auto& icons = entities.icons->addComponent<ComponentIconPointCloud>();

    entities.names = scene.createEntity();
    entities.names->addComponent<ComponentTransform>().move(Vector3{0.0f, 2.0f, 0.0f});
    auto& names =
        entities.names->addComponent<ComponentWorldText>(font.regular, Theme::text * alpha(0.5f), config.guiFontSize);
    names.setOffset(Vector2{0.0f, -systemBodySize.y});

    for (const auto& body : system.bodies) {
        std::visit(
            overloaded{
                [&](PlanetData* planet) {
                    auto entity = scene.createEntity();
                    entities.bodies.push_back(entity);

                    const auto pos = Vector3{planet->pos.x, 0.0f, planet->pos.y};
                    entity->addComponent<ComponentTransform>().move(pos);
                    if (planet->parent) {
                        icons.add(pos, systemBodySize, color, images.systemMoon);
                    } else {
                        icons.add(pos, systemBodySize, color, images.systemPlanet);
                        entity->getComponent<ComponentTransform>().scale({2.0f, 2.0f, 2.0f});
                    }

                    // entity->addComponent<ComponentPlanet>(planet->type, planet->seed);

                    clickable.add(pos);
                    // names.add(pos, planet->name);

                    auto orbit = scene.createEntity();
                    entities.orbits.push_back(orbit);
                    orbit->addComponent<ComponentTransform>();
                    // orbit->addComponent<Component2DShape>(Component2DShape::Type::Circle, Theme::text * alpha(0.1f));

                    if (planet->parent) {
                        if (const auto it = system.planets.find(planet->parent.value()); it != system.planets.end()) {
                            orbit->getComponent<ComponentTransform>().move({it->second.pos.x, 0.0f, it->second.pos.y});

                            const auto length = glm::length(glm::distance(it->second.pos, planet->pos));
                            orbit->getComponent<ComponentTransform>().scale(Vector3{length});
                        }
                    } else {
                        orbit->getComponent<ComponentTransform>().scale(Vector3{glm::length(pos)});
                    }
                },
                [&](SectorData* sector) {
                    const auto pos = Vector3{sector->pos.x, 0.0f, sector->pos.y};
                    icons.add(pos, systemBodySize, color, images.systemMoon);

                    clickable.add(pos);
                    names.add(pos, sector->name);
                },
            },
            body);
    }
}*/

void ViewSystem::onEnter() {
}

void ViewSystem::onExit() {
}

#include "ViewSystem.hpp"
#include "../Gui/GuiManager.hpp"
#include "../Utils/Overloaded.hpp"
#include "Client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const Vector2 systemBodySelectable{32.0f, 32.0f};
static const Vector2 systemBodyIconSize{32.0f, 32.0f};

static std::vector<ComponentLines::Line> orbitalLines = ComponentLines::createCircle(1.0f, Color4{1.0f});

ViewSystem::ViewSystem(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager,
                       AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache, FontFamily& font,
                       Client& client) :
    config{config},
    vulkan{vulkan},
    guiManager{guiManager},
    assetsManager{assetsManager},
    voxelShapeCache{voxelShapeCache},
    client{client},
    font{font} {

    icons.systemPlanet = assetsManager.getImages().find("icon_ringed_planet");
    icons.systemMoon = assetsManager.getImages().find("icon_world");
    icons.iconSelect = assetsManager.getImages().find("icon_target");
    icons.currentPos = assetsManager.getImages().find("icon_position_marker");
    icons.info = assetsManager.getImages().find("icon_info");
    icons.view = assetsManager.getImages().find("icon_magnifying_glass");
    icons.travel = assetsManager.getImages().find("icon_vortex");

    textures.star = assetsManager.getTextures().find("space_sun_flare");
    textures.starLow = assetsManager.getTextures().find("star_spectrum_low");
    textures.starHigh = assetsManager.getTextures().find("star_spectrum_high");
}

ViewSystem::~ViewSystem() {
    try {
        if (futureLoad.valid()) {
            futureLoad.get();
        }
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to construct system scene");
    }
}

void ViewSystem::update(const float deltaTime, const Vector2i& viewport) {
    if (futureLoad.valid() && futureLoad.ready()) {
        try {
            futureLoad.get();
            finalize();
            loading = false;
        } catch (...) {
            EXCEPTION_NESTED("Failed to construct system scene");
        }
    }

    if (loading || !scene) {
        return;
    }

    scene->update(deltaTime);
}

void ViewSystem::showContextMenu(const Vector2i& pos, const std::string& sectorId) {
    guiManager.showContextMenu(pos, [=](GuiWindowContextMenu& menu) {
        //
        menu.addItem(icons.info, "Info", []() {});
        menu.addItem(icons.travel, "Warp", [=]() { doWarpTo(sectorId); });
    });
}

void ViewSystem::doWarpTo(const std::string& sectorId) {
    MessageActionWarpTo msg{};
    msg.sectorId = sectorId;
    client.send(msg);
}

void ViewSystem::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewSystem::eventMouseMoved(const Vector2i& pos) {
    if (!loading) {
        scene->eventMouseMoved(pos);
    }
}

void ViewSystem::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (!loading) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewSystem::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (!loading) {
        scene->eventMouseReleased(pos, button);

        const auto& camera = *scene->getPrimaryCamera();
        if (const auto selected = scene->getSelectedEntity();
            selected.has_value() && button == MouseButton::Right && !camera.isPanning()) {
            const auto it = entities.icons.find(selected->getHandle());
            if (it != entities.icons.end()) {
                logger.info("Selected sector: {}", it->second);
                showContextMenu(pos, it->second);
            }
        } else if (guiManager.isContextMenuVisible()) {
            guiManager.getContextMenu().setEnabled(false);
        }
    }
}

void ViewSystem::eventMouseScroll(const int xscroll, const int yscroll) {
    if (!loading) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewSystem::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (!loading) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewSystem::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (!loading) {
        scene->eventKeyReleased(key, modifiers);
    }
}

void ViewSystem::eventCharTyped(const uint32_t code) {
    if (!loading) {
        scene->eventCharTyped(code);
    }
}

Scene* ViewSystem::getScene() {
    if (loading) {
        return nullptr;
    }
    return scene.get();
}

void ViewSystem::load() {
    scene = std::make_unique<Scene>(config, &voxelShapeCache);

    const auto& cache = client.getCache();

    // Setup planet entities
    for (const auto& planet : cache.planets) {
        { // Planet
            auto entity = scene->createEntity().getHandle();

            auto& transform = scene->addComponent<ComponentTransform>(entity);
            transform.move(Vector3{planet.pos.x, 0.0f, planet.pos.y});
            transform.scale(Vector3{planet.radius * 2.0f});

            auto& componentPlanet = scene->addComponent<ComponentPlanet>(entity, planet.type, 1);
            componentPlanet.setBackground(false);
            componentPlanet.setHighRes(false);

            /*auto& componentIcon = entity.addComponent<ComponentIcon>(images.systemPlanet);
            componentIcon.setSize(systemBodyIconSize);
            componentIcon.setColor(Color4{1.0f});*/
        }

        { // Orbit lines
            auto entity = scene->createEntity().getHandle();

            Vector3 origin{0.0f, 0.0f, 0.0f};
            auto radius = glm::length(planet.pos);

            if (planet.parentId) {
                const auto parent = std::find_if(cache.planets.begin(), cache.planets.end(), [&](const auto& p) {
                    return p.id == *planet.parentId;
                });

                if (parent != cache.planets.end()) {
                    origin = Vector3{parent->pos.x, 0.0f, parent->pos.y};
                    radius = glm::length(planet.pos - parent->pos);
                }
            }

            auto& transform = scene->addComponent<ComponentTransform>(entity);
            transform.scale(Vector3{radius});
            transform.move(origin);

            auto& lines = scene->addComponent<ComponentLines>(entity, orbitalLines);
            lines.setColor(Color4{0.5f, 0.5f, 0.5f, 0.3f});
        }
    }

    // Setup sector entities
    entities.icons.clear();
    for (const auto& sector : cache.sectors) {
        { // Icon
            auto entity = scene->createEntity().getHandle();

            auto& transform = scene->addComponent<ComponentTransform>(entity);
            transform.move(Vector3{sector.pos.x, 0.0f, sector.pos.y});

            auto& icon = scene->addComponent<ComponentIcon>(entity, sector.icon);
            icon.setSize(systemBodyIconSize);
            icon.setColor(Color4{1.0f});

            entities.icons.emplace(entity, sector.id);
        }
    }

    /*{ // Figure out where we are
        logger.info("Fetching player location");
        MessagePlayerLocationRequest req{};

        auto future = client.send(req, useFuture<MessagePlayerLocationResponse>());
        auto res = future.get(std::chrono::seconds(1));

        if (!res.location) {
            EXCEPTION("Player has no location");
        }

        // We have user interface supplied galaxy ID and system ID that we need to view.
        // Check if this location is the player's current location
        if (!location.galaxyId.empty()) {
            location.isCurrent =
                location.galaxyId == res.location->galaxyId && location.systemId == res.location->systemId;
            if (location.isCurrent) {
                location.sectorId = res.location->sectorId;
            } else {
                location.sectorId.clear();
            }
        }
        // We have no interface supplied galaxy ID or system ID.
        // We should display the current player's location.
        else {
            location.galaxyId = res.location->galaxyId;
            location.systemId = res.location->systemId;
            location.sectorId = res.location->sectorId;
        }
    }

    loadingValue = 0.2f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Get the galaxy
        logger.info("Fetching galaxy data");
        MessageFetchGalaxyRequest req{};
        req.galaxyId = location.galaxyId;

        auto future = client.send(req, useFuture<MessageFetchGalaxyResponse>());
        auto res = future.get(std::chrono::seconds(1));

        galaxy.name = res.name;
    }

    loadingValue = 0.2f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Get the planets
        logger.info("Fetching planets data");
        system.planets.clear();

        MessageFetchPlanetsRequest req{};
        req.galaxyId = location.galaxyId;
        req.systemId = location.systemId;

        while (!stopToken.shouldStop()) {
            auto future = client.send(req, useFuture<MessageFetchPlanetsResponse>());
            auto res = future.get(std::chrono::seconds(1));

            logger.info("Got planets page with {} items", res.items.size());
            for (auto& planet : res.items) {
                system.planets.insert(std::make_pair(planet.id, std::move(planet)));
            }

            if (res.page.hasNext && !req.token.empty()) {
                req.token = res.page.token;
            } else {
                break;
            }
        }
    }

    loadingValue = 0.4f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Get the planets
        logger.info("Fetching sectors data");
        system.sectors.clear();

        MessageFetchSectorsRequest req{};
        req.galaxyId = location.galaxyId;
        req.systemId = location.systemId;

        while (!stopToken.shouldStop()) {
            auto future = client.send(req, useFuture<MessageFetchSectorsResponse>());
            auto res = future.get(std::chrono::seconds(1));

            logger.info("Got sectors page with {} items", res.items.size());
            for (auto& sector : res.items) {
                system.sectors.insert(std::make_pair(sector.id, std::move(sector)));
            }

            if (res.page.hasNext && !req.token.empty()) {
                req.token = res.page.token;
            } else {
                break;
            }
        }
    }

    loadingValue = 0.8f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Setup Entities
        for (const auto& [_, planet] : system.planets) {
            auto entity = scene->createEntity();
            auto& transform = entity.addComponent<ComponentTransform>();
            transform.move(Vector3{planet.pos.x, 0.0f, planet.pos.y});
            transform.scale(Vector3{planet.radius * 2.0f});
            auto& componentPlanet = entity.addComponent<ComponentPlanet>(planet.type, 1);
            componentPlanet.setBackground(false);
            componentPlanet.setHighRes(false);
            auto& componentIcon = entity.addComponent<ComponentIcon>(images.systemPlanet);
            componentIcon.setSize(systemBodyIconSize);
            componentIcon.setColor(Color4{1.0f});
        }
    }*/
}

void ViewSystem::finalize() {
    { // To keep the renderer away from complaining
        auto entity = scene->createEntity();
        entity.addComponent<ComponentPointLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        entity.addComponent<ComponentTransform>();
    }

    { // Skybox
        auto entity = scene->createEntity();
        auto& skybox = entity.addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    { // Sun
        auto entity = scene->createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        transform.translate(Vector3{0.0f, -1.0f, 0.0f});
        auto& flare = entity.addComponent<ComponentStarFlare>(textures.star, textures.starLow, textures.starHigh);
        flare.setSize(Vector2{0.75f, 0.75f});
        flare.setBackground(false);
    }

    { // Our primary camera
        auto entity = scene->createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        auto& camera = entity.addComponent<ComponentCamera>(transform);
        auto& cameraPanning = entity.addComponent<ComponentCameraPanning>(camera);
        cameraPanning.setTarget({0.0f, 10.0f, 0.0f});
        cameraPanning.setDistanceRange(3.0f, 250.0f);
        cameraPanning.setDistance(5.0f);
        scene->setPrimaryCamera(entity);

        entities.camera = entity;
    }
}

void ViewSystem::clearEntities() {
    /*entities.camera.reset();
    entities.bodies.clear();
    entities.orbits.clear();
    entities.icons.reset();
    entities.cursor.reset();
    entities.positions.reset();
    entities.names.reset();*/
}

void ViewSystem::onEnter() {
    clearEntities();
    scene.reset();

    loading = true;
    futureLoad = std::async([this]() { return load(); });
}

void ViewSystem::onExit() {
    // Stop loading if we have exited
    if (futureLoad.valid()) {
        try {
            futureLoad.get();
        } catch (...) {
            EXCEPTION_NESTED("Failed to construct galaxy scene");
        }
    }
}

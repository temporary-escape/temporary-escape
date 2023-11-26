#include "ViewSystem.hpp"
#include "../Graphics/Theme.hpp"
#include "../Utils/Overloaded.hpp"
#include "Client.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const Vector2 systemBodySelectable{32.0f, 32.0f};
static const Vector2 systemBodyIconSize{32.0f, 32.0f};

ViewSystem::ViewSystem(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                       VoxelShapeCache& voxelShapeCache, Client& client, FontFamily& font) :
    parent{parent},
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    voxelShapeCache{voxelShapeCache},
    client{client},
    font{font},
    guiModalLoading{"System Map"} {

    guiModalLoading.setEnabled(false);

    images.systemPlanet = assetsManager.getImages().find("icon_ringed_planet");
    images.systemMoon = assetsManager.getImages().find("icon_world");
    images.iconSelect = assetsManager.getImages().find("icon_target");

    textures.star = assetsManager.getTextures().find("space_sun_flare");
    textures.starLow = assetsManager.getTextures().find("star_spectrum_low");
    textures.starHigh = assetsManager.getTextures().find("star_spectrum_high");
}

void ViewSystem::update(const float deltaTime) {
    guiModalLoading.setProgress(loadingValue);

    if (futureLoad.valid() && futureLoad.ready()) {
        try {
            futureLoad.get();
            finalize();
            loading = false;
            guiModalLoading.setEnabled(false);
        } catch (...) {
            EXCEPTION_NESTED("Failed to construct galaxy scene");
        }
    }

    if (loading) {
        return;
    }

    scene->update(deltaTime);
}

void ViewSystem::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewSystem::renderNuklear(Nuklear& nuklear, const Vector2i& viewport) {
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
        scene->setPrimaryCamera(entity);

        entities.camera = entity;
    }
}

void ViewSystem::clearEntities() {
    entities.camera.reset();
    entities.bodies.clear();
    entities.orbits.clear();
    entities.icons.reset();
    entities.cursor.reset();
    entities.positions.reset();
    entities.names.reset();
}

void ViewSystem::reset() {
    location.galaxyId.clear();
    location.systemId.clear();
}

void ViewSystem::reset(const std::string& galaxyId, const std::string& systemId) {
    location.galaxyId = galaxyId;
    location.systemId = systemId;
}

void ViewSystem::onEnter() {
    clearEntities();
    scene.reset();

    loading = true;
    loadingValue = 0.0f;
    guiModalLoading.setEnabled(true);

    stopToken.stop();
    stopToken = StopToken{};

    futureLoad = std::async([this]() { return load(); });
}

void ViewSystem::onExit() {
    // Stop loading if we have exited
    stopToken.stop();
    guiModalLoading.setEnabled(false);
}

#include "view_galaxy.hpp"
#include "../graphics/theme.hpp"
#include "../math/convex_hull.hpp"
#include "client.hpp"
#include "game.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const Vector2 systemStarSelectable{48.0f, 48.0f};
static const Vector2 systemStarSize{32.0f, 32.0f};

ViewGalaxy::Gui::Gui(const Config& config, AssetsManager& assetsManager) : modalLoading{"Galaxy Map"} {

    modalLoading.setEnabled(false);
}

ViewGalaxy::ViewGalaxy(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                       VoxelShapeCache& voxelShapeCache, Client& client, Gui& gui, FontFamily& font) :
    parent{parent},
    config{config},
    vulkan{vulkan},
    assetsManager{assetsManager},
    voxelShapeCache{voxelShapeCache},
    client{client},
    gui{gui},
    font{font} {

    textures.systemStar = assetsManager.getTextures().find("star_flare");
    images.iconSelect = assetsManager.getImages().find("icon_target");
    images.iconCurrentPos = assetsManager.getImages().find("icon_position_marker");
}

ViewGalaxy::~ViewGalaxy() {
    try {
        if (futureLoad.valid()) {
            futureLoad.get();
        }
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to construct galaxy scene");
    }
}

void ViewGalaxy::update(const float deltaTime) {
    gui.modalLoading.setProgress(loadingValue);

    if (futureLoad.valid() && futureLoad.ready()) {
        try {
            futureLoad.get();
            finalize();
            loading = false;
            gui.modalLoading.setEnabled(false);
        } catch (...) {
            EXCEPTION_NESTED("Failed to construct galaxy scene");
        }
    }

    if (loading) {
        return;
    }

    scene->update(deltaTime);

    const auto* camera = scene->getPrimaryCamera();

    // Update connections alpha color
    if (entities.systems) {
        const auto f = 1.0f - glm::clamp(map(camera->getOrthoScale(), 25.0f, 120.0f, 0.0f, 0.8f), 0.0f, 0.8f);
        entities.systems.getComponent<ComponentLines>().setColor(alpha(f));
    }

    // Update region labels alpha color
    if (entities.regions) {
        const auto f = glm::clamp(map(camera->getOrthoScale(), 20.0f, 70.0f, 0.1f, 0.85f), 0.1f, 0.85f);
        entities.regions.getComponent<ComponentWorldText>().setColor(Theme::text * alpha(f));
    }

    // Update system names
    if (entities.names) {
        const auto f = 1.0f - glm::clamp(map(camera->getOrthoScale(), 20.0f, 100.0f, 0.5f, 1.0f), 0.5f, 1.0f);
        entities.names.getComponent<ComponentWorldText>().setColor(Theme::text * alpha(f));
    }
}

void ViewGalaxy::renderCanvas(Canvas& canvas, const Vector2i& viewport) {
}

void ViewGalaxy::eventMouseMoved(const Vector2i& pos) {
    if (!loading) {
        scene->eventMouseMoved(pos);
    }
}

void ViewGalaxy::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    if (!loading) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewGalaxy::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    if (!loading) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewGalaxy::eventMouseScroll(const int xscroll, const int yscroll) {
    if (!loading) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewGalaxy::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (!loading) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewGalaxy::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (!loading) {
        scene->eventKeyReleased(key, modifiers);
    }
}

void ViewGalaxy::eventCharTyped(const uint32_t code) {
    if (!loading) {
        scene->eventCharTyped(code);
    }
}

Scene* ViewGalaxy::getScene() {
    if (loading) {
        return nullptr;
    }
    return scene.get();
}

void ViewGalaxy::load() {
    scene = std::make_unique<Scene>(config, &voxelShapeCache);

    /*{ // Figure out where we are
        logger.info("Fetching player location");
        MessagePlayerLocationRequest req{};

        auto future = client.send(req, useFuture<MessagePlayerLocationResponse>());
        auto res = future.get(std::chrono::seconds(1));

        if (!res.location) {
            EXCEPTION("Player has no location");
        }

        location.galaxyId = res.location->galaxyId;
        location.systemId = res.location->systemId;
        location.sectorId = res.location->sectorId;
    }

    loadingValue = 0.1f;
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

    { // Get the regions
        logger.info("Fetching regions data");
        galaxy.regions.clear();

        MessageFetchRegionsRequest req{};
        req.galaxyId = location.galaxyId;

        while (!stopToken.shouldStop()) {
            auto future = client.send(req, useFuture<MessageFetchRegionsResponse>());
            auto res = future.get(std::chrono::seconds(1));

            logger.info("Got regions page with {} items", res.items.size());
            for (auto& region : res.items) {
                galaxy.regions.insert(std::make_pair(region.id, std::move(region)));
            }

            if (res.page.hasNext && !req.token.empty()) {
                req.token = res.page.token;
            } else {
                break;
            }
        }
    }

    loadingValue = 0.3f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Get the factions
        logger.info("Fetching factions data");
        factions.clear();

        MessageFetchFactionsRequest req{};
        req.galaxyId = location.galaxyId;

        while (!stopToken.shouldStop()) {
            auto future = client.send(req, useFuture<MessageFetchFactionsResponse>());
            auto res = future.get(std::chrono::seconds(1));

            logger.info("Got factions page with {} items", res.items.size());
            for (auto& faction : res.items) {
                factions.insert(std::make_pair(faction.id, std::move(faction)));
            }

            if (res.page.hasNext && !res.page.token.empty()) {
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

    { // Get the systems
        logger.info("Fetching systems data");
        galaxy.systems.clear();

        MessageFetchSystemsRequest req{};
        req.galaxyId = location.galaxyId;

        while (!stopToken.shouldStop()) {
            auto future = client.send(req, useFuture<MessageFetchSystemsResponse>());
            auto res = future.get(std::chrono::seconds(1));

            logger.info("Got systems page with {} items", res.items.size());
            for (auto& system : res.items) {
                galaxy.systems.insert(std::make_pair(system.id, std::move(system)));
            }

            if (res.page.hasNext && !res.page.token.empty()) {
                req.token = res.page.token;
            } else {
                break;
            }
        }
    }

    loadingValue = 0.6f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Ordered systems list needed for creating shapes
        galaxy.systemsOrdered.clear();
        galaxy.systemsOrdered.reserve(galaxy.systems.size());

        for (const auto& [_, system] : galaxy.systems) {
            galaxy.systemsOrdered.push_back(&system);
        }
    }

    loadingValue = 0.8f;
    if (stopToken.shouldStop()) {
        return;
    }

    logger.info("Creating entities");

    entities.systems = scene->createEntity();
    entities.systems.addComponent<ComponentTransform>();
    auto& pointCloud = entities.systems.addComponent<ComponentPointCloud>(textures.systemStar);
    auto& lines = entities.systems.addComponent<ComponentLines>(std::vector<ComponentLines::Line>{});

    entities.names = scene->createEntity();
    entities.names.addComponent<ComponentTransform>();
    auto& names =
        entities.names.addComponent<ComponentWorldText>(font.regular, Theme::text * alpha(0.5f), config.guiFontSize);
    names.setOffset(Vector2{0, -config.guiFontSize});

    entities.regions = scene->createEntity();
    entities.regions.addComponent<ComponentTransform>();
    auto& regionsNames =
        entities.regions.addComponent<ComponentWorldText>(font.regular, Theme::text * alpha(0.1f), 32.0f);

    { // Create entities for regions
        logger.info("Creating regions");

        for (const auto& [_, region] : galaxy.regions) {
            regionsNames.add(Vector3{region.pos.x, 0.0f, region.pos.y}, region.name);
        }
    }

    { // System lines and point cloud
        logger.info("Creating systems and connections");

        for (const auto& [systemId, system] : galaxy.systems) {
            auto starColor = Color4{0.8f, 0.8f, 0.8f, 1.0f};
            auto connectionColor = Color4{0.7f, 0.7f, 0.7f, 0.3f};

            const auto region = galaxy.regions.find(system.regionId);
            if (region == galaxy.regions.end()) {
                EXCEPTION("No such region: '{}' for system: '{}'", system.regionId);
            }

            if (system.factionId) {
                const auto faction = factions.find(*system.factionId);
                if (faction == factions.end()) {
                    EXCEPTION("No such faction: '{}' for system: '{}'", *system.factionId);
                }

                starColor = hsvToRgb(Color4{faction->second.color, 0.9f, 1.0f, 1.0f});
            }

            pointCloud.add(Vector3{system.pos.x, 0.0f, system.pos.y}, {0.03f, 0.03f}, starColor);

            for (const auto& otherId : system.connections) {
                const auto other = galaxy.systems.find(otherId);
                if (other == galaxy.systems.end()) {
                    EXCEPTION("No such connection found: '{}' for system: '{}'", otherId, system.id);
                }

                lines.add(system.pos, other->second.pos, connectionColor);
            }

            names.add(Vector3{system.pos.x, 0.0f, system.pos.y}, system.name);
        }
    }

    loadingValue = 0.9f;
    if (stopToken.shouldStop()) {
        return;
    }

    { // Calculate background
        logger.info("Creating background");

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

        const auto voronoi = computeVoronoiDiagram(positions, clip);

        if (voronoi.cells.size() != galaxy.systemsOrdered.size()) {
            EXCEPTION("Voronoi incorrect number of cells");
        }

        entities.voronoi = scene->createEntity();
        entities.voronoi.addComponent<ComponentTransform>().move(Vector3{0.0f, -0.1f, 0.0f});
        auto& polyShape = entities.voronoi.addComponent<ComponentPolyShape>();

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
    }

    { // Cursor
        entities.cursor = scene->createEntity();
        entities.cursor.addComponent<ComponentTransform>();
        auto& componentIcon = entities.cursor.addComponent<ComponentIcon>(images.iconSelect);
        componentIcon.setSize(systemStarSelectable);
        componentIcon.setColor(Theme::primary);
        entities.cursor.setDisabled(true);
    }

    { // Current Position
        const auto currentSystem = galaxy.systems.at(location.systemId);
        entities.currentPos = scene->createEntity();
        auto& transform = entities.currentPos.addComponent<ComponentTransform>();
        transform.move(Vector3{currentSystem.pos.x, 0.0f, currentSystem.pos.y});
        auto& icon = entities.currentPos.addComponent<ComponentIcon>(images.iconCurrentPos);
        icon.setOffset(Vector2{0.0f, -(systemStarSize.y / 2.0f)});
        icon.setSize(systemStarSize);
        icon.setColor(Theme::primary);
    }*/

    /*{ // User Input
        logger.info("Creating system user input callbacks");
        for (const auto& [systemId, system] : galaxy.systems) {
            auto entity = scene->createEntity();
            entity.addComponent<ComponentTransform>().translate({system.pos.x, 0.0f, system.pos.y});
            auto& selectable = entity.addComponent<Component2DSelectable>(systemStarSize);
            selectable.setOnHoverCallback([this, id = systemId](const bool enter) {
                entities.cursor.setDisabled(!enter);
                auto& found = galaxy.systems.at(id);
                entities.cursor.getComponent<ComponentTransform>().move({found.pos.x, 1.0f, found.pos.y});
            });
            selectable.setOnSelectCallback([this, id = systemId](const MouseButton button) {
                logger.debug("setOnSelectCallback systemId: {}", id); //
            });
        }
    }*/
}

void ViewGalaxy::finalize() {
    { // To keep the renderer away from complaining
        auto entity = scene->createEntity();
        entity.addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
        entity.addComponent<ComponentTransform>().translate(Vector3{0.0f, 1.0f, 0.0f});
    }

    { // Skybox
        auto entity = scene->createEntity();
        auto& skybox = entity.addComponent<ComponentSkybox>(0);
        auto skyboxTextures = SkyboxTextures{vulkan, Color4{0.02f, 0.02f, 0.02f, 1.0f}};
        skybox.setTextures(vulkan, std::move(skyboxTextures));
    }

    { // Our primary camera
        auto entity = scene->createEntity();
        auto& transform = entity.addComponent<ComponentTransform>();
        auto& camera = entity.addComponent<ComponentCamera>(transform);
        camera.setOrthographic(25.0f);
        camera.lookAt({0.0f, 10.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
        camera.setZoomRange(3.0f, 250.0f);
        scene->setPrimaryCamera(entity);

        entities.camera = entity;
    }

    loadingValue = 1.0f;
}

void ViewGalaxy::clearEntities() {
    entities.camera.reset();
    entities.regions.reset();
    entities.systems.reset();
    entities.cursor.reset();
    entities.voronoi.reset();
    entities.names.reset();
}

void ViewGalaxy::onEnter() {
    clearEntities();
    scene.reset();

    loading = true;
    loadingValue = 0.0f;
    gui.modalLoading.setEnabled(true);

    stopToken.stop();
    stopToken = StopToken{};

    futureLoad = std::async([this]() { return load(); });
}

void ViewGalaxy::onExit() {
    // Stop loading if we have exited
    stopToken.stop();
    gui.modalLoading.setEnabled(false);
}

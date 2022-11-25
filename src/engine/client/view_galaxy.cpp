#include "view_galaxy.hpp"
#include "../graphics/renderer.hpp"
#include "client.hpp"

#define CMP "ViewGalaxy"

using namespace Engine;

ViewGalaxy::ViewGalaxy(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines, Registry& registry,
                       Canvas& canvas, FontFamily& font, Nuklear& nuklear, Client& client) :
    config{config},
    vulkan{vulkan},
    registry{registry},
    canvas{canvas},
    font{font},
    nuklear{nuklear},
    client{client},
    skybox{vulkan, Color4{0.1f, 0.1f, 0.1f, 1.0f}},
    scene{&registry.getVoxelShapeCache(), &scenePipelines},
    modalLoading{nuklear, "Galaxy Map"} {

    textures.systemStar = registry.getTextures().find("star_flare");

    // To keep the renderer away from complaining
    auto sun = std::make_shared<Entity>();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.0f, 1.0f, 1.0f, 1.0f});
    sun->translate(Vector3{0.0f, 1.0f, 0.0f});
    scene.addEntity(sun);

    // Our primary camera
    auto camera = std::make_shared<Entity>();
    auto cmp = camera->addComponent<ComponentCamera>();
    camera->addComponent<ComponentUserInput>(*cmp);
    cmp->setOrthographic(5.0f);
    cmp->lookAt({0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    cmp->setZoomRange(3.0f, 500.0f);
    scene.addEntity(camera);
    scene.setPrimaryCamera(camera);
}

void ViewGalaxy::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewGalaxy::render(const Vector2i& viewport, Renderer& renderer) {
    renderer.render(viewport, scene, skybox);
}

void ViewGalaxy::renderCanvas(const Vector2i& viewport) {
    nuklear.begin(viewport);
    if (loading) {
        modalLoading.draw(viewport);
    }
    nuklear.end();
}

void ViewGalaxy::eventUserInput(const UserInput::Event& event) {
    scene.eventUserInput(event);
}

void ViewGalaxy::load() {
    loading = true;
    loadingValue = 0.1f;
    modalLoading.setProgress(loadingValue);
    fetchCurrentLocation();
}

void ViewGalaxy::fetchCurrentLocation() {
    MessagePlayerLocationRequest req{};

    client.send(req, [this](MessagePlayerLocationResponse res) {
        Log::d(CMP, "Received player location info");

        location.galaxyId = res.galaxyId;
        location.systemId = res.systemId;
        location.sectorId = res.sectorId;

        loadingValue = 0.2f;

        fetchGalaxyInfo();
    });
}

void ViewGalaxy::fetchGalaxyInfo() {
    MessageFetchGalaxyRequest req{};
    req.galaxyId = location.galaxyId;

    client.send(req, [this](MessageFetchGalaxyResponse res) {
        Log::d(CMP, "Received galaxy info for: '{}'", location.galaxyId);

        galaxy.name = res.name;
        galaxy.systems.clear();
        galaxy.regions.clear();
        factions.clear();

        loadingValue = 0.3f;

        fetchFactionsPage("");
    });
}

void ViewGalaxy::fetchFactionsPage(const std::string& token) {
    MessageFetchFactionsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this](MessageFetchFactionsResponse res) {
        for (const auto& faction : res.factions) {
            factions[faction.id] = faction;
        }

        if (res.hasNext) {
            fetchRegionsPage(res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} factions", factions.size());
            loadingValue = 0.5f;
            fetchRegionsPage("");
        }
    });
}

void ViewGalaxy::fetchRegionsPage(const std::string& token) {
    MessageFetchRegionsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this](MessageFetchRegionsResponse res) {
        for (const auto& region : res.regions) {
            galaxy.regions[region.id] = region;
        }

        if (res.hasNext) {
            fetchRegionsPage(res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} regions", galaxy.regions.size());
            loadingValue = 0.5f;
            fetchSystemsPage("");
        }
    });
}

void ViewGalaxy::fetchSystemsPage(const std::string& token) {
    MessageFetchSystemsRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;

    client.send(req, [this](MessageFetchSystemsResponse res) {
        for (const auto& system : res.systems) {
            galaxy.systems[system.id] = system;
        }

        if (res.hasNext) {
            fetchSystemsPage(res.token);
        } else {
            Log::i(CMP, "Received galaxy with {} systems", galaxy.systems.size());
            loadingValue = 0.7f;
            updateGalaxy();
        }
    });
}

void ViewGalaxy::updateGalaxy() {
    static const Vector2 systemStarSize{32.0f, 32.0f};

    Log::i(CMP, "Recreating galaxy objects with {} systems", galaxy.systems.size());
    loading = false;
    loadingValue = 1.0f;

    if (entities.systems) {
        scene.removeEntity(entities.systems);
    }

    entities.systems = std::make_shared<Entity>();
    auto pointCloud = entities.systems->addComponent<ComponentPointCloud>(textures.systemStar);
    for (const auto& [systemId, system] : galaxy.systems) {
        const auto region = galaxy.regions.find(system.regionId);
        if (region == galaxy.regions.end()) {
            EXCEPTION("No such region id: '{}'", system.regionId);
        }

        pointCloud->add(Vector3{system.pos.x, 0.0f, system.pos.y}, systemStarSize, Color4{1.0f, 1.0f, 1.0f, 1.0f});
    }
    scene.addEntity(entities.systems);

    auto debug = std::make_shared<Entity>();
    debug->addComponent<ComponentDebug>()->addBox(glm::translate(Vector3{0.0f}), 0.5f, Color4{1.0f, 0.0f, 0.0f, 1.0f});
    scene.addEntity(debug);
}

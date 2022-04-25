#include "ViewMap.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

#define CMP "ViewMap"

using namespace Engine;

ViewMap::ViewMap(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer, Client& client,
                 Widgets& widgets)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client), widgets(widgets) {

    textures.star = assetManager.find<AssetTexture>("star_flare");
    images.currentPosition = assetManager.find<AssetImage>("icon_position_marker");
    fontFaceRegular = assetManager.find<AssetFontFamily>("iosevka-aile")->get("regular");
}

void ViewMap::load() {
    loading = true;

    entityCamera.reset();
    entitiesSystems.clear();
    entitiesRegions.clear();
    scene = std::make_unique<Scene>();

    auto skybox = std::make_shared<Entity>();
    skybox->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.0f, 0.0f, 0.0f, 1.0f}));
    skybox->scale(Vector3{1000.0f});
    scene->addEntity(skybox);

    entityCamera = std::make_shared<Entity>();
    auto camera = entityCamera->addComponent<ComponentCameraTop>();
    camera->setOrthographic(50.0f);
    entityCamera->addComponent<ComponentUserInput>(*camera);
    scene->addEntity(entityCamera);
    scene->setPrimaryCamera(entityCamera);

    fetchGalaxy();
}

void ViewMap::render(const Vector2i& viewport) {
    if (scene != nullptr) {
        scene->update(0.1f);
        renderer.setEnableBackground(true);
        renderer.setEnableBloom(false);
        renderer.render(*scene);
    }
}

void ViewMap::renderGui(const Vector2i& viewport) {
    if (loading) {
        widgets.loading("Initializing map data...", 0.5f);
    } else {
        renderRegionLabels();
        renderRegionSystemLabels();
        renderCurrentPositionMarker();
        renderCurrentPositionInfo();
    }
}

void ViewMap::renderRegionLabels() {
    const auto camera = scene->getPrimaryCamera();

    canvas.beginPath();
    canvas.fontFace(fontFaceRegular->getHandle());
    canvas.fontSize(36.0f);
    canvas.fillColor(GuiColors::text * alpha(0.3f));

    for (const auto& pair : data.regions) {
        const auto& region = pair.second;

        const auto bounds = canvas.textBounds(region.name);

        auto pos = camera->worldToScreen(Vector3{region.pos.x, 0.0f, region.pos.y});

        canvas.text(pos - Vector2{bounds.x / 2.0f, bounds.y / 2.0f}, region.name);
    }
    canvas.closePath();
}

void ViewMap::renderRegionSystemLabels() {
    const auto camera = scene->getPrimaryCamera();

    canvas.beginPath();
    canvas.fontFace(fontFaceRegular->getHandle());
    canvas.fontSize(18.0f);
    canvas.fillColor(GuiColors::text * alpha(0.7f));

    const auto& currentSystem = data.systems.at(client.getPlayerLocation().systemId);
    const auto& currentRegion = data.regions.at(currentSystem.regionId);

    for (const auto& pair : data.systems) {
        const auto& system = pair.second;
        if (system.regionId != currentRegion.id) {
            continue;
        }

        const auto bounds = canvas.textBounds(system.name);

        auto pos = camera->worldToScreen(Vector3{system.pos.x, 0.0f, system.pos.y});

        canvas.text(pos + Vector2{16.0f, -bounds.y / 2.0f}, system.name);
    }
    canvas.closePath();
}

void ViewMap::renderCurrentPositionMarker() {
    static const Vector2 markerSize{32.0f};

    const auto& currentSystem = data.systems.at(client.getPlayerLocation().systemId);

    const auto pos = scene->getPrimaryCamera()->worldToScreen(Vector3{currentSystem.pos.x, 0.0f, currentSystem.pos.y});

    canvas.beginPath();
    canvas.rectImage(pos - Vector2{markerSize.x / 2.0f, markerSize.y}, markerSize, images.currentPosition->getImage(),
                     GuiColors::primary);
    canvas.fill();
    canvas.closePath();
}

void ViewMap::renderCurrentPositionInfo() {
    static const Vector2 markerSize{64.0f};

    const auto& currentSystem = data.systems.at(client.getPlayerLocation().systemId);
    const auto& currentRegion = data.regions.at(currentSystem.regionId);

    using Item = std::tuple<float, Color4, std::string>;
    const std::vector<Item> items = {
        {36.0f, GuiColors::primary, currentSystem.name},
        {18.0f, GuiColors::text, fmt::format("Galaxy: {}", data.galaxy.name)},
        {18.0f, GuiColors::text, fmt::format("Region: {}", currentRegion.name)},
    };

    Vector2 box;
    canvas.fontFace(fontFaceRegular->getHandle());
    for (const auto& [size, _, text] : items) {
        canvas.fontSize(size);
        const auto bounds = canvas.textBounds(text);

        box.x = std::max(bounds.x, box.x);
        box.y += bounds.y + 4.0f;
    }

    box.x += 8.0f + markerSize.x;
    box.y += 8.0f;

    canvas.fillColor(GuiColors::backgroundTransparent);
    canvas.beginPath();
    canvas.rect(Vector2{50.0f, 30.0f}, box);
    canvas.fill();
    canvas.closePath();

    canvas.beginPath();
    Vector2 pos{54.0f, 34.0f};
    auto textPos = pos + Vector2{markerSize.x, 0.0f};

    for (const auto& [size, color, text] : items) {
        canvas.fontSize(size);
        const auto bounds = canvas.textBounds(text);
        textPos.y += bounds.y;

        canvas.fillColor(color);
        canvas.text(textPos, text);

        textPos.y += 4.0f;
    }
    canvas.closePath();

    canvas.beginPath();
    canvas.rectImage(pos, markerSize, images.currentPosition->getImage(), GuiColors::primary);
    canvas.fill();
    canvas.closePath();
}

void ViewMap::fetchGalaxy() {
    MessageFetchGalaxy::Request req{};
    req.galaxyId = client.getPlayerLocation().galaxyId;

    client.send(req, [this](MessageFetchGalaxy::Response res) {
        Log::d(CMP, "fetchGalaxy result");
        data.galaxy = std::move(res.galaxy);
        fetchRegions();
    });
}

void ViewMap::fetchRegions() {
    MessageFetchRegions::Request req{};
    req.galaxyId = client.getPlayerLocation().galaxyId;

    client.send(req, [this](MessageFetchRegions::Response res) {
        Log::d(CMP, "fetchRegions result");
        data.regions.clear();
        for (auto& region : res.regions) {
            data.regions.insert(std::make_pair(region.id, std::move(region)));
        }
        fetchSystems();
    });
}

void ViewMap::fetchSystems() {
    MessageFetchSystems::Request req{};
    req.galaxyId = client.getPlayerLocation().galaxyId;

    client.send(req, [this](MessageFetchSystems::Response res) {
        Log::d(CMP, "fetchSystems result");
        data.systems.clear();
        for (auto& system : res.systems) {
            data.systems.insert(std::make_pair(system.id, std::move(system)));
        }
        reconstruct();
    });
}

void ViewMap::reconstruct() {
    static const auto colorStarInRegion = Color4{1.0f, 1.0f, 1.0f, 1.0f};
    static const auto colorStarOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.7f};
    static const auto colorConnectionInRegion = GuiColors::secondary * alpha(0.6f);
    static const auto colorConnectionOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.2f};

    static const Vector2 starSize{32.0f};
    static const Vector2 markerSize{32.0f};

    if (!scene) {
        return;
    }

    loading = false;

    const auto getCurrentSystem = [this]() -> SystemData* {
        auto it = data.systems.find(client.getPlayerLocation().systemId);
        if (it != data.systems.end()) {
            return &it->second;
        }
        return nullptr;
    };

    const auto getCurrentRegion = [this](SystemData* systemData) -> RegionData* {
        if (!systemData) {
            return nullptr;
        }

        auto it = data.regions.find(systemData->regionId);
        if (it != data.regions.end()) {
            return &it->second;
        }
        return nullptr;
    };

    try {
        for (auto& entity : entitiesSystems) {
            scene->removeEntity(entity);
        }
        entitiesSystems.clear();

        for (auto& entity : entitiesRegions) {
            scene->removeEntity(entity);
        }
        entitiesRegions.clear();

        auto currentSystem = getCurrentSystem();
        auto currentRegion = getCurrentRegion(currentSystem);

        const auto isInsideOfRegion = [&](const SystemData& system) {
            if (!currentSystem || !currentRegion) {
                return false;
            }

            return currentRegion->id == system.regionId;
        };

        const auto isCurrentRegion = [&](const RegionData& region) {
            if (!currentRegion) {
                return false;
            }

            return currentRegion->id == region.id;
        };

        std::unordered_set<std::string> connections;

        std::unordered_map<std::string, std::shared_ptr<ComponentLines>> regionLines;

        for (const auto& [_, region] : data.regions) {
            auto entity = std::make_shared<Entity>();
            entity->translate(Vector3{region.pos.x, 0.0f, region.pos.y});
            auto lines = entity->addComponent<ComponentLines>();
            // auto label =
            //     entity->addComponent<ComponentText>(fontFaceRegular, region.name, GuiColors::text *
            //     alpha(0.3f), 36.0f);
            // label->setVisible(true);
            // label->setCentered(true);

            regionLines[region.id] = lines;

            scene->addEntity(entity);
            entitiesRegions.push_back(entity);
        }

        for (const auto& [_, region] : data.regions) {
            const Color4& regionColor = isCurrentRegion(region) ? colorConnectionInRegion : colorConnectionOutRegion;

            auto entity = std::make_shared<Entity>();
            auto pointCloud = entity->addComponent<ComponentPointCloud>(textures.star);
            auto lines = entity->addComponent<ComponentLines>();

            for (const auto& [_, system] : data.systems) {
                if (system.regionId != region.id) {
                    continue;
                }

                const Color4& color = isInsideOfRegion(system) ? colorStarInRegion : colorStarOutRegion;
                pointCloud->add(Vector3{system.pos.x, 0.0f, system.pos.y}, starSize, color);

                // auto label = entity->addComponent<ComponentText>(fontFaceRegular, system.name,
                //                                                  GuiColors::text * alpha(0.7f), 18.0f);
                // label->setVisible(isInsideOfRegion(system));
                // label->setOffset(Vector2{starSize.x / 2.0f, 0.0f});

                for (const auto& conn : system.connections) {
                    const auto& other = data.systems.at(conn);

                    const auto t0 = fmt::format("{}/{}", system.id, other.id);
                    const auto t1 = fmt::format("{}/{}", other.id, system.id);

                    if (connections.find(t0) == connections.end() && connections.find(t1) == connections.end()) {
                        lines->add(Vector3{system.pos.x, 0.0f, system.pos.y}, Vector3{other.pos.x, 0.0f, other.pos.y},
                                   regionColor);

                        connections.insert(t0);
                        connections.insert(t1);
                    }
                }
            }

            scene->addEntity(entity);
            entitiesSystems.push_back(entity);
        }

        if (currentSystem) {
            entityCamera->getComponent<ComponentCameraTop>()->moveTo(
                Vector3{currentSystem->pos.x, 0.0f, currentSystem->pos.y});
        }

    } catch (...) {
        EXCEPTION_NESTED("Failed to reconstruct the gui map");
    }
}

void ViewMap::eventMouseMoved(const Vector2i& pos) {
    if (scene) {
        scene->eventMouseMoved(pos);
    }
}

void ViewMap::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMousePressed(pos, button);
    }
}

void ViewMap::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (scene) {
        scene->eventMouseReleased(pos, button);
    }
}

void ViewMap::eventMouseScroll(const int xscroll, const int yscroll) {
    if (scene) {
        scene->eventMouseScroll(xscroll, yscroll);
    }
}

void ViewMap::eventKeyPressed(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyPressed(key, modifiers);
    }
}

void ViewMap::eventKeyReleased(const Key key, const Modifiers modifiers) {
    if (scene) {
        scene->eventKeyReleased(key, modifiers);
    }
}

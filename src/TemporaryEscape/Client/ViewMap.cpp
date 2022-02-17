#include "ViewMap.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"
#include "Widgets.hpp"

using namespace Engine;

ViewMap::ViewMap(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer, Client& client,
                 GuiContext& gui)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client), gui(gui) {

    images.galaxyStar = assetManager.find<AssetImage>("star_flare");
    images.currentPosition = assetManager.find<AssetImage>("icon_position_marker");
    fontFaceRegular = assetManager.find<AssetFontFamily>("iosevka-aile")->get("regular");

    auto& store = client.getStore();

    store.player.location.onChange([this]() { reconstruct(); });
    store.galaxy.galaxy.onChange([this]() { this->client.fetchGalaxyRegions(); });
    store.galaxy.regions.onChange([this]() { this->client.fetchGalaxySystems(); });
    store.galaxy.systems.onChange([this]() { reconstruct(); });
}

void ViewMap::load() {
    loading = true;

    entityCamera.reset();
    entitiesSystems.clear();
    entitiesRegions.clear();
    scene = std::make_unique<Scene>();

    entityCamera = std::make_shared<Entity>();
    auto camera = entityCamera->addComponent<ComponentCameraTop>();
    camera->setPrimary(true);
    camera->setOrthographic(50.0f);
    camera->setZoom(50.0f);
    entityCamera->addComponent<ComponentUserInput>(*camera);
    scene->addEntity(entityCamera);

    client.fetchGalaxy();
}

void ViewMap::render(const Vector2i& viewport) {
    if (scene != nullptr) {
        scene->update(0.0f);
        renderer.render(viewport, *scene);
    }
}

void ViewMap::renderGui(const Vector2i& viewport) {
    if (loading) {
        Widgets::loading(gui, viewport, "Initializing map data...", 0.5f);
    } else {
        renderCurrentPositionInfo();
    }
}

void ViewMap::renderCurrentPositionInfo() {
    static const Vector2 markerSize{64.0f};

    auto& store = client.getStore();

    auto camera = entityCamera->getComponent<ComponentCameraTop>();

    const auto& currentSystem = store.galaxy.systems.value().at(store.player.location.value().systemId);
    const auto& currentRegion = store.galaxy.regions.value().at(currentSystem.regionId);

    using Item = std::tuple<float, Color4, std::string>;
    const std::vector<Item> items = {
        {36.0f, GuiColors::primary, currentSystem.name},
        {18.0f, GuiColors::text, fmt::format("Galaxy: {}", store.galaxy.galaxy.value().name)},
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

void ViewMap::reconstruct() {
    static const Vector2 starSize{32.0f};
    static const Vector2 markerSize{32.0f};

    loading = false;

    for (auto& entity : entitiesSystems) {
        scene->removeEntity(entity);
    }
    entitiesSystems.clear();

    for (auto& entity : entitiesRegions) {
        scene->removeEntity(entity);
    }
    entitiesRegions.clear();

    auto& store = client.getStore();

    std::optional<SystemData> currentSystem;
    if (!store.player.location.value().systemId.empty()) {
        auto it = store.galaxy.systems.value().find(store.player.location.value().systemId);
        if (it != store.galaxy.systems.value().end()) {
            currentSystem = it->second;
        }
    }

    std::optional<RegionData> currentRegion;
    if (currentSystem.has_value()) {
        auto it = store.galaxy.regions.value().find(currentSystem.value().regionId);
        if (it != store.galaxy.regions.value().end()) {
            currentRegion = it->second;
        }
    }

    const auto isInsideOfRegion = [&](const SystemData& system) {
        if (!currentSystem || !currentRegion) {
            return false;
        }

        return currentRegion.value().id == system.regionId;
    };

    auto const isCurrentRegion = [&](const RegionData& region) {
        if (!currentRegion) {
            return false;
        }

        return currentRegion.value().id == region.id;
    };

    static const auto colorStarInRegion = Color4{1.0f, 1.0f, 1.0f, 1.0f};
    static const auto colorStarOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.7f};
    static const auto colorConnectionInRegion = GuiColors::secondary * alpha(0.4f);
    static const auto colorConnectionOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.1f};

    std::unordered_map<std::string, std::shared_ptr<ComponentCanvasLines>> regionLines;

    for (const auto& [_, region] : store.galaxy.regions.value()) {
        const Color4& color = isCurrentRegion(region) ? colorConnectionInRegion : colorConnectionOutRegion;

        auto entity = std::make_shared<Entity>();
        entity->translate(Vector3{region.pos.x, 0.0f, region.pos.y});
        auto lines = entity->addComponent<ComponentCanvasLines>(2.0f, color);
        auto label = entity->addComponent<ComponentCanvasLabel>(fontFaceRegular, region.name,
                                                                GuiColors::text * alpha(0.3f), 36.0f);
        label->setVisible(true);
        label->setCentered(true);

        regionLines[region.id] = lines;

        scene->addEntity(entity);
        entitiesRegions.push_back(entity);
    }

    for (const auto& [_, system] : store.galaxy.systems.value()) {
        const Color4& color = isInsideOfRegion(system) ? colorStarInRegion : colorStarOutRegion;

        auto entity = std::make_shared<Entity>();
        entity->translate(Vector3{system.pos.x, 0.0f, system.pos.y});
        entity->addComponent<ComponentCanvasImage>(images.galaxyStar, starSize, color);
        auto label = entity->addComponent<ComponentCanvasLabel>(fontFaceRegular, system.name,
                                                                GuiColors::text * alpha(0.7f), 18.0f);
        label->setVisible(isInsideOfRegion(system));
        label->setOffset(Vector2{starSize.x / 2.0f, 0.0f});

        if (currentSystem.has_value() && system.id == currentSystem.value().id) {
            label->setColor(GuiColors::primary);

            auto marker =
                entity->addComponent<ComponentCanvasImage>(images.currentPosition, markerSize, GuiColors::primary);
            marker->setOffset(Vector2{0.0f, -markerSize.y / 2.0f});
        }

        scene->addEntity(entity);
        entitiesSystems.push_back(entity);

        for (const auto& conn : system.connections) {
            const auto& other = store.galaxy.systems.value().at(conn);
            regionLines.at(system.regionId)
                ->add(Vector3{system.pos.x, 0.0f, system.pos.y}, Vector3{other.pos.x, 0.0f, other.pos.y});
        }
    }

    if (currentSystem.has_value()) {
        entityCamera->getComponent<ComponentCameraTop>()->moveTo(
            Vector3{currentSystem.value().pos.x, 0.0f, currentSystem.value().pos.y});
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

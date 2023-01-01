#include "view_system.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/theme.hpp"
#include "client.hpp"

#define CMP "ViewSystem"

using namespace Engine;

static const Vector2 systemBodySelectable{32.0f, 32.0f};
static const Vector2 systemBodySize{24.0f, 24.0f};

static std::vector<ComponentLines::Line> createRingLines() {
    Color4 color{0.7f, 0.7f, 0.7f, 0.2f};

    std::vector<ComponentLines::Line> rings;
    rings.resize(256);

    const auto step = glm::radians(360.0 / static_cast<double>(rings.size()));
    for (size_t i = 1; i < rings.size(); i++) {
        const auto ii = static_cast<double>(i);
        rings[i].posA = glm::rotateY(Vector3{1.0f, 0.0f, 0.0f}, static_cast<float>(step * (ii - 1)));
        rings[i].posB = glm::rotateY(Vector3{1.0f, 0.0f, 0.0f}, static_cast<float>(step * ii));

        rings[i].colorA = color;
        rings[i].colorB = color;
    }

    rings[0].colorA = color;
    rings[0].colorB = color;
    rings[0].posA = rings.back().posB;
    rings[0].posB = rings.at(1).posA;

    return rings;
}

static std::vector<ComponentLines::Line> createRingLines(const Vector3& origin, float width) {
    static const auto ring = createRingLines();

    std::vector<ComponentLines::Line> lines;
    lines.resize(ring.size());

    for (size_t i = 0; i < ring.size(); i++) {
        lines[i].posA = (ring[i].posA * width) + origin;
        lines[i].posB = (ring[i].posB * width) + origin;
        lines[i].colorA = ring[i].colorA;
        lines[i].colorB = ring[i].colorB;
    }

    return lines;
}

ViewSystem::ViewSystem(const Config& config, Renderer& renderer, Registry& registry, Client& client) :
    config{config},
    renderer{renderer},
    registry{registry},
    client{client},
    skybox{renderer.getVulkan(), Color4{0.1f, 0.1f, 0.1f, 1.0f}},
    scene{&registry.getVoxelShapeCache()} {

    images.systemPlanet = registry.getImages().find("icon_ringed_planet");
    images.systemMoon = registry.getImages().find("icon_world");

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

void ViewSystem::update(const float deltaTime) {
    scene.update(deltaTime);
}

void ViewSystem::render(const Vector2i& viewport) {
    /*Renderer::Options options{};
    options.blurStrength = 0.0f;
    options.exposure = 1.0f;
    renderer.render(viewport, scene, skybox, options);*/
}

void ViewSystem::renderCanvas(const Vector2i& viewport) {
    /*if (input.hover != nullptr) {
        auto pos = camera->worldToScreen(Vector3{input.hover->pos.x, 0.0f, input.hover->pos.y}, true);
        canvas.color(Theme::primary);
        canvas.rectOutline(pos - systemBodySelectable / 2.0f, systemBodySelectable, 1.0f);
    }*/
}

void ViewSystem::renderGui(const Vector2i& viewport) {
    /*gui.modalLoading.setEnabled(loading);
    gui.modalLoading.setProgress(loadingValue);

    nuklear.draw(gui.modalLoading);
    nuklear.draw(gui.contextMenu);*/
}

void ViewSystem::eventMouseMoved(const Vector2i& pos) {
    scene.eventMouseMoved(pos);

    if (!gui.contextMenu.isEnabled()) {
        input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera->getViewport().y) - pos.y});
    }
}

void ViewSystem::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    scene.eventMousePressed(pos, button);
    gui.oldMousePos = pos;

    if (button == MouseButton::Right && gui.contextMenu.isEnabled()) {
        gui.contextMenu.setEnabled(false);
        input.hover = rayCast(Vector2{pos.x, static_cast<float>(camera->getViewport().y) - pos.y});
    }
}

void ViewSystem::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
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

void ViewSystem::load() {
    loading = true;
    loadingValue = 0.1f;
    input.hover = nullptr;
    input.selected = nullptr;

    // Reset entities
    clearEntities();

    // Reset gui
    gui.contextMenu.setEnabled(false);

    // Cancel previous load sequence
    stopToken.stop();

    stopToken = StopToken{};
    fetchCurrentLocation(stopToken);
}

void ViewSystem::fetchCurrentLocation(const StopToken& stop) {
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

        fetchSectors(stop, "");
    });
}

void ViewSystem::fetchSectors(const StopToken& stop, const std::string& token) {
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
        }

        if (res.hasNext) {
            fetchPlanetaryBodiesPage(stop, res.token);
        } else {
            Log::i(CMP, "Received system with {} sector", system.sectors.size());
            loadingValue = 0.7f;
            fetchPlanetaryBodiesPage(stop, "");
        }
    });
}

void ViewSystem::fetchPlanetaryBodiesPage(const StopToken& stop, const std::string& token) {
    MessageFetchPlanetaryBodiesRequest req{};
    req.token = token;
    req.galaxyId = location.galaxyId;
    req.systemId = location.systemId;

    client.send(req, [this, stop](MessageFetchPlanetaryBodiesResponse res) {
        if (stop.shouldStop()) {
            return;
        }

        for (const auto& body : res.bodies) {
            system.bodies[body.id] = body;
        }

        if (res.hasNext) {
            fetchPlanetaryBodiesPage(stop, res.token);
        } else {
            Log::i(CMP, "Received system with {} planetary bodies", system.bodies.size());
            loadingValue = 0.7f;
            updateSystem();
        }
    });
}

void ViewSystem::updateSystem() {
    Log::i(CMP, "Recreating system with {} objects", 0);
    loading = false;
    loadingValue = 1.0f;

    clearEntities();
    createEntitiesPlanets();
}

void ViewSystem::clearEntities() {
    scene.removeEntity(entities.iconPointCloud);
    entities.iconPointCloud = nullptr;

    scene.removeEntity(entities.orbitalRings);
    entities.orbitalRings = nullptr;
}

void ViewSystem::createEntitiesPlanets() {
    entities.iconPointCloud = std::make_shared<Entity>();
    entities.orbitalRings = std::make_shared<Entity>();

    auto pointCloud = entities.iconPointCloud->addComponent<ComponentIconPointCloud>();
    auto orbitalRings = entities.orbitalRings->addComponent<ComponentLines>();

    for (const auto& [bodyId, body] : system.bodies) {
        Color4 color{1.0f, 1.0f, 1.0f, 1.0f};

        if (body.isMoon) {
            pointCloud->add(Vector3{body.pos.x, 0.0f, body.pos.y}, systemBodySize, color, images.systemMoon);
            const auto& parent = system.bodies.at(*body.parent);
            orbitalRings->append(
                createRingLines(Vector3{parent.pos.x, 0.0f, parent.pos.y}, glm::length(parent.pos - body.pos)));
        } else {
            pointCloud->add(Vector3{body.pos.x, 0.0f, body.pos.y}, systemBodySize, color, images.systemPlanet);
            orbitalRings->append(createRingLines({0.0f, 0.0f, 0.0f}, glm::length(body.pos)));
        }
    }

    scene.addEntity(entities.iconPointCloud);
    scene.addEntity(entities.orbitalRings);
}

const SystemData* ViewSystem::rayCast(const Vector2& mousePos) {
    /*std::vector<std::tuple<float, const SystemData*>> found;

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

    return std::get<1>(found.front());*/
    return nullptr;
}

void ViewSystem::onEnter() {
}

void ViewSystem::onExit() {
}

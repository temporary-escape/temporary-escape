#include "ViewMap.hpp"

#include "../Scene/ComponentLines.hpp"
#include "../Scene/ComponentPointCloud.hpp"
#include "Messages.hpp"
#include "Widgets.hpp"

using namespace Scissio;

ViewMap::ViewMap(const Config& config, Network::Client& client, Store& store, AssetManager& assetManager)
    : Store::Listener(store), config(config), client(client),
      assetManager(assetManager), viewport{0}, cameraMove{false}, mousePos{0}, mousePosDelta{0}, loading{true} {

    onNotify(store.regions, [this](Store& store) {
        store.systems.value().clear();
        this->client.send(0, MessageSystemsRequest{});
    });

    onNotify(store.systems, [this](Store& store) {
        if (galaxy) {
            // TODO: remove entity
        }

        const auto textureFlare = this->assetManager.find<BasicTexture>("star_flare");

        galaxy = scene.addEntity();

        auto stars = galaxy->addComponent<ComponentPointCloud>(textureFlare);
        auto lines = galaxy->addComponent<ComponentLines>();

        stars->reserve(store.systems.value().size());

        std::unordered_map<uint64_t, const RegionDto*> regions;
        for (const auto& region : store.regions.value()) {
            regions.insert(std::make_pair(region.id, &region));
        }

        std::unordered_map<uint64_t, Vector2> positions;
        positions.reserve(store.systems.value().size());

        for (const auto& system : store.systems.value()) {
            positions.insert(std::make_pair(system.id, system.pos));
        }

        for (const auto& system : store.systems.value()) {
            const auto* region = regions.at(system.regionId);

            const auto starColor = hsvToRgb({region->hue * 360.0f, 0.75f, 1.0f, 1.0f});
            const auto lineColor = starColor * 0.3f;

            const auto pos = Vector3{system.pos.x, 0.0f, system.pos.y};
            stars->insert(pos, 3.0f, starColor);

            for (const auto& destinationId : system.links) {
                const auto it = positions.find(destinationId);
                if (it != positions.end()) {
                    const auto dst = Vector3{it->second.x, 0.0f, it->second.y};
                    lines->insert(pos, dst, lineColor);
                }
            }
        }

        loading = false;
    });

    camera.translate({0.0f, 100.0f, 0.0f});
    camera.rotate({1.0f, 0.0f, 0.0f}, -90.0f);

    store.systems.value().clear();
    client.send(0, MessageRegionsRequest{});
}

void ViewMap::update(const Vector2i& viewport) {
    this->viewport = viewport;

    {
        static constexpr auto d = 0.1f;

        if (cameraMove) {
            Vector3 dir{mousePosDelta.x * d, mousePosDelta.y * -d, 0.0f};
            camera.translate(dir);
            mousePosDelta = {0.0f, 0.0f};
        }

        camera.setProjection(viewport, 70.0f);
    }

    scene.update();
}

void ViewMap::render(const Vector2i& viewport, Renderer& renderer) {
    RenderOptions options;
    options.withSkybox = false;

    renderer.setView(camera.getViewMatrix());
    renderer.setProjection(camera.getProjectionMatrix());
    renderer.render(scene, options);
}

void ViewMap::renderCanvas(const Vector2i& viewport, Canvas2D& canvas, GuiContext& gui) {
    if (loading) {
        Widgets::modal(gui, "Please wait!", "Loading galaxy map...");
    }
}

void ViewMap::eventMouseMoved(const Vector2i& pos) {
    if (cameraMove) {
        mousePosDelta = this->mousePos - pos;
    }

    this->mousePos = pos;
}

void ViewMap::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    mousePos = pos;
    cameraMove |= button == MouseButton::Right;
}

void ViewMap::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    mousePos = pos;
    cameraMove &= !(button == MouseButton::Right);
}

void ViewMap::eventKeyPressed(const Key key, const Modifiers modifiers) {
}

void ViewMap::eventKeyReleased(const Key key, const Modifiers modifiers) {
}

void ViewMap::eventMouseScroll(int xscroll, int yscroll) {
}

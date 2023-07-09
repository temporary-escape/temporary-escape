#pragma once

#include "../assets/assets_manager.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../gui/gui_modal_loading.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewSystem : public View {
public:
    struct Gui {
        explicit Gui(const Config& config, AssetsManager& assetsManager);

        GuiModalLoading modalLoading;
    };

    explicit ViewSystem(Game& parent, const Config& config, Renderer& renderer, AssetsManager& assetsManager,
                        Client& client, Gui& gui, FontFamily& font);
    ~ViewSystem() = default;

    void update(float deltaTime) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void eventEntitySelected(uint32_t id) override;
    void onEnter() override;
    void onExit() override;
    Scene* getScene() override;

    void reset();
    void reset(const std::string& galaxyId, const std::string& systemId);

private:
    void load();
    void finalize();
    void clearEntities();

    Game& parent;
    const Config& config;
    Renderer& renderer;
    AssetsManager& assetsManager;
    Client& client;
    FontFamily& font;
    Gui& gui;
    std::unique_ptr<Scene> scene;

    struct {
        std::string galaxyId;
        std::string systemId;
        std::string sectorId;
        bool isCurrent{false};
    } location;

    struct {
        std::string name;
        std::unordered_map<std::string, PlanetData> planets;
        std::unordered_map<std::string, SectorData> sectors;
        std::vector<std::variant<PlanetData*, SectorData*>> bodies;
    } system;

    struct {
        std::string name;
    } galaxy;

    struct {
        Entity camera;
        std::vector<Entity> bodies;
        std::vector<Entity> orbits;
        Entity icons;
        Entity cursor;
        Entity positions;
        Entity names;
    } entities;

    struct {
        ImagePtr systemPlanet;
        ImagePtr systemMoon;
        ImagePtr iconSelect;
    } images;

    struct {
        TexturePtr star;
        TexturePtr starLow;
        TexturePtr starHigh;
    } textures;

    struct {
        const SystemData* hover{nullptr};
        const SystemData* selected{nullptr};
    } input;

    bool loading{false};
    std::atomic<float> loadingValue{0.0f};
    StopToken stopToken;
    Future<void> futureLoad;
};
} // namespace Engine

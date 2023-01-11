#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../gui/gui_context_menu.hpp"
#include "../gui/gui_modal_loading.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class Client;

class ViewGalaxy : public View {
public:
    explicit ViewGalaxy(const Config& config, Renderer& renderer, Registry& registry, Client& client);
    ~ViewGalaxy() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport) override;
    void renderCanvas(const Vector2i& viewport) override;
    void renderGui(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;

    void load();

private:
    void fetchCurrentLocation(const StopToken& stop);
    void fetchGalaxyInfo(const StopToken& stop);
    void fetchFactionsPage(const StopToken& stop, const std::string& token);
    void fetchSystemsPage(const StopToken& stop, const std::string& token);
    void fetchRegionsPage(const StopToken& stop, const std::string& token);
    void updateGalaxy();
    void clearEntities();
    void createEntitiesRegions();
    void createInputIndices();
    void clearInputIndices();
    const SystemData* rayCast(const Vector2& mousePos);

    const Config& config;
    Renderer& renderer;
    Registry& registry;
    Client& client;
    Skybox skybox;
    Scene scene;

    struct {
        std::string galaxyId;
        std::string systemId;
        std::string sectorId;
    } location;

    struct {
        std::string name;
        std::unordered_map<std::string, SystemData> systems;
        std::unordered_map<std::string, RegionData> regions;
    } galaxy;

    std::unordered_map<std::string, FactionData> factions;

    struct {
        std::shared_ptr<Entity> camera;
        std::unordered_map<std::string, EntityPtr> regions;
    } entities;

    struct {
        TexturePtr systemStar;
    } textures;

    struct {
        std::vector<const SystemData*> indices;
        std::vector<Vector3> positions;
        const SystemData* hover{nullptr};
        const SystemData* selected{nullptr};
    } input;

    struct {
        GuiModalLoading modalLoading{"Galaxy Map"};
        GuiContextMenu contextMenu;

        Vector2i oldMousePos;
    } gui;

    bool loading{false};
    float loadingValue{0.0f};
    StopToken stopToken;
};
} // namespace Engine

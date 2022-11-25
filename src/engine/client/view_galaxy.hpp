#pragma once

#include "../assets/registry.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/skybox.hpp"
#include "../gui/gui_modal_loading.hpp"
#include "../scene/scene.hpp"
#include "../server/world.hpp"
#include "view.hpp"

namespace Engine {
class Client;

class ViewGalaxy : public View {
public:
    explicit ViewGalaxy(const Config& config, VulkanDevice& vulkan, Scene::Pipelines& scenePipelines,
                        Registry& registry, Canvas& canvas, FontFamily& font, Nuklear& nuklear, Client& client);
    ~ViewGalaxy() = default;

    void update(float deltaTime) override;
    void render(const Vector2i& viewport, Renderer& renderer) override;
    void renderCanvas(const Vector2i& viewport) override;
    void eventUserInput(const UserInput::Event& event) override;

    void load();

private:
    void fetchCurrentLocation();
    void fetchGalaxyInfo();
    void fetchFactionsPage(const std::string& token);
    void fetchSystemsPage(const std::string& token);
    void fetchRegionsPage(const std::string& token);
    void updateGalaxy();

    const Config& config;
    VulkanDevice& vulkan;
    Registry& registry;
    Canvas& canvas;
    FontFamily& font;
    Nuklear& nuklear;
    Client& client;
    Skybox skybox;
    Scene scene;

    GuiModalLoading modalLoading;

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
        EntityPtr systems;
    } entities;

    struct {
        TexturePtr systemStar;
    } textures;

    bool loading{false};
    float loadingValue{0.0f};
};
} // namespace Engine

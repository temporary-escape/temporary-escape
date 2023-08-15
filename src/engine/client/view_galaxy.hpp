#pragma once

#include "../assets/assets_manager.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../gui/gui_modal_loading.hpp"
#include "../math/voronoi_diagram.hpp"
#include "../scene/scene.hpp"
#include "../server/schemas.hpp"
#include "../utils/stop_token.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewGalaxy : public View {
public:
    struct Gui {
        explicit Gui(const Config& config, AssetsManager& assetsManager);

        GuiModalLoading modalLoading;
    };

    explicit ViewGalaxy(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                        VoxelShapeCache& voxelShapeCache, Client& client, Gui& gui, FontFamily& font);
    ~ViewGalaxy();

    void update(float deltaTime) override;
    void renderCanvas(Canvas& canvas, const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;
    void onEnter() override;
    void onExit() override;
    Scene* getScene() override;

private:
    void load();
    void finalize();
    void clearEntities();
    /*void fetchCurrentLocation(const StopToken& stop);
    void fetchGalaxyInfo(const StopToken& stop);
    void fetchFactionsPage(const StopToken& stop, const std::string& token);
    void fetchSystemsPage(const StopToken& stop, const std::string& token);
    void fetchRegionsPage(const StopToken& stop, const std::string& token);
    void updateGalaxy();
    void clearEntities();
    void createEntitiesRegions();
    void calculateBackground();
    void createBackground(const VoronoiResult& voronoi);*/

    Game& parent;
    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    VoxelShapeCache& voxelShapeCache;
    Client& client;
    Gui& gui;
    FontFamily& font;
    std::unique_ptr<Scene> scene;

    struct {
        Entity camera;
        Entity regions;
        Entity systems;
        Entity cursor;
        Entity voronoi;
        Entity names;
        Entity currentPos;
    } entities;

    struct {
        TexturePtr systemStar;
    } textures;

    struct {
        ImagePtr iconSelect;
        ImagePtr iconCurrentPos;
    } images;

    bool loading{false};
    Future<void> futureLoad;
};
} // namespace Engine

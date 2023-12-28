#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Future.hpp"
#include "../Graphics/Canvas.hpp"
#include "../Graphics/Nuklear.hpp"
#include "../Gui/GuiModalLoading.hpp"
#include "../Math/VoronoiDiagram.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Schemas.hpp"
#include "../Utils/StopToken.hpp"
#include "View.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewGalaxy : public View {
public:
    explicit ViewGalaxy(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                        VoxelShapeCache& voxelShapeCache, Client& client, FontFamily& font);
    ~ViewGalaxy();

    void update(float deltaTime, const Vector2i& viewport) override;
    void renderCanvas(Canvas2& canvas, const Vector2i& viewport) override;
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

    Game& parent;
    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    VoxelShapeCache& voxelShapeCache;
    Client& client;
    FontFamily& font;
    GuiModalLoading guiModalLoading;
    std::unique_ptr<Scene> scene;
    std::optional<Entity> selectedEntity;

    struct {
        Entity camera;
        Entity regions;
        Entity systems;
        Entity voronoi;
        Entity names;
        Entity currentPos;
        std::unordered_map<Entity::Handle, std::string> icons;
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

#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Future.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Schemas.hpp"
#include "../Utils/StopToken.hpp"
#include "View.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewSystem : public View {
public:
    explicit ViewSystem(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager,
                        AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache, FontFamily& font,
                        Client& client);
    ~ViewSystem();

    void update(float deltaTime, const Vector2i& viewport) override;
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
    void showContextMenu(const Vector2i& pos, const std::string& sectorId);
    void doWarpTo(const std::string& sectorId);

    const Config& config;
    VulkanRenderer& vulkan;
    GuiManager& guiManager;
    AssetsManager& assetsManager;
    VoxelShapeCache& voxelShapeCache;
    Client& client;
    FontFamily& font;
    std::unique_ptr<Scene> scene;

    struct {
        Entity camera;
        std::unordered_map<EntityId, std::string> icons;
    } entities;

    struct {
        ImagePtr systemPlanet;
        ImagePtr systemMoon;
        ImagePtr iconSelect;
        ImagePtr currentPos;
        ImagePtr info;
        ImagePtr view;
        ImagePtr travel;
    } icons;

    struct {
        TexturePtr star;
        TexturePtr starLow;
        TexturePtr starHigh;
    } textures;

    bool loading{false};
    Future<void> futureLoad;
};
} // namespace Engine

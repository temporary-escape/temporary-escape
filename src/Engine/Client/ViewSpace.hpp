#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Schemas.hpp"
#include "View.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API GuiManager;
class ENGINE_API GuiWindowShipToolbar;
class ENGINE_API GuiWindowShipStatus;
class ENGINE_API GuiWindowCurrentLocation;
class ENGINE_API GuiWindowSceneOverview;

class ENGINE_API ViewSpace : public View {
public:
    explicit ViewSpace(const Config& config, VulkanRenderer& vulkan, GuiManager& guiManager,
                       AssetsManager& assetsManager, VoxelShapeCache& voxelShapeCache, FontFamily& font,
                       Client& client);
    ~ViewSpace() = default;

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
    void updateGuiCurrentLocation();
    void showContextMenu(const Vector2i& mousePos, const Entity& entity);
    void showContextMenu(const Vector2i& mousePos);
    void doTargetEntity(const Entity& entity);
    void doApproachEntity(const Entity& entity);
    void doGoHere(const Vector3& direction);
    void doOrbitEntity(const Entity& entity, float radius);
    void doKeepAtDistanceEntity(const Entity& entity, float disrance);
    void doStopMovement();
    void renderCanvasSelectedEntity(Canvas& canvas, const Vector2i& viewport, const Scene& scene,
                                    const ComponentCamera& camera);
    void renderCanvasApproaching(Canvas& canvas, const Vector2i& viewport, const Scene& scene,
                                 const ComponentCamera& camera);

    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    VoxelShapeCache& voxelShapeCache;
    GuiManager& guiManager;
    FontFamily& font;
    Client& client;

    struct {
        GuiWindowShipToolbar* toolbar{nullptr};
        GuiWindowShipStatus* status{nullptr};
        GuiWindowCurrentLocation* location{nullptr};
        GuiWindowSceneOverview* overview{nullptr};
    } gui;

    struct {
        ImagePtr nested;
        ImagePtr approach;
        ImagePtr orbit;
        ImagePtr distance;
        ImagePtr info;
        ImagePtr attack;
    } icons;

    /*struct {
        bool forward{false};
        bool backwards{false};
        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};
        bool update{false};
        bool boost{false};
    } control;*/
};
} // namespace Engine

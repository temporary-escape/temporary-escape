#pragma once

#include "../assets/assets_manager.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../gui/gui_context_menu.hpp"
#include "../scene/scene.hpp"
#include "../server/schemas.hpp"
#include "view.hpp"

namespace Engine {
class ENGINE_API Client;
class ENGINE_API Game;

class ENGINE_API ViewSpace : public View {
public:
    explicit ViewSpace(Game& parent, const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
                       VoxelShapeCache& voxelShapeCache, FontFamily& font, Client& client);
    ~ViewSpace() = default;

    void update(float deltaTime) override;
    void renderCanvas(Canvas& canvas, const Vector2i& viewport) override;
    void renderNuklear(Nuklear& nuklear, const Vector2i& viewport) override;
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
    void doTargetEntity(const Entity& entity);
    void renderCanvasSelectedEntity(Canvas& canvas, const Scene& scene, const ComponentCamera& camera);

    Game& parent;
    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    VoxelShapeCache& voxelShapeCache;
    FontFamily& font;
    Client& client;
    GuiContextMenu guiContextMenu;

    struct {
        bool forward{false};
        bool backwards{false};
        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};
        bool update{false};
        bool boost{false};
    } control;
};
} // namespace Engine

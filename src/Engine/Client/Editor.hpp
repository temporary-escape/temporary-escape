#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Config.hpp"
#include "../Font/FontFamily.hpp"
#include "../Future.hpp"
#include "../Graphics/Canvas.hpp"
#include "../Graphics/Nuklear.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Gui/GuiBlockSelector.hpp"
#include "../Gui/GuiContextMenu.hpp"
#include "../Gui/GuiMainMenu.hpp"
#include "../Gui/GuiSideMenu.hpp"
#include "../Server/Server.hpp"
#include "ViewBuild.hpp"

namespace Engine {
class ENGINE_API Database;

class ENGINE_API Editor : public UserInput {
public:
    explicit Editor(const Config& config, VulkanRenderer& vulkan, AudioContext& audio, AssetsManager& assetsManager,
                    VoxelShapeCache& voxelShapeCache, FontFamily& font);
    virtual ~Editor();

    void update(float deltaTime);
    void render(VulkanCommandBuffer& vkb, Renderer& renderer, const Vector2i& viewport);
    void renderCanvas(Canvas& canvas, Nuklear& nuklear, const Vector2i& viewport);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    const Config& config;
    VulkanRenderer& vulkan;
    AssetsManager& assetsManager;
    FontFamily& font;
    GuiMainMenu guiMainMenu;
    ViewBuild view;
};
} // namespace Engine

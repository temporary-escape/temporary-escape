#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../font/font_family.hpp"
#include "../future.hpp"
#include "../graphics/canvas.hpp"
#include "../graphics/nuklear.hpp"
#include "../graphics/renderer.hpp"
#include "../gui/gui_block_selector.hpp"
#include "../gui/gui_context_menu.hpp"
#include "../gui/gui_side_menu.hpp"
#include "../server/server.hpp"
#include "view_build.hpp"

namespace Engine {
class ENGINE_API Database;

class ENGINE_API Editor : public UserInput {
public:
    explicit Editor(const Config& config, VulkanRenderer& vulkan, AssetsManager& assetsManager,
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
    AssetsManager& assetsManager;
    FontFamily& font;
    ViewBuild view;
};
} // namespace Engine

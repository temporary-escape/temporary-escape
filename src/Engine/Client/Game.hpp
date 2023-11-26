#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Assets/VoxelShapeCache.hpp"
#include "../Config.hpp"
#include "../Database/DatabaseRocksdb.hpp"
#include "../Font/FontFamily.hpp"
#include "../Future.hpp"
#include "../Graphics/Canvas.hpp"
#include "../Graphics/Nuklear.hpp"
#include "../Graphics/Renderer.hpp"
#include "../Graphics/RendererPlanetSurface.hpp"
#include "../Graphics/RendererSkybox.hpp"
#include "../Gui/GuiMainMenu.hpp"
#include "../Server/Server.hpp"
#include "Client.hpp"
#include "Stats.hpp"
#include "ViewBuild.hpp"
#include "ViewGalaxy.hpp"
#include "ViewSpace.hpp"
#include "ViewSystem.hpp"

namespace Engine {
class ENGINE_API Database;

class ENGINE_API Game : public UserInput {
public:
    explicit Game(const Config& config, VulkanRenderer& vulkan, RendererSkybox& rendererSkybox,
                  RendererPlanetSurface& rendererPlanetSurface, AssetsManager& assetsManager,
                  VoxelShapeCache& voxelShapeCache, FontFamily& font, Client& client);
    virtual ~Game();

    bool isReady() const;
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

    void switchToGalaxyMap();
    void switchToSystemMap();
    void switchToSystemMap(const std::string& galaxyId, const std::string& systemId);

private:
    const Config& config;
    VulkanRenderer& vulkan;
    RendererSkybox& rendererSkybox;
    RendererPlanetSurface& rendererPlanetSurface;
    AssetsManager& assetsManager;
    FontFamily& font;
    Client& client;
    GuiMainMenu guiMainMenu;

    std::unique_ptr<ViewBuild> viewBuild;
    std::unique_ptr<ViewSpace> viewSpace;
    std::unique_ptr<ViewGalaxy> viewGalaxy;
    std::unique_ptr<ViewSystem> viewSystem;
    View* view{nullptr};
};
} // namespace Engine

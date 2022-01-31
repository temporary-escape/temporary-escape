#include "../Gui/GuiContext.hpp"
#include "ViewMap.hpp"
#include "ViewSpace.hpp"

namespace Scissio {
class SCISSIO_API ViewRoot : public View {
public:
    explicit ViewRoot(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                      Client& client);

    void render(const Vector2i& viewport) override;
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;

private:
    const Config& config;
    Canvas2D& canvas;
    Client& client;
    GuiContext gui;
    ViewSpace viewSpace;
    ViewMap viewMap;

    bool mapActive;
};
} // namespace Scissio

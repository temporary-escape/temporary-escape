#pragma once

#include "../Assets/Icon.hpp"
#include "../Gui/GuiContext.hpp"

namespace Scissio {
class SCISSIO_API WindowBuildMenu : public GuiWindow {
public:
    explicit WindowBuildMenu(GuiContext& ctx, AssetManager& assetManager);
    virtual ~WindowBuildMenu() = default;
    WindowBuildMenu(const WindowBuildMenu& other) = delete;
    WindowBuildMenu& operator=(const WindowBuildMenu& other) = delete;

    void render() override;

private:
    struct Item {
        std::string title;
        IconPtr icon;
    };

    std::vector<Item> items;
};
} // namespace Scissio

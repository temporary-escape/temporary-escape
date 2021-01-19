#pragma once

#include "../Assets/Block.hpp"
#include "../Gui/GuiContext.hpp"

namespace Scissio {
class SCISSIO_API WindowBlockSelector : public GuiWindow {
public:
    explicit WindowBlockSelector(GuiContext& ctx, AssetManager& assetManager);
    virtual ~WindowBlockSelector() = default;
    WindowBlockSelector(const WindowBlockSelector& other) = delete;
    WindowBlockSelector& operator=(const WindowBlockSelector& other) = delete;

    void render() override;

private:
    struct Category {
        std::string label;
        std::list<BlockPtr> choices;
    };

    std::list<Category> categories;
    std::list<Category>::const_iterator category;
};
} // namespace Scissio

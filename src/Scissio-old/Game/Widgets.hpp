#pragma once

#include "../Gui/GuiContext.hpp"
#include "Schemas.hpp"

namespace Scissio::Widgets {
struct SidebarItem {
    IconPtr icon;
    std::string title;
    std::function<void()> callback;
    bool toggleable{true};

    bool active{false};
};

struct BlockSelectorItem {
    const BlockDto* block{nullptr};
    ImagePtr thumbnail{nullptr};
};

struct BlockSelectorCategory {
    std::string label;
    std::vector<BlockSelectorItem> items;
};

struct BlockSelectorData {
    std::vector<BlockSelectorCategory> categories;

    size_t selected{0};
    const BlockDto* block{nullptr};
};

extern SCISSIO_API void modal(GuiContext& gui, const std::string& title, const std::string& message);
extern SCISSIO_API void sidebar(GuiContext& gui, std::vector<SidebarItem>& items);
extern SCISSIO_API void blockSelector(GuiContext& gui, BlockSelectorData& data,
                                      const std::function<void(const BlockDto&)>& callback);
} // namespace Scissio::Widgets

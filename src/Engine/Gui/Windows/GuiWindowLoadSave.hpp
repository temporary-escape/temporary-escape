#pragma once

#include "../../Server/Matchmaker.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowLoadSave : public GuiWindow {
public:
    using OnLoadCallback = std::function<void(const Path&)>;
    using OnCreateCallback = GuiWidgetButton::OnClickCallback;
    using OnCloseCallback = GuiWidgetButton::OnClickCallback;

    explicit GuiWindowLoadSave(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, GuiManager& guiManager,
                               const Path& dir);

    void loadInfos();
    void setOnLoad(OnLoadCallback callback);

private:
    Path dir;
    GuiWidgetTextInput* inputSearch;
    GuiWidgetGroup* group;
    OnLoadCallback onLoad;
};
} // namespace Engine

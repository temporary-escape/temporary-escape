#pragma once

#include "../../Database/SaveInfo.hpp"
#include "../../Server/MatchmakerClient.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowLoadSave : public GuiWindow {
public:
    using OnLoadCallback = std::function<void(const Path&)>;
    using OnCreateCallback = GuiWidgetButton::OnClickCallback;

    explicit GuiWindowLoadSave(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, GuiManager& guiManager,
                               Path dir);

    void loadInfos();
    void setOnLoad(OnLoadCallback callback);
    void setOnCreate(OnCreateCallback callback);
    void setMode(MultiplayerMode value);
    [[nodiscard]] MultiplayerMode getMode() const {
        return mode;
    }

private:
    Path dir;
    GuiWidgetTextInput* inputSearch;
    GuiWidgetGroup* group;
    OnLoadCallback onLoad;
    Path selected;
    SaveInfo selectedInfo;
    GuiWidgetButton* buttonCreate{nullptr};
    GuiWidgetButton* buttonDelete{nullptr};
    GuiWidgetButton* buttonPlay{nullptr};
    MultiplayerMode mode{MultiplayerMode::Singleplayer};
};
} // namespace Engine

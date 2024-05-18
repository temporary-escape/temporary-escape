#pragma once

#include "../../Server/Matchmaker.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowCreateSave : public GuiWindow {
public:
    struct Form {
        Path path;
        uint64_t seed;
    };

    using OnCreateCallback = std::function<void(const Form&)>;
    using OnCloseCallback = GuiWidgetButton::OnClickCallback;

    explicit GuiWindowCreateSave(const FontFamily& fontFamily, int fontSize, GuiManager& guiManager, Path dir);

    void setOnCreate(OnCreateCallback callback);
    void setOnClose(OnCloseCallback callback);

private:
    GuiManager& guiManager;
    Path dir;
    GuiWidgetButton* buttonClose;
    GuiWidgetButton* buttonCreate;
    OnCreateCallback onCreate;
    GuiWidgetTextInput* inputName;
    GuiWidgetTextInput* inputSeed;
};
} // namespace Engine

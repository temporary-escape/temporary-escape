#pragma once

#include "../../Database/SaveInfo.hpp"
#include "../../Server/MatchmakerClient.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowLoadStatus : public GuiWindow {
public:
    explicit GuiWindowLoadStatus(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setStatus(const std::string& message, float progress);

private:
    GuiWidgetText* text;
    GuiWidgetProgressBar* progressBar;
};
} // namespace Engine

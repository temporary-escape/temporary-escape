#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowModal : public GuiWindow {
public:
    using OnClickCallback = std::function<void(const std::string&)>;

    GuiWindowModal(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, std::string title, std::string text,
                   const std::vector<std::string>& choices, int timeout);

    void update(const Vector2i& viewport) override;
    void setOnClickCallback(const OnClickCallback& value);
    void setChoices(const std::vector<std::string>& choices);
    void setText(std::string value);

private:
    std::string text;
    OnClickCallback onClickCallback{nullptr};
    int timeout{-1};
    GuiWidgetProgressBar* progressBar{nullptr};
    GuiWidgetRow* widgetRowText{nullptr};
    GuiWidgetRow* widgetRowChoices{nullptr};
    GuiWidgetText* widgetText{nullptr};
    std::chrono::steady_clock::time_point start;
    float lastWidth{0.0f};
};
} // namespace Engine

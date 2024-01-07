#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowModal : public GuiWindow {
public:
    using OnClickCallback = std::function<void(const std::string&)>;

    GuiWindowModal(const FontFamily& fontFamily, int fontSize, std::string title, std::string text,
                   const std::vector<std::string>& choices, int timeout);

    void update(const Vector2i& viewport) override;
    void setOnClickCallback(const OnClickCallback& value);

private:
    OnClickCallback onClickCallback{nullptr};
    int timeout{-1};
    GuiWidgetProgressBar* progressBar{nullptr};
    std::chrono::steady_clock::time_point start;
};
} // namespace Engine

#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowModal : public GuiWindow {
public:
    using OnClickCallback = std::function<void(const std::string&)>;

    GuiWindowModal(const FontFamily& fontFamily, int fontSize, std::string title, std::string text,
                   const std::vector<std::string>& choices);

    void setOnClickCallback(const OnClickCallback& value);

private:
    OnClickCallback onClickCallback{nullptr};
};
} // namespace Engine

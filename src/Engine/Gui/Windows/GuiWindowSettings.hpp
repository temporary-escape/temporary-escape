#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowSettings : public GuiWindow2 {
public:
    using OnSubmitCallback = std::function<void(bool)>;

    explicit GuiWindowSettings(const FontFamily& fontFamily, int fontSize, VulkanRenderer& vulkan, Config& config);

    void setOnSubmit(OnSubmitCallback callback);
    void reset();

private:
    VulkanRenderer& vulkan;
    Config& config;
    OnSubmitCallback onSubmitCallback;
    GuiWidgetCombo* comboResolution{nullptr};
};
} // namespace Engine

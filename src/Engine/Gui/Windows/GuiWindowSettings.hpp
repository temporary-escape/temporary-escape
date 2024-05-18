#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowSettings : public GuiWindow {
public:
    using OnApplyCallback = std::function<void()>;
    using OnSubmitCallback = std::function<void(bool)>;

    explicit GuiWindowSettings(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, VulkanRenderer& vulkan,
                               Config& config, GuiManager& guiManager);

    void setOnApply(OnApplyCallback callback);
    void setOnSubmit(OnSubmitCallback callback);
    void reset();

private:
    void onSave();
    void onCancel();
    void onModalClick(const std::string& choice);

    VulkanRenderer& vulkan;
    Config& config;
    GuiManager& guiManager;
    Config configBackup;
    OnApplyCallback onApplyCallback;
    OnSubmitCallback onSubmitCallback;
    GuiWidgetCombo* comboResolution{nullptr};
};
} // namespace Engine

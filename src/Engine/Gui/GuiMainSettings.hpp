#pragma once

#include "GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiMainSettings : public GuiWindow {
public:
    explicit GuiMainSettings();
    ~GuiMainSettings() override = default;

    void setSettings(const Config& value) {
        config = value;
    }
    void setVideoModes(std::vector<Vector2i> value);
    template <typename Fn> void setOnApply(Fn&& fn) {
        onApply = std::forward<Fn>(fn);
    }

private:
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;
    void drawLayoutTabs(Nuklear& nuklear);
    void drawLayoutGraphics(Nuklear& nuklear);
    void drawBottomBar(Nuklear& nuklear);

    Config config;
    size_t tabsValue{0};
    std::vector<Vector2i> videoModes;
    std::vector<std::string> videoModesStr;
    size_t videoModeChosen{0};
    std::function<void(const Config&)> onApply;
};
} // namespace Engine

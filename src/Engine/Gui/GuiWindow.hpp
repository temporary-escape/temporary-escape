#pragma once

#include "../Graphics/Nuklear.hpp"
#include "Widgets/GuiWidgetButton.hpp"
#include "Widgets/GuiWidgetButtonToggle.hpp"
#include "Widgets/GuiWidgetCheckbox.hpp"
#include "Widgets/GuiWidgetCombo.hpp"
#include "Widgets/GuiWidgetContextButton.hpp"
#include "Widgets/GuiWidgetImage.hpp"
#include "Widgets/GuiWidgetImageToggle.hpp"
#include "Widgets/GuiWidgetLabel.hpp"
#include "Widgets/GuiWidgetLayout.hpp"
#include "Widgets/GuiWidgetProgressBar.hpp"
#include "Widgets/GuiWidgetRow.hpp"
#include "Widgets/GuiWidgetTabs.hpp"
#include "Widgets/GuiWidgetTemplateRow.hpp"
#include "Widgets/GuiWidgetTextInput.hpp"

namespace Engine {
class ENGINE_API GuiWindow : public GuiContext, public GuiWidgetLayout {
public:
    GuiWindow(const FontFamily& fontFamily, int fontSize);
    virtual ~GuiWindow() = default;

    virtual void update(const Vector2i& viewport);
    void draw() override;

    void setTitle(std::string value);
    const std::string& getTitle() const {
        return title;
    }

    void setSize(const Vector2& value);
    const Vector2& getSize() const {
        return size;
    }
    void setPos(const Vector2& value);
    const Vector2& getPos() const {
        return pos;
    }
    void setEnabled(bool value);
    bool isEnabled() const {
        return enabled;
    }

    void setCentered(bool value);
    void setBordered(bool value);
    void setBackground(bool value);
    void setNoScrollbar(bool value);
    void setHeader(bool value);
    void setTransparent(bool value);
    void setDynamic(bool value);
    void setNoInput(bool value);
    void setHeaderSuccess(bool value);
    void setHeaderDanger(bool value);

private:
    std::string id;
    std::string title{"Window"};
    Vector2 size;
    Vector2 pos;
    GuiContext::Flags flags{0};
    bool enabled{false};
    bool centered{true};
};
} // namespace Engine

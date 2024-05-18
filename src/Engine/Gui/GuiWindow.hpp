#pragma once

#include "../Graphics/Nuklear.hpp"
#include "GuiContext.hpp"
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
class ENGINE_API GuiWindow : public GuiWidgetLayout {
public:
    using OnCloseCallback = std::function<void()>;

    GuiWindow(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);
    virtual ~GuiWindow() = default;

    virtual void update(const Vector2i& viewport);
    void draw() override;

    void setTitle(const std::string_view& value);
    const std::string& getTitle() const {
        return title;
    }
    const std::string& getId() const {
        return id;
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

    void setOpacity(float value);
    float getOpacity() const {
        return opacity;
    }

    void setCentered(bool value);
    void setBordered(bool value);
    void setBackground(bool value);
    void setNoScrollbar(bool value);
    void setHeader(bool value);
    void setDynamic(bool value);
    void setNoInput(bool value);
    void setHeaderPrimary(bool value);
    void setHeaderSuccess(bool value);
    void setHeaderDanger(bool value);
    void setCloseable(bool value);
    int getFontSize() const {
        return ctx.getFontSize();
    }
    float getPadding() const {
        return padding;
    }
    void setPadding(float value) {
        padding = value;
    }
    void close();
    void setOnClose(OnCloseCallback value);

private:
    GuiContext& ctx;
    std::string id;
    std::string title{"Window"};
    Vector2 size{200.0f, 200.0f};
    Vector2 pos{100.0f, 100.0f};
    GuiContext::Flags flags{0};
    bool enabled{false};
    bool centered{true};
    float opacity{1.0f};
    float padding{2.0f};
    int ignoreInput{0};
    OnCloseCallback onCloseCallback;
};
} // namespace Engine

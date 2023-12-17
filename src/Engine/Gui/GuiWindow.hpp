#pragma once

#include "../Graphics/Nuklear.hpp"
#include "Widgets/GuiWidgetButton.hpp"
#include "Widgets/GuiWidgetButtonToggle.hpp"
#include "Widgets/GuiWidgetCheckbox.hpp"
#include "Widgets/GuiWidgetCombo.hpp"
#include "Widgets/GuiWidgetLabel.hpp"
#include "Widgets/GuiWidgetLayout.hpp"
#include "Widgets/GuiWidgetRow.hpp"
#include "Widgets/GuiWidgetTabs.hpp"
#include "Widgets/GuiWidgetTextInput.hpp"

namespace Engine {
class ENGINE_API GuiWindow {
public:
    GuiWindow();
    virtual ~GuiWindow() = default;

    void draw(Nuklear& nuklear, const Vector2& viewport);

    const Vector2& getSize() const;
    const Vector2& getPos() const;
    Nuklear::Flags getFlags() const;
    void setSize(const Vector2& value);
    void setEnabled(bool value);
    void setPos(const Vector2& value);
    void setFlags(unsigned int value);
    const std::string& getTitle() const;
    void setTitle(const std::string& value);
    void setBordered();
    void setAlwaysBackground();
    void setNoScrollbar();
    void setDynamic();
    void setFontSize(int size);
    bool isCursorInside(const Vector2i& mousePos);

    bool isEnabled() const {
        return enabled;
    }

protected:
    virtual void beforeDraw(Nuklear& nuklear, const Vector2& viewport);
    virtual void drawLayout(Nuklear& nuklear) = 0;

    bool enabled{true};
    std::string title;
    Vector2 size{100.0f, 100.0f};
    Vector2 pos{0.0f, 0.0f};
    Nuklear::Flags flags{0};
    int fontSize{0};
};

class ENGINE_API GuiWindow2 : public GuiContext, public GuiWidgetLayout {
public:
    GuiWindow2(const FontFamily& fontFamily, int fontSize);
    virtual ~GuiWindow2() = default;

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

    void setBordered(bool value);
    void setBackground(bool value);
    void setNoScrollbar(bool value);
    void setDynamic(bool value);
    void setHeaderSuccess(bool value);
    void setHeaderDanger(bool value);

private:
    std::string title{"Window"};
    Vector2 size;
    Vector2 pos;
    GuiContext::Flags flags{0};
    bool enabled{false};
};
} // namespace Engine

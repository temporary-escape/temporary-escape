#pragma once

#include "../Assets/Image.hpp"
#include "../Graphics/Canvas.hpp"
#include "GuiStyle.hpp"

struct nk_context;
struct nk_user_font;

namespace Engine {
enum class GuiTextAlign : uint32_t {
    Left = 0x01 | 0x10,
    Center = 0x02 | 0x10,
    Right = 0x04 | 0x10,
    LeftTop = 0x01 | 0x08,
    CenterTop = 0x02 | 0x08,
    RightTop = 0x04 | 0x08,
    LeftBottom = 0x01 | 0x20,
    CenterBottom = 0x02 | 0x20,
    RightBottom = 0x04 | 0x20,
};

class ENGINE_API GuiContext {
public:
    using Flags = uint32_t;

    enum class WindowFlag : Flags {
        Border = (1 << (0)),
        Moveable = (1 << (1)),
        Scaleable = (1 << (2)),
        Closeable = (1 << (3)),
        Minimizable = (1 << (4)),
        NoScrollbar = (1 << (5)),
        Title = (1 << (6)),
        ScrollAutoHide = (1 << (7)),
        Background = (1 << (8)),
        ScaleLeft = (1 << (9)),
        NoInput = (1 << (10)),
        Dynamic = (1 << (11)),
        NonInteractive = (1 << (10)) | (1 << (12)),
        HeaderPrimary = (1 << (28)),
        HeaderSuccess = (1 << (29)),
        HeaderDanger = (1 << (30)),
    };

    struct WindowOptions {
        Flags flags;
        float opacity;
    };

    explicit GuiContext(const FontFamily& fontFamily, int fontSize);
    virtual ~GuiContext();

    void update();
    void render(Canvas& canvas);

    bool windowBegin(const std::string& id, const std::string& title, const Vector2& pos, const Vector2& size,
                     const WindowOptions& options);
    void windowEnd(Flags flags);
    bool groupBegin(const std::string& name, bool scrollbar, const bool border, const Color4& borderColor);
    void groupEnd();

    void layoutRowBegin(float height, int columns);
    void layoutRowPush(float width);
    void layoutRowEnd();

    void layoutTemplateBegin(float height);
    void layoutTemplatePushDynamic();
    void layoutTemplatePushVariable(float width);
    void layoutTemplatePushStatic(float width);
    void layoutTemplateEnd();

    void skip();
    bool button(const std::string& label, const GuiStyleButton& style, const ImagePtr& image = nullptr);
    bool buttonToggle(const std::string& label, bool& value);
    bool contextButton(const std::string& label, const ImagePtr& imageLeft, const ImagePtr& imageRight);
    bool checkbox(const std::string& label, bool& value);
    void label(const std::string& label, const Color4& color);
    void text(const std::string& value);
    bool textInput(std::string& text, size_t max);
    bool comboBegin(const Vector2& size, const std::string& label);
    void comboEnd();
    bool comboItem(const std::string& label);
    void progress(float value, float max);
    void progress(float value, float max, const GuiStyleProgress& style, float height);
    void image(const ImagePtr& img, const Color4& color);
    bool imageToggle(const ImagePtr& img, bool& value, const GuiStyleButton& style, const Color4& color);
    bool imageToggleLabel(const ImagePtr& img, bool& value, const GuiStyleButton& style, const Color4& color,
                          const std::string& text, const GuiTextAlign align);
    void tooltip(const std::string& text);
    bool isHovered() const;
    bool isMouseDown(const MouseButton button) const;
    Vector2 getWidgetSize() const;
    Vector2 getPadding() const;

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);
    void eventInputBegin();
    void eventInputEnd();
    void setFocused(const std::string& id);
    bool isWindowClosed() const;
    void setInputEnabled(bool value);

    bool hasActiveInput() const {
        return activeInput;
    }
    int getFontSize() const {
        return fontSize;
    }
    void setPadding(float value);
    const FontFamily& getFont() const;

private:
    struct CustomStyle;

    static inline const auto padding = 4.0f;

    void applyTheme();

    const int fontSize;
    std::unique_ptr<nk_context> nk;
    std::unique_ptr<nk_user_font> font;
    std::unique_ptr<CustomStyle> custom;

    std::vector<char> editBuffer;
    bool activeInput{false};
    bool inputEnabled{true};
};

inline GuiContext::Flags operator|(const GuiContext::WindowFlag a, const GuiContext::WindowFlag b) {
    return static_cast<GuiContext::Flags>(a) | static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags operator|(const GuiContext::Flags a, const GuiContext::WindowFlag b) {
    return a | static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags& operator|=(GuiContext::Flags& a, const GuiContext::WindowFlag b) {
    a = a | b;
    return a;
}

inline GuiContext::Flags operator&(const GuiContext::Flags a, const GuiContext::WindowFlag b) {
    return a & static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags& operator&=(GuiContext::Flags& a, const GuiContext::WindowFlag b) {
    a = a & static_cast<GuiContext::Flags>(b);
    return a;
}

inline GuiContext::Flags operator~(const GuiContext::WindowFlag a) {
    return ~static_cast<GuiContext::Flags>(a);
}
} // namespace Engine

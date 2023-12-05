#pragma once

#include "../Graphics/Canvas2.hpp"

struct nk_context;
struct nk_user_font;

namespace Engine {
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
        Transparent = (1 << (29)),
    };

    explicit GuiContext(const FontFamily& fontFamily, int fontSize);
    virtual ~GuiContext();

    void update();
    void render(Canvas2& canvas);

    bool beginWindow(const std::string& title, const Vector2& pos, const Vector2& size, Flags flags);
    void endWindow();

    void layoutRowBegin(float height, int columns);
    void layoutRowPush(float width);
    void layoutRowEnd();

    bool button(const std::string& label);

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);

    bool isDirty() const {
        return dirty;
    }
    void clearDirty() {
        dirty = false;
    }
    void setDirty() {
        dirty = true;
    }

private:
    static inline const auto padding = 4.0f;

    void applyTheme();

    const FontFamily& fontFamily;
    const int fontSize;
    std::unique_ptr<nk_context> nk;
    std::array<std::unique_ptr<nk_user_font>, FontFamily::total> fonts;
    std::list<std::function<void()>> inputEvents;
    bool dirty{true};
    bool resetStyle{false};
};

inline GuiContext::Flags operator|(const GuiContext::WindowFlag a, const GuiContext::WindowFlag b) {
    return static_cast<GuiContext::Flags>(a) | static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags operator|(const GuiContext::Flags a, const GuiContext::WindowFlag b) {
    return a | static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags operator|=(const GuiContext::Flags a, const GuiContext::WindowFlag b) {
    return a | static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags operator&(const GuiContext::Flags a, const GuiContext::WindowFlag b) {
    return a & static_cast<GuiContext::Flags>(b);
}

inline GuiContext::Flags operator~(const GuiContext::WindowFlag a) {
    return static_cast<GuiContext::Flags>(a);
}
} // namespace Engine

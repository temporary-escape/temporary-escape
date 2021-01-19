#pragma once

#include <cstdint>

namespace Scissio {
typedef uint32_t GuiFlags;

struct GuiFlag {
    static constexpr GuiFlags Border = (1 << (0));
    static constexpr GuiFlags Moveable = (1 << (1));
    static constexpr GuiFlags Scaleable = (1 << (2));
    static constexpr GuiFlags Closeable = (1 << (3));
    static constexpr GuiFlags Minimizable = (1 << (4));
    static constexpr GuiFlags NoScrollbar = (1 << (5));
    static constexpr GuiFlags Title = (1 << (6));
    static constexpr GuiFlags ScrollAutoHide = (1 << (7));
    static constexpr GuiFlags Background = (1 << (8));
    static constexpr GuiFlags ScaleLeft = (1 << (9));
    static constexpr GuiFlags NoInput = (1 << (10));
    static constexpr GuiFlags Dynamic = (1 << (11));
    static constexpr GuiFlags CenterX = (1 << (29));
    static constexpr GuiFlags CenterY = 1 << 30;
};

/*inline GuiFlags operator|(const GuiFlags a, const GuiFlag b) {
    return static_cast<GuiFlags>(a | GuiFlags(b));
}
inline GuiFlags operator|(const GuiFlag a, const GuiFlag b) {
    return GuiFlags(a) | GuiFlags(b);
}
inline GuiFlags& operator|=(GuiFlags& a, const GuiFlag b) {
    a = static_cast<GuiFlags>(a | GuiFlags(b));
    return a;
}
inline bool operator&(const GuiFlags a, const GuiFlag b) {
    return static_cast<GuiFlags>(a & GuiFlags(b));
}*/
} // namespace Scissio

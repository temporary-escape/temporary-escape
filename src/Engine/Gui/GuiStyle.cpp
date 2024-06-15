#include "GuiStyle.hpp"
#include "../Config.hpp"

using namespace Engine;

GuiColor Engine::toGuiColor(const Color4& color) {
    return GuiColor{
        static_cast<uint8_t>(color.r * 255.0f),
        static_cast<uint8_t>(color.g * 255.0f),
        static_cast<uint8_t>(color.b * 255.0f),
        static_cast<uint8_t>(color.a * 255.0f),
    };
}

static GuiStyleButton buttonStyleSolid(const Color4& primary) {
    return {
        GuiStyleColor{
            toGuiColor(Colors::white),
            toGuiColor(Colors::white),
            toGuiColor(Colors::black),
        }, // Text
        GuiStyleColor{
            toGuiColor(Colors::transparent),
            toGuiColor(Colors::white),
            toGuiColor(Colors::white),
        }, // Border
        GuiStyleColor{
            toGuiColor(primary),
            toGuiColor(primary),
            toGuiColor(Colors::white),
        }, // Color
    };
}

static GuiStyleButton buttonStyleOutline(const Color4& primary) {
    return {
        GuiStyleColor{
            toGuiColor(primary),
            toGuiColor(primary),
            toGuiColor(Colors::black),
        }, // Text
        GuiStyleColor{
            toGuiColor(primary),
            toGuiColor(Colors::white),
            toGuiColor(primary),
        }, // Border
        GuiStyleColor{
            toGuiColor(primary * alpha(0.1f)),
            toGuiColor(primary * alpha(0.1f)),
            toGuiColor(primary),
        }, // Color
    };
}

GuiStyleButton Engine::guiStyleButtonBlue{buttonStyleSolid(Colors::blue)};
GuiStyleButton Engine::guiStyleButtonBlueOutline{buttonStyleOutline(Colors::blue)};
GuiStyleButton Engine::guiStyleButtonGreen{buttonStyleSolid(Colors::green)};
GuiStyleButton Engine::guiStyleButtonGreenOutline{buttonStyleOutline(Colors::green)};
GuiStyleButton Engine::guiStyleButtonYellow{buttonStyleSolid(Colors::yellow)};
GuiStyleButton Engine::guiStyleButtonYellowOutline{buttonStyleOutline(Colors::yellow)};
GuiStyleButton Engine::guiStyleButtonRed{buttonStyleSolid(Colors::red)};
GuiStyleButton Engine::guiStyleButtonRedOutline{buttonStyleOutline(Colors::red)};
GuiStyleButton Engine::guiStyleButtonPurple{buttonStyleSolid(Colors::purple)};
GuiStyleButton Engine::guiStyleButtonPurpleOutline{buttonStyleOutline(Colors::purple)};
GuiStyleButton Engine::guiStyleButtonGray{buttonStyleSolid(Colors::grey)};
GuiStyleButton Engine::guiStyleButtonGrayOutline{buttonStyleOutline(Colors::grey)};

GuiStyleWindow Engine::guiStyleWindowDefault{
    toGuiColor(Colors::background), // Header
    toGuiColor(Colors::yellow),     // Border
    toGuiColor(Colors::yellow),     // Title
    toGuiColor(Colors::background), // Background
};
GuiStyleWindow Engine::guiStyleWindowYellow{
    toGuiColor(Colors::yellow),     // Header
    toGuiColor(Colors::yellow),     // Border
    toGuiColor(Colors::black),      // Title
    toGuiColor(Colors::background), // Background
};
GuiStyleWindow Engine::guiStyleWindowGreen{
    toGuiColor(Colors::green),      // Header
    toGuiColor(Colors::green),      // Border
    toGuiColor(Colors::white),      // Title
    toGuiColor(Colors::background), // Background
};
GuiStyleWindow Engine::guiStyleWindowRed{
    toGuiColor(Colors::red),        // Header
    toGuiColor(Colors::red),        // Border
    toGuiColor(Colors::white),      // Title
    toGuiColor(Colors::background), // Background
};
GuiStyleWindow Engine::guiStyleWindowNone{
    toGuiColor(Colors::transparent), // Header
    toGuiColor(Colors::transparent), // Border
    toGuiColor(Colors::white),       // Title
    toGuiColor(Colors::transparent), // Background
};

GuiStyleProgress Engine::guiStyleProgressYellow{
    GuiStyleColor{
        toGuiColor(Colors::border),
        toGuiColor(Colors::white),
        toGuiColor(Colors::white),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::yellow),
        toGuiColor(Colors::yellow),
        toGuiColor(Colors::white),
    }, // Bar
    GuiStyleColor{
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
    }, // Background
};
GuiStyleProgress Engine::guiStyleProgressGreen{
    GuiStyleColor{
        toGuiColor(Colors::border),
        toGuiColor(Colors::white),
        toGuiColor(Colors::white),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::green),
        toGuiColor(Colors::green),
        toGuiColor(Colors::white),
    }, // Bar
    GuiStyleColor{
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
    }, // Background
};
GuiStyleProgress Engine::guiStyleProgressBlue{
    GuiStyleColor{
        toGuiColor(Colors::border),
        toGuiColor(Colors::white),
        toGuiColor(Colors::white),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::blue),
        toGuiColor(Colors::blue),
        toGuiColor(Colors::white),
    }, // Bar
    GuiStyleColor{
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
    }, // Background
};
GuiStyleProgress Engine::guiStyleProgressPurple{
    GuiStyleColor{
        toGuiColor(Colors::border),
        toGuiColor(Colors::white),
        toGuiColor(Colors::white),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::purple),
        toGuiColor(Colors::purple),
        toGuiColor(Colors::white),
    }, // Bar
    GuiStyleColor{
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
        toGuiColor(Colors::background),
    }, // Background
};

GuiStyleGroup Engine::guiStyleGroupDefault{
    GuiStyleColor{
        toGuiColor(Colors::border),
        toGuiColor(Colors::border),
        toGuiColor(Colors::border),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
    }, // Background
};
GuiStyleGroup Engine::guiStyleGroupSelectable{
    GuiStyleColor{
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::white),
        toGuiColor(Colors::yellow),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::group),
        toGuiColor(Colors::group),
        toGuiColor(Colors::group),
    }, // Background
};
GuiStyleGroup Engine::guiStyleGroupNone{
    GuiStyleColor{
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
    }, // Border
    GuiStyleColor{
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
        toGuiColor(Colors::transparent),
    }, // Background
};

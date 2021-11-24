#pragma once

#include "Stats.hpp"
#include "Widget.hpp"

namespace Scissio {
class WidgetDebugStats : public Widget {
public:
    WidgetDebugStats(GuiContext& gui, const Stats& stats);

private:
    void renderInternal(GuiContext& gui) override;

    const Stats& stats;
};
} // namespace Scissio

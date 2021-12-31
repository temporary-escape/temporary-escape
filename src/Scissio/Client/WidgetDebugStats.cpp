#include "WidgetDebugStats.hpp"

using namespace Scissio;

WidgetDebugStats::WidgetDebugStats(GuiContext& gui, const Scissio::Stats& stats) : Widget(gui), stats(stats) {
}

void WidgetDebugStats::renderInternal(GuiContext& gui) {
    auto draw = [&](const std::string& label, const std::string& format, auto stat) {
        gui.layoutDynamic(25.0f, 2);
        gui.label(label);
        gui.label(fmt::format(format, stat), TextAlign::Right);
    };

    draw("Server latency", "{}ms", stats.network.latencyMs.load());
    draw("Packets sent", "{}", stats.network.packetsSent.load());
    draw("Packets received", "{}", stats.network.packetsReceived.load());
    draw("Frame time", "{}ms", stats.render.frameTimeMs.load());
}

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

    draw("Network latency", "{}ms", stats.networkLatencyMs.load());
    draw("Server latency", "{}ms", stats.serverLatencyMs.load());
    draw("Packets sent", "{}", stats.packetsSent.load());
    draw("Packets received", "{}", stats.packetsReceived.load());
    draw("Frame time", "{}ms", stats.frameTimeMs.load());
}

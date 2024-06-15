#include "GuiWindowLoadStatus.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowLoadStatus::GuiWindowLoadStatus(GuiContext& ctx, const FontFamily& fontFamily, int fontSize) :
    GuiWindow{ctx, fontFamily, fontSize} {

    setSize({500.0f, 250.0f});
    setTitle("LOADING");
    setNoScrollbar(true);
    setCloseable(false);
    setDynamic(true);
    // setStyle(guiStyleWindowYellow);

    {
        auto& row = addWidget<GuiWidgetRow>(60.0f, 1);
        text = &row.addWidget<GuiWidgetText>("", getSize().x - 20.0f);
    }

    {
        auto& row = addWidget<GuiWidgetRow>(20.0f, 2);
        progressBar = &row.addWidget<GuiWidgetProgressBar>();
        progressBar->setMax(1.0f);
        progressBar->setValue(0.0f);
        progressBar->setStyle(guiStyleProgressYellow);
    }
}

void GuiWindowLoadStatus::setStatus(const std::string& message, const float progress) {
    text->setText(message, getSize().x - 20.0f);
    progressBar->setValue(glm::clamp(progress, 0.0f, 1.0f));
}

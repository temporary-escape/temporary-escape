#include "GuiWindowModal.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowModal::GuiWindowModal(const FontFamily& fontFamily, int fontSize, std::string title, std::string text,
                               const std::vector<std::string>& choices, int timeout) :
    GuiWindow{fontFamily, fontSize}, timeout{timeout} {
    setSize({350.0f, 200.0f});
    setTitle(std::move(title));
    setDynamic(true);

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        row.addWidget<GuiWidgetLabel>(std::move(text));
    }

    if (timeout) {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        progressBar = &row.addWidget<GuiWidgetProgressBar>();
        progressBar->setMax(timeout);
        progressBar->setValue(timeout);
        start = std::chrono::steady_clock::now();
    }

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 2 + choices.size());
        row.addEmpty().setWidth(0.1f);

        for (const auto& choice : choices) {
            auto& button = row.addWidget<GuiWidgetButton>(choice);
            button.setWidth(0.8f / static_cast<float>(choices.size()));
            button.setOnClick([this, choice]() {
                try {
                    if (this->onClickCallback) {
                        this->onClickCallback(choice);
                    }
                } catch (std::exception& e) {
                    BACKTRACE(e, "Modal callback caught exception");
                }
            });
        }
        row.addEmpty().setWidth(0.1f);
    }
}

void GuiWindowModal::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);
    if (progressBar) {
        ctx.setDirty();

        const auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        if (diff > timeout) {
            diff = timeout;
        }
        progressBar->setValue(timeout - diff);

        if (timeout - diff <= 0) {
            onClickCallback("");
        }
    }
}

void GuiWindowModal::setOnClickCallback(const OnClickCallback& value) {
    onClickCallback = value;
}

#include "GuiWindowModal.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowModal::GuiWindowModal(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, std::string title,
                               std::string text, const std::vector<std::string>& choices, int timeout) :
    GuiWindow{ctx, fontFamily, fontSize}, text{std::move(text)}, timeout{timeout} {
    setSize({350.0f, 200.0f});
    setTitle(std::move(title));
    setDynamic(true);

    {
        widgetRowText = &addWidget<GuiWidgetRow>(30.0f, 1);
        widgetText = &widgetRowText->addWidget<GuiWidgetText>(this->text, getSize().x - 20.0f);
        widgetRowText->setHeight(widgetText->getSuggestedHeight());
    }

    if (timeout) {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        progressBar = &row.addWidget<GuiWidgetProgressBar>();
        progressBar->setMax(timeout);
        progressBar->setValue(timeout);
        start = std::chrono::steady_clock::now();
    }

    {
        widgetRowChoices = &addWidget<GuiWidgetRow>(30.0f, 2 + choices.size());
        setChoices(choices);
    }

    lastWidth = getSize().x;
}

void GuiWindowModal::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);

    if (progressBar) {
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

    if (lastWidth != getSize().x) {
        lastWidth = getSize().x;

        widgetText->setText(this->text, getSize().x - 20.0f);
        widgetRowText->setHeight(widgetText->getSuggestedHeight());
    }
}

void GuiWindowModal::setText(std::string value) {
    text = std::move(value);
    lastWidth = 0.0f;
}

void GuiWindowModal::setChoices(const std::vector<std::string>& choices) {
    widgetRowChoices->clearWidgets();

    const auto margin = choices.size() > 4 ? 0.0f : 1.0f - static_cast<float>(choices.size()) * 0.25f;
    const auto width = choices.size() > 4 ? 1.0f / static_cast<float>(choices.size()) : 0.25f;

    if (margin > 0.01f) {
        widgetRowChoices->addEmpty().setWidth(margin / 2.0f);
    }

    for (const auto& choice : choices) {
        auto& button = widgetRowChoices->addWidget<GuiWidgetButton>(choice);
        button.setWidth(width);
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

    if (margin > 0.001f) {
        widgetRowChoices->addEmpty().setWidth(margin / 2.0f);
    }
}

void GuiWindowModal::setOnClickCallback(const OnClickCallback& value) {
    onClickCallback = value;
}

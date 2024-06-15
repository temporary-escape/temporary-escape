#include "GuiWindowModal.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowModal::GuiWindowModal(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, std::string title,
                               std::string text) :
    GuiWindow{ctx, fontFamily, fontSize}, text{std::move(text)} {
    setSize({350.0f, 200.0f});
    setTitle(std::move(title));
    setDynamic(true);

    {
        widgetRowText = &addWidget<GuiWidgetRow>(30.0f, 1);
        widgetText = &widgetRowText->addWidget<GuiWidgetText>(this->text, getSize().x - 20.0f);
        widgetRowText->setHeight(widgetText->getSuggestedHeight());
    }

    widgetRowTimeout = &addWidget<GuiWidgetRow>(0.0f, 1);
    widgetRowChoices = &addWidget<GuiWidgetRow>(0.0f, 1);

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

        if (onTimeout && timeout - diff <= 0) {
            onTimeout();
            onTimeout = nullptr;
        }
    }

    if (lastWidth != getSize().x && !this->text.empty()) {
        lastWidth = getSize().x;

        widgetText->setText(this->text, getSize().x - 20.0f);
        widgetRowText->setHeight(widgetText->getSuggestedHeight());
    }
}

void GuiWindowModal::setText(std::string value) {
    text = std::move(value);
    lastWidth = 0.0f;
}

void GuiWindowModal::addChoice(const std::string& label, OnChoiceCallback callback) {
    addChoice(label, guiStyleButtonYellowOutline, std::move(callback));
}

void GuiWindowModal::clearChoices() {
    choices.clear();
    recreateChoices();
}

void GuiWindowModal::addChoice(const std::string& label, const GuiStyleButton& style, OnChoiceCallback callback) {
    choices.push_back(Choice{
        label,
        std::move(callback),
        &style,
    });
    recreateChoices();
}

void GuiWindowModal::setTimeout(const int value, OnTimeoutCallback callback) {
    timeout = value;
    onTimeout = std::move(callback);

    widgetRowTimeout->setHeight(30.0f);
    progressBar = &widgetRowTimeout->addWidget<GuiWidgetProgressBar>();
    progressBar->setMax(timeout);
    progressBar->setValue(timeout);
    start = std::chrono::steady_clock::now();
}

void GuiWindowModal::setCloseOnClick(const bool value) {
    closeOnClick = value;
}

void GuiWindowModal::recreateChoices() {
    widgetRowChoices->clearWidgets();

    widgetRowChoices->setHeight(30.0f);

    const auto margin = choices.size() > 4 ? 0.0f : 1.0f - static_cast<float>(choices.size()) * 0.25f;
    const auto width = choices.size() > 4 ? 1.0f / static_cast<float>(choices.size()) : 0.25f;

    if (margin > 0.1f) {
        widgetRowChoices->setColumns(choices.size() + 2);
    } else {
        widgetRowChoices->setColumns(choices.size());
    }

    if (margin > 0.01f) {
        widgetRowChoices->addEmpty().setWidth(margin / 2.0f);
    }

    for (const auto& choice : choices) {
        auto& button = widgetRowChoices->addWidget<GuiWidgetButton>(choice.label);
        button.setStyle(*choice.style);
        button.setWidth(width);
        button.setOnClick([this, &choice]() {
            try {
                if (choice.callback) {
                    choice.callback();

                    if (closeOnClick) {
                        this->close();
                    }
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

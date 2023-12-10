#include "GuiWindowModal.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowModal::GuiWindowModal(const FontFamily& fontFamily, int fontSize, std::string title, std::string text,
                               const std::vector<std::string>& choices) :
    GuiWindow2{fontFamily, fontSize} {
    setSize({350.0f, 200.0f});
    setTitle(std::move(title));
    setDynamic(true);

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        row.addWidget<GuiWidgetLabel>(std::move(text));
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

void GuiWindowModal::setOnClickCallback(const OnClickCallback& value) {
    onClickCallback = value;
}

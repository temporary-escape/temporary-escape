#include "GuiWindowMultiplayerSettings.hpp"
#include "../../Utils/Platform.hpp"
#include "../GuiManager.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static bool isValid(const std::string& value) {
    static std::regex re{"^[a-zA-Z0-9\\s]+$"};
    std::smatch m;
    return std::regex_match(value, m, re);
}

static std::string_view validateAlphaNumeric(const std::string& value, const size_t minSize) {
    if (value.empty() && minSize == 0) {
        return "";
    } else if (value.size() < minSize) {
        return "is too short";
    } else if (value.front() == ' ') {
        return "must not start with a space";
    } else if (value.back() == ' ') {
        return "must not end with a space";
    } else if (!isValid(value)) {
        return "contains invalid characters";
    }
    return "";
}

GuiWindowMultiplayerSettings::GuiWindowMultiplayerSettings(GuiContext& ctx, const FontFamily& fontFamily,
                                                           const int fontSize) :
    GuiWindow{ctx, fontFamily, fontSize} {

    setSize({500.0f, 600.0f});
    setTitle("HOST GAME");
    setNoScrollbar(true);
    setCloseable(true);

    const auto height = getSize().y - 30.0 - 30.0f * 6.0f - ctx.getPadding().y * 8.0f;

    { // Server name
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);

        row.addWidget<GuiWidgetLabel>("Server name:");
        inputName = &row.addWidget<GuiWidgetTextInput>();
        inputName->setOnModify([this]() { validate(); });
    }

    { // Server password
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);

        row.addWidget<GuiWidgetLabel>("Server password:");
        inputPassword = &row.addWidget<GuiWidgetTextInput>();
        inputPassword->setOnModify([this]() { validate(); });
    }

    { auto& row = addWidget<GuiWidgetRow>(height, 1); }

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        labelError = &row.addWidget<GuiWidgetLabel>("");
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        row.addEmpty().setWidth(0.25f);

        auto& done = row.addWidget<GuiWidgetButton>("Start");
        done.setWidth(0.5f);
        done.setStyle(guiStyleButtonGreen);
        done.setOnClick([this]() {
            if (onStart && valid) {
                Form form{};
                form.name = inputName->getValue();
                form.password = inputPassword->getValue();

                onStart(form);
            }
        });

        row.addEmpty().setWidth(0.25f);
    }
}

void GuiWindowMultiplayerSettings::setServerName(std::string value) {
    inputName->setValue(std::move(value));
    validate();
}

void GuiWindowMultiplayerSettings::validate() {
    const auto nameCheck = validateAlphaNumeric(inputName->getValue(), 6);
    if (!nameCheck.empty()) {
        labelError->setLabel(fmt::format("<t>Name {}</t>", nameCheck));
    }

    const auto passCheck = validateAlphaNumeric(inputPassword->getValue(), 0);
    if (!passCheck.empty()) {
        labelError->setLabel(fmt::format("<t>Password {}</t>", passCheck));
    }

    valid = nameCheck.empty() && passCheck.empty();
    labelError->setHidden(valid);
}

void GuiWindowMultiplayerSettings::setOnStart(OnStartCallback callback) {
    onStart = std::move(callback);
}

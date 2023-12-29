#include "GuiWindowCreateProfile.hpp"
#include <regex>

using namespace Engine;

static bool isValid(const std::string& value) {
    static std::regex re{"^[a-zA-Z0-9\\s]+$"};
    std::smatch m;
    return std::regex_match(value, m, re);
}

GuiWindowCreateProfile::GuiWindowCreateProfile(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {
    setSize({350.0f, 200.0f});
    setTitle("Create New Profile");
    setDynamic(true);

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        input = &row.addWidget<GuiWidgetTextInput>(30);
        input->setValue("Some Player");
        input->setOnModify([this]() {
            const auto& val = input->getValue();
            if (val.size() < 3) {
                labelError->setLabel("Must have at least 3 characters");
                valid = false;
            } else if (val.front() == ' ') {
                labelError->setLabel("Must not start with a space");
                valid = false;
            } else if (val.back() == ' ') {
                labelError->setLabel("Must not end with a space");
                valid = false;
            } else if (!isValid(val)) {
                labelError->setLabel("Contains invalid characters");
                valid = false;
            } else {
                labelError->setLabel("");
                valid = true;
            }
        });
    }

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        labelError = &row.addWidget<GuiWidgetLabel>("");
    }

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 3);
        row.addEmpty().setWidth(0.25f);
        auto& button = row.addWidget<GuiWidgetButton>("Create");
        button.setWidth(0.5f);
        button.setOnClick([this]() {
            if (valid && onCreateCallback) {
                Result result{};
                result.name = input->getValue();
                onCreateCallback(result);
            }
        });
        row.addEmpty().setWidth(0.25f);
    }
}

void GuiWindowCreateProfile::setOnCreateCallback(OnCreateCallback value) {
    onCreateCallback = std::move(value);
}
